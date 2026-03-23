#!/bin/bash
# MyMisu OS - Phase 3-8 Setup Script
# Run from ~/mymisu_os
echo "Adding Phase 3-8: GDT, IDT, Keyboard, Utils, Memory, Shell..."

# ===== src/ports.h =====
cat > src/ports.h << 'EOF'
#ifndef PORTS_H
#define PORTS_H

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

#endif
EOF

# ===== src/string.h =====
cat > src/string.h << 'EOF'
#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

size_t strlen(const char* str);
int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, size_t n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);
void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, const void* src, size_t num);
int memcmp(const void* a, const void* b, size_t num);
int atoi(const char* str);

#endif
EOF

# ===== src/string.c =====
cat > src/string.c << 'EOF'
#include "string.h"

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;
}

int strncmp(const char* a, const char* b, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) return (unsigned char)a[i] - (unsigned char)b[i];
        if (a[i] == '\0') return 0;
    }
    return 0;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) dest[i] = src[i];
    for (; i < n; i++) dest[i] = '\0';
    return dest;
}

char* strcat(char* dest, const char* src) {
    char* d = dest + strlen(dest);
    while ((*d++ = *src++));
    return dest;
}

void* memset(void* ptr, int value, size_t num) {
    unsigned char* p = (unsigned char*) ptr;
    for (size_t i = 0; i < num; i++) p[i] = (unsigned char) value;
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t num) {
    unsigned char* d = (unsigned char*) dest;
    const unsigned char* s = (const unsigned char*) src;
    for (size_t i = 0; i < num; i++) d[i] = s[i];
    return dest;
}

int memcmp(const void* a, const void* b, size_t num) {
    const unsigned char* pa = a;
    const unsigned char* pb = b;
    for (size_t i = 0; i < num; i++) {
        if (pa[i] != pb[i]) return pa[i] - pb[i];
    }
    return 0;
}

int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    while (*str == ' ') str++;
    if (*str == '-') { sign = -1; str++; }
    else if (*str == '+') { str++; }
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return sign * result;
}
EOF

# ===== src/kprintf.h =====
cat > src/kprintf.h << 'EOF'
#ifndef KPRINTF_H
#define KPRINTF_H

void kprintf(const char* fmt, ...);

#endif
EOF

# ===== src/kprintf.c =====
cat > src/kprintf.c << 'EOF'
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
EOF

# ===== src/gdt.h =====
cat > src/gdt.h << 'EOF'
#ifndef GDT_H
#define GDT_H

#include <stdint.h>

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void gdt_init(void);
extern void gdt_flush(uint32_t);

#endif
EOF

# ===== src/gdt.c =====
cat > src/gdt.c << 'EOF'
#include "gdt.h"
#include "string.h"

static struct gdt_entry gdt_entries[3];
static struct gdt_ptr gp;

static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low    = base & 0xFFFF;
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high   = (base >> 24) & 0xFF;
    gdt_entries[num].limit_low   = limit & 0xFFFF;
    gdt_entries[num].granularity  = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt_entries[num].access      = access;
}

void gdt_init(void) {
    gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
    gp.base  = (uint32_t) &gdt_entries;

    gdt_set_gate(0, 0, 0, 0, 0);                /* Null segment */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); /* Kernel code: exec/read */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); /* Kernel data: read/write */

    gdt_flush((uint32_t) &gp);
}
EOF

# ===== src/gdt_flush.s =====
cat > src/gdt_flush.s << 'EOF'
.global gdt_flush
.type gdt_flush, @function
gdt_flush:
    mov 4(%esp), %eax    /* Get pointer to GDT ptr struct */
    lgdt (%eax)          /* Load GDT */

    mov $0x10, %ax       /* Kernel data segment = 0x10 */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    ljmp $0x08, $.flush  /* Far jump to kernel code segment = 0x08 */
.flush:
    ret
EOF

# ===== src/idt.h =====
cat > src/idt.h << 'EOF'
#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

