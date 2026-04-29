#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PROCESSES 16
#define PROC_NAME_LEN 32
#define KSTACK_SIZE   4096  /* 4 KB per process */

typedef enum {
    PROC_UNUSED = 0,
    PROC_RUNNING,
    PROC_READY,
    PROC_BLOCKED,
    PROC_TERMINATED
} proc_state_t;

typedef struct {
    uint32_t pid;
    char name[PROC_NAME_LEN];
    proc_state_t state;
    uint32_t saved_esp;        /* saved stack pointer for context switch */
    uint32_t* kstack;          /* base of kernel stack (4 KB)            */
    uint32_t esp;              /* legacy field, kept for ps display      */
    uint32_t eip;              /* legacy field                           */
    uint32_t page_directory;
    uint32_t priority;
    uint32_t ticks_used;
    uint32_t created_tick;
    uint32_t wake_tick;        /* tick at which a sleeping proc wakes    */
    void (*entry)(void);       /* function this process starts in        */
    bool in_use;
} process_t;

/* Init and accessors */
void process_init(void);
int process_create(const char* name, uint32_t priority);
int process_create_kernel(const char* name, void (*fn)(void));
void process_terminate(uint32_t pid);
uint32_t process_get_current_pid(void);
process_t* process_get_table(void);
process_t* process_get_current(void);
int process_get_count(void);
void process_tick(void);

/* Scheduler */
void schedule(void);
void process_sleep(uint32_t ticks);
void process_yield(void);
void process_set_scheduling_enabled(bool enabled);
bool process_is_scheduling_enabled(void);

/* Implemented in context_switch.s */
extern void context_switch(uint32_t* old_esp_ptr, uint32_t new_esp);

#endif
