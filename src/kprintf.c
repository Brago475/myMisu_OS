#include "kprintf.h"
#include "vga.h"
#include <stdarg.h>

static void print_int(int value) {
    if (value < 0) {
        terminal_putchar('-');
        value = -value;
    }
    if (value / 10) print_int(value / 10);
    terminal_putchar('0' + (value % 10));
}

static void print_uint(unsigned int value) {
    if (value / 10) print_uint(value / 10);
    terminal_putchar('0' + (value % 10));
}

static void print_hex(unsigned int value) {
    const char* hex = "0123456789abcdef";
    if (value / 16) print_hex(value / 16);
    terminal_putchar(hex[value % 16]);
}

void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (size_t i = 0; fmt[i]; i++) {
        if (fmt[i] == '%' && fmt[i + 1]) {
            i++;
            switch (fmt[i]) {
                case 's': {
                    const char* s = va_arg(args, const char*);
                    if (s) terminal_writestring(s);
                    else terminal_writestring("(null)");
                    break;
                }
                case 'd': print_int(va_arg(args, int)); break;
                case 'u': print_uint(va_arg(args, unsigned int)); break;
                case 'x': print_hex(va_arg(args, unsigned int)); break;
                case 'c': terminal_putchar((char) va_arg(args, int)); break;
                case '%': terminal_putchar('%'); break;
                default: terminal_putchar('%'); terminal_putchar(fmt[i]); break;
            }
        } else {
            terminal_putchar(fmt[i]);
        }
    }

    va_end(args);
}