typedef void (*isr_handler_t)(registers_t*);
void register_interrupt_handler(uint8_t n, isr_handler_t handler);

#endif
EOF

# ===== src/idt.c =====
cat > src/idt.c << 'EOF'
#include "idt.h"
#include "string.h"
#include "ports.h"
#include "kprintf.h"
#include "vga.h"

static struct idt_entry idt_entries[256];
static struct idt_ptr idtp;
static isr_handler_t interrupt_handlers[256];

extern void idt_flush(uint32_t);

/* Defined in isr.s */
extern void isr0(void);  extern void isr1(void);  extern void isr2(void);
extern void isr3(void);  extern void isr4(void);  extern void isr5(void);
extern void isr6(void);  extern void isr7(void);  extern void isr8(void);
extern void isr9(void);  extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void);
extern void isr15(void); extern void isr16(void); extern void isr17(void);
extern void isr18(void); extern void isr19(void); extern void isr20(void);
extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void);
extern void isr27(void); extern void isr28(void); extern void isr29(void);
extern void isr30(void); extern void isr31(void);

extern void irq0(void);  extern void irq1(void);  extern void irq2(void);
extern void irq3(void);  extern void irq4(void);  extern void irq5(void);
extern void irq6(void);  extern void irq7(void);  extern void irq8(void);
extern void irq9(void);  extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void); extern void irq14(void);
extern void irq15(void);

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_low  = base & 0xFFFF;
    idt_entries[num].base_high = (base >> 16) & 0xFFFF;
    idt_entries[num].sel       = sel;
    idt_entries[num].always0   = 0;
    idt_entries[num].flags     = flags;
}

void register_interrupt_handler(uint8_t n, isr_handler_t handler) {
    interrupt_handlers[n] = handler;
}

static void pic_remap(void) {
    outb(0x20, 0x11); io_wait();
    outb(0xA0, 0x11); io_wait();
    outb(0x21, 0x20); io_wait();  /* Master PIC: IRQs start at IDT 32 */
    outb(0xA1, 0x28); io_wait();  /* Slave PIC: IRQs start at IDT 40 */
    outb(0x21, 0x04); io_wait();
    outb(0xA1, 0x02); io_wait();
    outb(0x21, 0x01); io_wait();
    outb(0xA1, 0x01); io_wait();
    outb(0x21, 0x0);              /* Unmask all master IRQs */
    outb(0xA1, 0x0);              /* Unmask all slave IRQs */
}

void idt_init(void) {
    idtp.limit = sizeof(struct idt_entry) * 256 - 1;
    idtp.base  = (uint32_t) &idt_entries;
    memset(&idt_entries, 0, sizeof(struct idt_entry) * 256);
    memset(&interrupt_handlers, 0, sizeof(isr_handler_t) * 256);

    pic_remap();

    /* CPU exceptions 0-31 */
    idt_set_gate(0,  (uint32_t)isr0,  0x08, 0x8E);
    idt_set_gate(1,  (uint32_t)isr1,  0x08, 0x8E);
    idt_set_gate(2,  (uint32_t)isr2,  0x08, 0x8E);
    idt_set_gate(3,  (uint32_t)isr3,  0x08, 0x8E);
    idt_set_gate(4,  (uint32_t)isr4,  0x08, 0x8E);
    idt_set_gate(5,  (uint32_t)isr5,  0x08, 0x8E);
    idt_set_gate(6,  (uint32_t)isr6,  0x08, 0x8E);
    idt_set_gate(7,  (uint32_t)isr7,  0x08, 0x8E);
    idt_set_gate(8,  (uint32_t)isr8,  0x08, 0x8E);
    idt_set_gate(9,  (uint32_t)isr9,  0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

    /* Hardware IRQs 0-15 mapped to IDT 32-47 */
    idt_set_gate(32, (uint32_t)irq0,  0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1,  0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2,  0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3,  0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4,  0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5,  0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6,  0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7,  0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8,  0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9,  0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);

    idt_flush((uint32_t) &idtp);
}

/* Called from isr_common in isr.s */
void isr_handler(registers_t* regs) {
    if (interrupt_handlers[regs->int_no]) {
        interrupt_handlers[regs->int_no](regs);
    } else if (regs->int_no < 32) {
        terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_RED));
        kprintf("\n  KERNEL PANIC: Exception %d  \n", regs->int_no);
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
        for(;;) asm volatile("hlt");
    }
}

