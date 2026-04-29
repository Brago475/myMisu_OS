#include "syscall.h"
#include "errno.h"
#include "vga.h"
#include "keyboard.h"
#include "timer.h"
#include "process.h"
#include "fs.h"
#include "kprintf.h"
#include "string.h"

static void syscall_handler(registers_t* regs);

void syscall_init(void) {
    register_interrupt_handler(128, syscall_handler);
}

int32_t sys_exit(int code) {
    (void)code;
    uint32_t pid = process_get_current_pid();
    if (pid > 1) process_terminate(pid);
    return 0;
}

int32_t sys_fork(void) { return -ENOSYS; }

int32_t sys_read(int fd, char* buf, size_t count) {
    if (!buf) return -EFAULT;
    if (fd == STDIN) {
        for (size_t i = 0; i < count; i++) {
            buf[i] = keyboard_getchar();
            if (buf[i] == '\n') return (int32_t)(i + 1);
        }
        return (int32_t)count;
    }
    if (fd >= 3) return fs_read_fd(fd - 3, buf, count);
    return -EBADF;
}

int32_t sys_write(int fd, const char* buf, size_t count) {
    if (!buf) return -EFAULT;
    if (fd == STDOUT) {
        for (size_t i = 0; i < count; i++) terminal_putchar(buf[i]);
        return (int32_t)count;
    }
    if (fd == STDERR) {
        terminal_setcolor(vga_entry_color(VGA_RED, VGA_BLACK));
        for (size_t i = 0; i < count; i++) terminal_putchar(buf[i]);
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
        return (int32_t)count;
    }
    if (fd >= 3) return fs_write_fd(fd - 3, buf, count);
    return -EBADF;
}

int32_t sys_open(const char* path, uint32_t flags) {
    if (!path) return -EFAULT;
    int fd = fs_open(path, flags);
    if (fd >= 0) return fd + 3;
    return fd;
}

int32_t sys_close(int fd) {
    if (fd < 3) return -EBADF;
    return fs_close(fd - 3);
}

int32_t sys_waitpid(int pid) { (void)pid; return -ENOSYS; }

int32_t sys_unlink(const char* path) {
    if (!path) return -EFAULT;
    return fs_unlink(path);
}

int32_t sys_execve(const char* path) { (void)path; return -ENOSYS; }

int32_t sys_chdir(const char* path) {
    if (!path) return -EFAULT;
    return fs_cd(path);
}

int32_t sys_lseek(int fd, int32_t offset, int whence) {
    if (fd < 3) return -EBADF;
    return fs_lseek(fd - 3, offset, whence);
}

int32_t sys_getpid(void) { return (int32_t)process_get_current_pid(); }

int32_t sys_kill(int pid) {
    if (pid <= 1) return -EPERM;
    process_terminate((uint32_t)pid);
    return 0;
}

int32_t sys_mkdir(const char* dirname) {
    if (!dirname) return -EFAULT;
    return fs_mkdir(dirname);
}

int32_t sys_rmdir(const char* dirname) {
    if (!dirname) return -EFAULT;
    return fs_rmdir(dirname);
}

int32_t sys_stat(const char* path, void* statbuf) {
    if (!path || !statbuf) return -EFAULT;
    return fs_stat(path, (fs_stat_t*)statbuf);
}

int32_t sys_getcwd(char* buf, size_t size) {
    if (!buf) return -EFAULT;
    return fs_getcwd(buf, size);
}

int32_t sys_sleep(uint32_t ticks) {
    uint32_t start = timer_get_ticks();
    while (timer_get_ticks() - start < ticks) {
        __asm__ __volatile__("hlt");
    }
    return 0;
}

int32_t sys_yield(void) {
    __asm__ __volatile__("hlt");
    return 0;
}

int32_t sys_uptime(void) { return (int32_t)timer_get_ticks(); }

static void syscall_handler(registers_t* regs) {
    int32_t result = -ENOSYS;
    switch (regs->eax) {
        case SYS_EXIT:    result = sys_exit((int)regs->ebx); break;
        case SYS_FORK:    result = sys_fork(); break;
        case SYS_READ:    result = sys_read((int)regs->ebx, (char*)regs->ecx, (size_t)regs->edx); break;
        case SYS_WRITE:   result = sys_write((int)regs->ebx, (const char*)regs->ecx, (size_t)regs->edx); break;
        case SYS_OPEN:    result = sys_open((const char*)regs->ebx, regs->ecx); break;
        case SYS_CLOSE:   result = sys_close((int)regs->ebx); break;
        case SYS_WAITPID: result = sys_waitpid((int)regs->ebx); break;
        case SYS_UNLINK:  result = sys_unlink((const char*)regs->ebx); break;
        case SYS_EXECVE:  result = sys_execve((const char*)regs->ebx); break;
        case SYS_CHDIR:   result = sys_chdir((const char*)regs->ebx); break;
        case SYS_LSEEK:   result = sys_lseek((int)regs->ebx, (int32_t)regs->ecx, (int)regs->edx); break;
        case SYS_GETPID:  result = sys_getpid(); break;
        case SYS_KILL:    result = sys_kill((int)regs->ebx); break;
        case SYS_MKDIR:   result = sys_mkdir((const char*)regs->ebx); break;
        case SYS_RMDIR:   result = sys_rmdir((const char*)regs->ebx); break;
        case SYS_STAT:    result = sys_stat((const char*)regs->ebx, (void*)regs->ecx); break;
        case SYS_GETCWD:  result = sys_getcwd((char*)regs->ebx, (size_t)regs->ecx); break;
        case SYS_SLEEP:   result = sys_sleep(regs->ebx); break;
        case SYS_YIELD:   result = sys_yield(); break;
        case SYS_UPTIME:  result = sys_uptime(); break;
        default:          result = -ENOSYS; break;
    }
    regs->eax = (uint32_t)result;
}
