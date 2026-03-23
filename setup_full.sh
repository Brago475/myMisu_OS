#!/bin/bash
echo "Adding system calls, processes, filesystem..."
cat > src/syscall.h << 'EOF'
#ifndef SYSCALL_H
#define SYSCALL_H
#include <stdint.h>
#include <stddef.h>
#define SYS_EXIT 1
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_GETPID 20
#define SYS_MKDIR 39
#define SYS_UPTIME 100
#define STDOUT 1
#define STDERR 2
void syscall_init(void);
#endif
EOF
echo "Created syscall.h"