/* Called from irq_common in isr.s */
void irq_handler(registers_t* regs) {
    /* Send EOI to slave PIC if needed */
    if (regs->int_no >= 40) {
        outb(0xA0, 0x20);
    }
    /* Send EOI to master PIC */
    outb(0x20, 0x20);

    if (interrupt_handlers[regs->int_no]) {
        interrupt_handlers[regs->int_no](regs);
    }
}
EOF

# ===== src/isr.s =====
cat > src/isr.s << 'EOF'
/* ISR stubs for CPU exceptions 0-31 */

.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    cli
    push $0          /* Dummy error code */
    push $\num       /* Interrupt number */
    jmp isr_common
.endm

.macro ISR_ERRCODE num
.global isr\num
isr\num:
    cli
    push $\num       /* Interrupt number (error code already pushed by CPU) */
    jmp isr_common
.endm

.macro IRQ irq_num, int_num
.global irq\irq_num
irq\irq_num:
    cli
    push $0
    push $\int_num
    jmp irq_common
.endm

/* CPU Exceptions */
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_ERRCODE   30
ISR_NOERRCODE 31

/* Hardware IRQs */
IRQ 0,  32
IRQ 1,  33
IRQ 2,  34
IRQ 3,  35
IRQ 4,  36
IRQ 5,  37
IRQ 6,  38
IRQ 7,  39
IRQ 8,  40
IRQ 9,  41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

/* Common ISR handler */
.extern isr_handler
isr_common:
    pusha            /* Push all registers */
    mov %ds, %ax
    push %eax        /* Save data segment */

    mov $0x10, %ax   /* Load kernel data segment */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp        /* Push pointer to registers_t */
    call isr_handler
    add $4, %esp

    pop %eax         /* Restore data segment */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popa
    add $8, %esp     /* Remove int_no and err_code */
    sti
    iret

/* Common IRQ handler */
.extern irq_handler
irq_common:
    pusha
    mov %ds, %ax
    push %eax

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    push %esp
    call irq_handler
    add $4, %esp

    pop %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    popa
    add $8, %esp
    sti
    iret

/* IDT flush */
.global idt_flush
idt_flush:
    mov 4(%esp), %eax
    lidt (%eax)
    ret
EOF

# ===== src/timer.h =====
cat > src/timer.h << 'EOF'
#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void timer_init(uint32_t frequency);
uint32_t timer_get_ticks(void);

#endif
EOF

# ===== src/timer.c =====
cat > src/timer.c << 'EOF'
#include "timer.h"
#include "idt.h"
#include "ports.h"

static uint32_t tick = 0;

static void timer_callback(registers_t* regs) {
    (void) regs;
    tick++;
}

