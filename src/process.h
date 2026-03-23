#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PROCESSES 16
#define PROC_NAME_LEN 32

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
    uint32_t esp;
    uint32_t eip;
    uint32_t page_directory;
    uint32_t priority;
    uint32_t ticks_used;
    uint32_t created_tick;
    bool in_use;
} process_t;

void process_init(void);
int process_create(const char* name, uint32_t priority);
void process_terminate(uint32_t pid);
uint32_t process_get_current_pid(void);
process_t* process_get_table(void);
int process_get_count(void);
void process_tick(void);

#endif
