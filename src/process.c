#include "process.h"
#include "string.h"
#include "timer.h"
#include "pmm.h"

static process_t proc_table[MAX_PROCESSES];
static uint32_t current_pid = 0;
static uint32_t next_pid = 1;
static bool scheduling_enabled = false;

/* Wrapper that runs a kernel task and cleans up when it returns. */
static void process_entry_wrapper(void) {
    asm volatile("sti");
    process_t* p = process_get_current();
    if (p && p->entry) {
        p->entry();
    }
    /* Task returned: mark terminated and yield forever. */
    if (p) {
        p->state = PROC_TERMINATED;
        p->in_use = 0;
    }
    while (1) process_yield();
}

void process_init(void) {
    memset(proc_table, 0, sizeof(proc_table));

    /* PID 0: kernel idle */
    proc_table[0].pid = 0;
    strcpy(proc_table[0].name, "kernel");
    proc_table[0].state = PROC_RUNNING;
    proc_table[0].priority = 0;
    proc_table[0].in_use = 1;

    /* PID 1: shell */
    proc_table[1].pid = 1;
    strcpy(proc_table[1].name, "shell");
    proc_table[1].state = PROC_RUNNING;
    proc_table[1].priority = 1;
    proc_table[1].in_use = 1;

    current_pid = 1;
    next_pid = 2;
}

static int find_slot_by_pid(uint32_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++)
        if (proc_table[i].in_use && proc_table[i].pid == pid) return i;
    return -1;
}

int process_create(const char* name, uint32_t priority) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!proc_table[i].in_use) {
            proc_table[i].pid = next_pid++;
            strncpy(proc_table[i].name, name, PROC_NAME_LEN - 1);
            proc_table[i].name[PROC_NAME_LEN - 1] = '\0';
            proc_table[i].state = PROC_READY;
            proc_table[i].priority = priority;
            proc_table[i].ticks_used = 0;
            proc_table[i].created_tick = timer_get_ticks();
            proc_table[i].wake_tick = 0;
            proc_table[i].kstack = 0;
            proc_table[i].entry = 0;
            proc_table[i].in_use = 1;
            return (int)proc_table[i].pid;
        }
    }
    return -1;
}

int process_create_kernel(const char* name, void (*fn)(void)) {
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!proc_table[i].in_use) { slot = i; break; }
    }
    if (slot < 0) return -1;

    /* Allocate a 4KB kernel stack */
    uint32_t  stack = pmm_alloc_page();
    if (!stack) return -1;

    proc_table[slot].pid = next_pid++;
    strncpy(proc_table[slot].name, name, PROC_NAME_LEN - 1);
    proc_table[slot].name[PROC_NAME_LEN - 1] = '\0';
    proc_table[slot].state = PROC_READY;
    proc_table[slot].priority = 1;
    proc_table[slot].ticks_used = 0;
    proc_table[slot].created_tick = timer_get_ticks();
    proc_table[slot].wake_tick = 0;
    proc_table[slot].kstack = (uint32_t*)(uintptr_t)stack;
    proc_table[slot].entry = fn;
    proc_table[slot].in_use = 1;

    /* Set up the stack so the first context_switch into this process
     * lands inside process_entry_wrapper.
     *
     * Stack grows downward. Top of stack is base + KSTACK_SIZE.
     * Layout (from high address to low):
     *   [top-4 ]  return address = process_entry_wrapper
     *   [top-8 ]  saved EDI  (popad order: edi pop'd last from low addr)
     *   [top-12]  saved ESI
     *   [top-16]  saved EBP
     *   [top-20]  saved ESP_dummy   (popad ignores this slot)
     *   [top-24]  saved EBX
     *   [top-28]  saved EDX
     *   [top-32]  saved ECX
     *   [top-36]  saved EAX
     * saved_esp points to [top-36] which is what popad will pop from.
     */
    uint32_t* sp = (uint32_t*)(stack + KSTACK_SIZE);
    *(--sp) = (uint32_t)process_entry_wrapper;  /* ret address */
    /* 8 zeroed registers for popad */
    for (int i = 0; i < 8; i++) *(--sp) = 0;
    proc_table[slot].saved_esp = (uint32_t)sp;

    return (int)proc_table[slot].pid;
}

