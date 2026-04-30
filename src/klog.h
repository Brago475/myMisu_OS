#ifndef KLOG_H
#define KLOG_H

#include <stdint.h>

#define KLOG_MAX_ENTRIES 64
#define KLOG_LINE_LEN    96

typedef enum {
    KLOG_INFO = 0,
    KLOG_WARN,
    KLOG_ERR,
    KLOG_OK
} klog_level_t;

typedef struct {
    uint32_t tick;
    klog_level_t level;
    char msg[KLOG_LINE_LEN];
} klog_entry_t;

void klog_init(void);
void klog(klog_level_t level, const char* msg);
void klog_print(void);  /* prints all entries */
int  klog_count(void);

#endif