void timer_init(uint32_t frequency) {
    register_interrupt_handler(32, timer_callback);

    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

uint32_t timer_get_ticks(void) {
    return tick;
}
EOF

# ===== src/keyboard.h =====
cat > src/keyboard.h << 'EOF'
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

void keyboard_init(void);
char keyboard_getchar(void);
bool keyboard_haschar(void);

#endif
EOF

# ===== src/keyboard.c =====
cat > src/keyboard.c << 'EOF'
#include "keyboard.h"
#include "idt.h"
#include "ports.h"

#define KB_BUFFER_SIZE 256

static char kb_buffer[KB_BUFFER_SIZE];
static volatile uint16_t kb_read = 0;
static volatile uint16_t kb_write = 0;
static bool shift_pressed = false;
static bool caps_lock = false;

/* US QWERTY scancode to ASCII (Set 1) */
static const char scancode_normal[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,  'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,  '\\','z','x','c','v','b','n','m',',','.','/',0,
    '*', 0, ' '
};

static const char scancode_shift[128] = {
    0,  27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,  'A','S','D','F','G','H','J','K','L',':','"','~',
    0,  '|','Z','X','C','V','B','N','M','<','>','?',0,
    '*', 0, ' '
};

static void keyboard_callback(registers_t* regs) {
    (void) regs;
    uint8_t scancode = inb(0x60);

    /* Key release (high bit set) */
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) shift_pressed = false;
        return;
    }

    /* Special keys */
    if (scancode == 0x2A || scancode == 0x36) { shift_pressed = true; return; }
    if (scancode == 0x3A) { caps_lock = !caps_lock; return; }

    /* Convert scancode to ASCII */
    char c = 0;
    if (scancode < 128) {
        bool use_shift = shift_pressed;
        /* Caps lock only affects letters */
        if (caps_lock && scancode_normal[scancode] >= 'a' && scancode_normal[scancode] <= 'z')
            use_shift = !use_shift;

        c = use_shift ? scancode_shift[scancode] : scancode_normal[scancode];
    }

    if (c == 0) return;

    /* Push into ring buffer */
    uint16_t next = (kb_write + 1) % KB_BUFFER_SIZE;
    if (next != kb_read) {
        kb_buffer[kb_write] = c;
        kb_write = next;
    }
}

void keyboard_init(void) {
    register_interrupt_handler(33, keyboard_callback);
}

bool keyboard_haschar(void) {
    return kb_read != kb_write;
}

char keyboard_getchar(void) {
    while (kb_read == kb_write) {
        asm volatile("hlt");  /* Wait for interrupt */
    }
    char c = kb_buffer[kb_read];
    kb_read = (kb_read + 1) % KB_BUFFER_SIZE;
    return c;
}
EOF

# ===== src/shell.h =====
cat > src/shell.h << 'EOF'
#ifndef SHELL_H
#define SHELL_H

void shell_init(void);
void shell_run(void);

#endif
EOF

# ===== src/shell.c =====
cat > src/shell.c << 'EOF'
#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "kprintf.h"
#include "string.h"
#include "timer.h"

#define CMD_BUFFER_SIZE 256
#define MAX_ARGS 16

static char cmd_buffer[CMD_BUFFER_SIZE];
static int cmd_len = 0;

