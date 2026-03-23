#include "process.h"
#include "string.h"
#include "timer.h"

static process_t proc_table[MAX_PROCESSES];
static uint32_t current_pid = 0;
static uint32_t next_pid = 1;

void process_init(void) {
    memset(proc_table, 0, sizeof(proc_table));

    proc_table[0].pid = 0;
    strcpy(proc_table[0].name, "kernel");
    proc_table[0].state = PROC_RUNNING;
    proc_table[0].priority = 0;
    proc_table[0].ticks_used = 0;
    proc_table[0].created_tick = 0;
    proc_table[0].in_use = 1;

    proc_table[1].pid = 1;
    strcpy(proc_table[1].name, "shell");
    proc_table[1].state = PROC_RUNNING;
    proc_table[1].priority = 1;
    proc_table[1].ticks_used = 0;
    proc_table[1].created_tick = 0;
    proc_table[1].in_use = 1;

    current_pid = 1;
    next_pid = 2;
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
            proc_table[i].in_use = 1;
            return (int)proc_table[i].pid;
        }
    }
    return -1;
}

void process_terminate(uint32_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (proc_table[i].in_use && proc_table[i].pid == pid && pid > 1) {
            proc_table[i].state = PROC_TERMINATED;
            proc_table[i].in_use = 0;
        }
    }
}

uint32_t process_get_current_pid(void) {
    return current_pid;
}

process_t* process_get_table(void) {
    return proc_table;
}

int process_get_count(void) {
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES; i++)
        if (proc_table[i].in_use) count++;
    return count;
}

void process_tick(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (proc_table[i].in_use && proc_table[i].pid == current_pid) {
            proc_table[i].ticks_used++;
            break;
        }
    }
}
