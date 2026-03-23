#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include <stddef.h>
#include "idt.h"

#define SYS_EXIT    1
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETPID  20
#define SYS_MKDIR   39
#define SYS_UPTIME  100

#define STDOUT 1
#define STDERR 2

void syscall_init(void);
int32_t sys_write(int fd, const char* buf, size_t count);
int32_t sys_read(int fd, char* buf, size_t count);
int32_t sys_mkdir(const char* dirname);
int32_t sys_getpid(void);
int32_t sys_uptime(void);

#endif