static void print_prompt(void) {
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("misu");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf(" > ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
}

static void cmd_help(void) {
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("\n  Available commands:\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  help     ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("- Show this help message\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  clear    ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("- Clear the screen\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  echo     ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("- Echo text back to screen\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  info     ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("- Show system information\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  time     ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("- Show uptime\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  color    ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("- Change colors (usage: color <fg> <bg>)\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  reboot   ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("- Reboot the system\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  panic    ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("- Trigger a kernel panic (test)\n");
    kprintf("\n");
}

static void cmd_info(void) {
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("\n  MyMisu OS v0.1.0\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("  Architecture:  x86 (i686)\n");
    kprintf("  Display:       VGA text mode 80x25\n");
    kprintf("  Keyboard:      PS/2 (US QWERTY)\n");
    kprintf("  Timer:         PIT at 100 Hz\n");
    kprintf("  Built with:    GCC 13.2.0 (i686-elf)\n");
    kprintf("  Built by:      James Mardi, Danny + AI\n");
    kprintf("\n");
}

static void cmd_time(void) {
    uint32_t ticks = timer_get_ticks();
    uint32_t seconds = ticks / 100;
    uint32_t minutes = seconds / 60;
    seconds = seconds % 60;

    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("\n  Uptime: %d min %d sec (%d ticks)\n\n", minutes, seconds, ticks);
}

static void cmd_echo(int argc, char** argv) {
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    for (int i = 1; i < argc; i++) {
        kprintf("%s", argv[i]);
        if (i < argc - 1) kprintf(" ");
    }
    kprintf("\n");
}

static void cmd_color(int argc, char** argv) {
    if (argc < 3) {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
        kprintf("\n  Usage: color <fg 0-15> <bg 0-15>\n");
        kprintf("  Colors: 0=black 1=blue 2=green 3=cyan 4=red\n");
        kprintf("          5=magenta 6=brown 7=lgrey 8=dgrey\n");
        kprintf("          9=lblue 10=lgreen 11=lcyan 12=lred\n");
        kprintf("          13=lmagenta 14=yellow 15=white\n\n");
        return;
    }
    int fg = atoi(argv[1]);
    int bg = atoi(argv[2]);
    if (fg < 0 || fg > 15 || bg < 0 || bg > 15) {
        kprintf("  Invalid color (0-15)\n");
        return;
    }
    terminal_setcolor(vga_entry_color((uint8_t)fg, (uint8_t)bg));
    kprintf("\n  Color changed!\n\n");
}

static void cmd_reboot(void) {
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK));
    kprintf("\n  Rebooting...\n");
    /* Triple fault: load null IDT and trigger interrupt */
    struct { uint16_t limit; uint32_t base; } __attribute__((packed)) null_idt = {0, 0};
    asm volatile ("lidt %0" : : "m"(null_idt));
    asm volatile ("int $0x03");
}

static void cmd_panic(void) {
    /* Trigger division by zero */
    volatile int x = 0;
    volatile int y = 1 / x;
    (void) y;
}

static void execute_command(char* input) {
    /* Parse into argv */
    char* argv[MAX_ARGS];
    int argc = 0;

    char* token = input;
    while (*token && argc < MAX_ARGS) {
        while (*token == ' ') token++;
        if (*token == '\0') break;

        argv[argc++] = token;
        while (*token && *token != ' ') token++;
        if (*token) { *token = '\0'; token++; }
    }

    if (argc == 0) return;

    /* Dispatch */
    if (strcmp(argv[0], "help") == 0) cmd_help();
    else if (strcmp(argv[0], "clear") == 0) { terminal_clear(); }
    else if (strcmp(argv[0], "echo") == 0) cmd_echo(argc, argv);
    else if (strcmp(argv[0], "info") == 0) cmd_info();
    else if (strcmp(argv[0], "time") == 0) cmd_time();
    else if (strcmp(argv[0], "color") == 0) cmd_color(argc, argv);
    else if (strcmp(argv[0], "reboot") == 0) cmd_reboot();
    else if (strcmp(argv[0], "panic") == 0) cmd_panic();
    else {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        kprintf("  Unknown command: %s\n", argv[0]);
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
        kprintf("  Type 'help' for available commands.\n");
    }
}

void shell_init(void) {
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("\n  Type 'help' for a list of commands.\n\n");
}

void shell_run(void) {
    shell_init();

    while (1) {
        print_prompt();
        cmd_len = 0;

        /* Read a line */
        while (1) {
            char c = keyboard_getchar();

            if (c == '\n') {
                terminal_putchar('\n');
                cmd_buffer[cmd_len] = '\0';
                break;
            } else if (c == '\b') {
                if (cmd_len > 0) {
                    cmd_len--;
                    terminal_putchar('\b');
                }
            } else if (cmd_len < CMD_BUFFER_SIZE - 1) {
                cmd_buffer[cmd_len++] = c;
                terminal_putchar(c);
            }
        }

        if (cmd_len > 0) {
            execute_command(cmd_buffer);
        }
    }
}
EOF

# ===== Update src/vga.h — remove duplicate outb/inb (now in ports.h) =====
# We'll keep them in vga.h too for backward compat, it won't hurt (static inline)

# ===== Update src/kernel.c =====
cat > src/kernel.c << 'EOF'
#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "kprintf.h"
#include "shell.h"

void kernel_main(unsigned long magic, unsigned long addr) {
    (void) magic;
    (void) addr;

    /* Initialize VGA */
    terminal_initialize();

    /* Boot screen logo */
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("\n");
    kprintf("  __  __       __  __ _              ___  ____  \n");
    kprintf(" |  \\/  |_   _|  \\/  (_)___ _   _  / _ \\/ ___| \n");
    kprintf(" | |\\/| | | | | |\\/| | / __| | | || | | \\___ \\ \n");
    kprintf(" | |  | | |_| | |  | | \\__ \\ |_| || |_| |___) |\n");
    kprintf(" |_|  |_|\\__, |_|  |_|_|___/\\__,_| \\___/|____/ \n");
    kprintf("         |___/                                  \n");

    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("\n  MyMisu OS v0.1.0");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf(" - A bare-metal x86 operating system\n\n");

    /* Initialize GDT */
    gdt_init();
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("GDT initialized\n");

    /* Initialize IDT */
    idt_init();
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("IDT initialized (256 entries)\n");

    /* Initialize timer at 100Hz */
    timer_init(100);
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("PIT timer at 100 Hz\n");

    /* Initialize keyboard */
    keyboard_init();
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("PS/2 keyboard driver loaded\n");

    /* Enable interrupts */
    asm volatile("sti");
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Interrupts enabled\n");

    /* Launch shell */
    shell_run();
}
EOF

# ===== Update Makefile =====
cat > Makefile << 'ENDOFMAKE'
CC = i686-elf-gcc
AS = i686-elf-as
LD = i686-elf-gcc

CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Isrc
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

ASM_SRCS = src/boot.s src/gdt_flush.s src/isr.s
C_SRCS = src/kernel.c src/vga.c src/gdt.c src/idt.c src/timer.c \
         src/keyboard.c src/string.c src/kprintf.c src/shell.c

ASM_OBJS = $(ASM_SRCS:.s=.o)
C_OBJS = $(C_SRCS:.c=.o)
OBJS = $(ASM_OBJS) $(C_OBJS)

KERNEL = build/mymisu.bin
ISO = build/mymisu.iso

.PHONY: all clean run debug

all: $(ISO)

%.o: %.s
	$(AS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KERNEL): $(OBJS)
	@mkdir -p build
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo ""
	@echo "=== Kernel linked: $(KERNEL) ==="

$(ISO): $(KERNEL)
	@mkdir -p iso/boot
	cp $(KERNEL) iso/boot/mymisu.bin
	grub-mkrescue -o $(ISO) iso/ 2>/dev/null
	@echo "=== ISO created: $(ISO) ==="
	@echo ""
	@echo "Run 'make run' to boot in QEMU!"

run: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 128M

debug: $(ISO)
	qemu-system-i386 -cdrom $(ISO) -m 128M -s -S &
	@echo "Run: i686-elf-gdb build/mymisu.bin -ex 'target remote :1234'"

clean:
	rm -f $(OBJS)
	rm -rf build/
	rm -f iso/boot/mymisu.bin
	@echo "Cleaned!"
ENDOFMAKE

echo ""
echo "========================================="
echo "  Phase 3-8 files added!"
echo "========================================="
echo ""
echo "New files:"
echo "  src/ports.h       - I/O port helpers"
echo "  src/string.c/h    - String/memory functions"
echo "  src/kprintf.c/h   - Formatted printing"
echo "  src/gdt.c/h       - Global Descriptor Table"
echo "  src/gdt_flush.s   - GDT assembly loader"
echo "  src/idt.c/h       - Interrupt Descriptor Table"
echo "  src/isr.s         - ISR/IRQ assembly stubs"
echo "  src/timer.c/h     - PIT timer driver"
echo "  src/keyboard.c/h  - PS/2 keyboard driver"
echo "  src/shell.c/h     - Interactive shell"
echo ""
echo "Updated:"
echo "  src/kernel.c      - Now initializes all subsystems"
echo "  Makefile           - Builds all new files"
echo ""
echo "Run 'make clean && make && make run' to test!"
