#include "syscall.h"
#include "vga.h"
#include "keyboard.h"
#include "timer.h"
#include "process.h"
#include "fs.h"
#include "kprintf.h"

static void syscall_handler(registers_t* regs) {
    int32_t result = -1;

    switch (regs->eax) {
        case SYS_WRITE:
            result = sys_write((int)regs->ebx, (const char*)regs->ecx, (size_t)regs->edx);
            break;
        case SYS_READ:
            result = sys_read((int)regs->ebx, (char*)regs->ecx, (size_t)regs->edx);
            break;
        case SYS_MKDIR:
            result = sys_mkdir((const char*)regs->ebx);
            break;
        case SYS_GETPID:
            result = sys_getpid();
            break;
        case SYS_UPTIME:
            result = sys_uptime();
            break;
        case SYS_EXIT:
            result = 0;
            break;
        default:
            result = -1;
            break;
    }

    regs->eax = (uint32_t)result;
}

int32_t sys_write(int fd, const char* buf, size_t count) {
    if (fd != STDOUT && fd != STDERR)
        return -1;
    if (buf == 0)
        return -1;

    if (fd == STDERR)
        terminal_setcolor(vga_entry_color(VGA_RED, VGA_BLACK));

    for (size_t i = 0; i < count; i++)
        terminal_putchar(buf[i]);

    if (fd == STDERR)
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));

    return (int32_t)count;
}

int32_t sys_read(int fd, char* buf, size_t count) {
    if (fd != 0) return -1;
    if (buf == 0) return -1;

    for (size_t i = 0; i < count; i++) {
        buf[i] = keyboard_getchar();
        if (buf[i] == '\n') return (int32_t)(i + 1);
    }
    return (int32_t)count;
}

int32_t sys_mkdir(const char* dirname) {
    if (dirname == 0) return -1;
    return fs_mkdir(dirname);
}

int32_t sys_getpid(void) {
    return (int32_t)process_get_current_pid();
}

int32_t sys_uptime(void) {
    return (int32_t)timer_get_ticks();
}

void syscall_init(void) {
    register_interrupt_handler(128, syscall_handler);
}
