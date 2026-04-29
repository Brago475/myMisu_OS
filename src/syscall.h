#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>
#include <stddef.h>
#include "idt.h"

/* Syscall numbers (Linux i386 compatible where possible) */
#define SYS_EXIT     1
#define SYS_FORK     2
#define SYS_READ     3
#define SYS_WRITE    4
#define SYS_OPEN     5
#define SYS_CLOSE    6
#define SYS_WAITPID  7
#define SYS_UNLINK  10
#define SYS_EXECVE  11
#define SYS_CHDIR   12
#define SYS_LSEEK   19
#define SYS_GETPID  20
#define SYS_KILL    37
#define SYS_MKDIR   39
#define SYS_RMDIR   40
#define SYS_STAT    18
#define SYS_GETCWD 183
#define SYS_SLEEP  162
#define SYS_YIELD  158
#define SYS_UPTIME 100

#define SYS_MAX    256

/* Standard file descriptors */
#define STDIN   0
#define STDOUT  1
#define STDERR  2

/* Init */
void syscall_init(void);

/* Handlers (kernel-side wrappers, can also be called directly) */
int32_t sys_exit(int code);
int32_t sys_fork(void);
int32_t sys_read(int fd, char* buf, size_t count);
int32_t sys_write(int fd, const char* buf, size_t count);
int32_t sys_open(const char* path, uint32_t flags);
int32_t sys_close(int fd);
int32_t sys_waitpid(int pid);
int32_t sys_unlink(const char* path);
int32_t sys_execve(const char* path);
int32_t sys_chdir(const char* path);
int32_t sys_lseek(int fd, int32_t offset, int whence);
int32_t sys_getpid(void);
int32_t sys_kill(int pid);
int32_t sys_mkdir(const char* dirname);
int32_t sys_rmdir(const char* dirname);
int32_t sys_stat(const char* path, void* statbuf);
int32_t sys_getcwd(char* buf, size_t size);
int32_t sys_sleep(uint32_t ticks);
int32_t sys_yield(void);
int32_t sys_uptime(void);

#endif