int process_terminate(uint32_t pid) {
    int slot = find_slot_by_pid(pid);
    if (slot < 0 || pid <= 1) return -1;
    proc_table[slot].state = PROC_TERMINATED;
    proc_table[slot].in_use = 0;
    if (proc_table[slot].kstack) {
        pmm_free_page((uint32_t)proc_table[slot].kstack);
        proc_table[slot].kstack = 0;
    }
    return 0;
}
uint32_t process_get_current_pid(void) { return current_pid; }

process_t* process_get_table(void) { return proc_table; }

process_t* process_get_current(void) {
    int slot = find_slot_by_pid(current_pid);
    if (slot < 0) return 0;
    return &proc_table[slot];
}

int process_get_count(void) {
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++)
        if (proc_table[i].in_use) count++;
    return count;
}

void process_tick(void) {
    /* Increment current process's CPU usage */
    int slot = find_slot_by_pid(current_pid);
    if (slot >= 0) proc_table[slot].ticks_used++;

    /* Wake any sleeping processes whose time has come */
    uint32_t now = timer_get_ticks();
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (proc_table[i].in_use &&
            proc_table[i].state == PROC_BLOCKED &&
            proc_table[i].wake_tick != 0 &&
            now >= proc_table[i].wake_tick) {
            proc_table[i].state = PROC_READY;
            proc_table[i].wake_tick = 0;
        }
    }
}

void process_set_scheduling_enabled(bool enabled) { scheduling_enabled = enabled; }
bool process_is_scheduling_enabled(void) { return scheduling_enabled; }

/* Round-robin scheduler. Picks the next READY process and context-switches. */
void schedule(void) {
    if (!scheduling_enabled) return;

    int cur_slot = find_slot_by_pid(current_pid);
    if (cur_slot < 0) cur_slot = 0;

    /* Scan from cur_slot+1 wrapping around for next READY. */
    int next_slot = -1;
    for (int i = 1; i <= MAX_PROCESSES; i++) {
        int s = (cur_slot + i) % MAX_PROCESSES;
        if (proc_table[s].in_use && proc_table[s].state == PROC_READY) {
            next_slot = s;
            break;
        }
    }

    if (next_slot < 0) return;  /* no one else ready, keep running */
    if (next_slot == cur_slot) return;

    /* Mark transitions */
    if (proc_table[cur_slot].state == PROC_RUNNING)
        proc_table[cur_slot].state = PROC_READY;
    proc_table[next_slot].state = PROC_RUNNING;

    uint32_t old_pid = current_pid;
    current_pid = proc_table[next_slot].pid;

    /* For the very first switch from a process that never had its esp saved,
     * we still need a place to store its esp. We use the current slot's
     * saved_esp field. That field is meaningless for kernel/shell on first
     * switch but becomes meaningful on subsequent switches.
     */
    (void)old_pid;
    context_switch(&proc_table[cur_slot].saved_esp, proc_table[next_slot].saved_esp);
}

void process_sleep(uint32_t ticks) {
    int slot = find_slot_by_pid(current_pid);
    if (slot < 0 || current_pid <= 1) {
        /* Can't safely sleep PID 0/1 - busy wait instead */
        uint32_t start = timer_get_ticks();
        while (timer_get_ticks() - start < ticks)
            __asm__ __volatile__("hlt");
        return;
    }
    proc_table[slot].state = PROC_BLOCKED;
    proc_table[slot].wake_tick = timer_get_ticks() + ticks;
    schedule();
}

void process_yield(void) {
    schedule();
}
