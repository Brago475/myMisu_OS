#!/bin/bash
# MyMisu OS - Project Setup Script
# Run from ~/mymisu_os

echo "Setting up MyMisu OS project structure..."

# Create directories
mkdir -p src
mkdir -p iso/boot/grub
mkdir -p build

# ===== src/boot.s =====
cat > src/boot.s << 'ENDOFFILE'
/* Multiboot header constants */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* Multiboot flag field */
.set MAGIC,    0x1BADB002       /* magic number for bootloader to find */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum: magic + flags + checksum = 0 */

/* Declare the Multiboot header */
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/* Reserve a 16KB stack */
.section .bss
.align 16
stack_bottom:
.skip 16384
stack_top:

/* Kernel entry point */
.section .text
.global _start
.type _start, @function
_start:
    /* Set up the stack pointer */
    mov $stack_top, %esp

    /* Push multiboot info pointer and magic number for kernel_main */
    push %ebx    /* multiboot info struct pointer */
    push %eax    /* multiboot magic number */

    /* Call the C kernel */
    call kernel_main

    /* If kernel_main returns, hang the CPU */
    cli
hang:
    hlt
    jmp hang

.size _start, . - _start
ENDOFFILE

# ===== src/vga.h =====
cat > src/vga.h << 'ENDOFFILE'
#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <stddef.h>

/* VGA text mode dimensions */
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

/* VGA color constants */
enum vga_color {
    VGA_BLACK         = 0,
    VGA_BLUE          = 1,
    VGA_GREEN         = 2,
    VGA_CYAN          = 3,
    VGA_RED           = 4,
    VGA_MAGENTA       = 5,
    VGA_BROWN         = 6,
    VGA_LIGHT_GREY    = 7,
    VGA_DARK_GREY     = 8,
    VGA_LIGHT_BLUE    = 9,
    VGA_LIGHT_GREEN   = 10,
    VGA_LIGHT_CYAN    = 11,
    VGA_LIGHT_RED     = 12,
    VGA_LIGHT_MAGENTA = 13,
    VGA_YELLOW        = 14,
    VGA_WHITE         = 15,
};

/* Create a VGA color byte from foreground and background */
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

/* Create a VGA character entry (char + color) */
static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t) c | (uint16_t) color << 8;
}

/* Terminal functions */
void terminal_initialize(void);
void terminal_clear(void);
void terminal_setcolor(uint8_t color);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* str);
void terminal_putchar_color(char c, uint8_t color);
void terminal_update_cursor(int x, int y);

/* I/O port helpers */
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#endif
ENDOFFILE

# ===== src/vga.c =====
cat > src/vga.c << 'ENDOFFILE'
#include "vga.h"

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

/* Initialize terminal: clear screen, set defaults */
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_buffer = (uint16_t*) VGA_MEMORY;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }

    terminal_update_cursor(0, 0);
}

/* Clear the screen */
void terminal_clear(void) {
    terminal_row = 0;
    terminal_column = 0;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }

    terminal_update_cursor(0, 0);
}

/* Set the current text color */
void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

/* Scroll the screen up by one line */
static void terminal_scroll(void) {
    for (size_t y = 1; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t dst = (y - 1) * VGA_WIDTH + x;
            const size_t src = y * VGA_WIDTH + x;
            terminal_buffer[dst] = terminal_buffer[src];
        }
    }

    for (size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
}

/* Write a single character at the current cursor position */
void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
    } else if (c == '\r') {
        terminal_column = 0;
    } else if (c == '\t') {
        terminal_column = (terminal_column + 4) & ~3;
        if (terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            terminal_row++;
        }
    } else if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            const size_t index = terminal_row * VGA_WIDTH + terminal_column;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    } else {
        const size_t index = terminal_row * VGA_WIDTH + terminal_column;
        terminal_buffer[index] = vga_entry(c, terminal_color);
        terminal_column++;

        if (terminal_column >= VGA_WIDTH) {
            terminal_column = 0;
            terminal_row++;
        }
    }

    if (terminal_row >= VGA_HEIGHT) {
        terminal_scroll();
        terminal_row = VGA_HEIGHT - 1;
    }

    terminal_update_cursor(terminal_column, terminal_row);
}

/* Write a character with a specific color */
void terminal_putchar_color(char c, uint8_t color) {
    uint8_t saved = terminal_color;
    terminal_color = color;
    terminal_putchar(c);
    terminal_color = saved;
}

/* Write a buffer of known size */
void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

/* Write a null-terminated string */
void terminal_writestring(const char* str) {
    size_t i = 0;
    while (str[i] != '\0') {
        terminal_putchar(str[i]);
        i++;
    }
}

/* Move the hardware blinking cursor */
void terminal_update_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;
    outb(0x3D4, 14);
    outb(0x3D5, pos >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, pos & 0xFF);
}
ENDOFFILE

# ===== src/kernel.c =====
cat > src/kernel.c << 'ENDOFFILE'
#include "vga.h"

/* Kernel entry point - called by boot.s */
void kernel_main(unsigned long magic, unsigned long addr) {
    (void) magic;
    (void) addr;

    terminal_initialize();

    /* Boot screen logo */
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    terminal_writestring("\n");
    terminal_writestring("  __  __       __  __ _              ___  ____  \n");
    terminal_writestring(" |  \\/  |_   _|  \\/  (_)___ _   _  / _ \\/ ___| \n");
    terminal_writestring(" | |\\/| | | | | |\\/| | / __| | | || | | \\___ \\ \n");
    terminal_writestring(" | |  | | |_| | |  | | \\__ \\ |_| || |_| |___) |\n");
    terminal_writestring(" |_|  |_|\\__, |_|  |_|_|___/\\__,_| \\___/|____/ \n");
    terminal_writestring("         |___/                                  \n");

    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring("\n  MyMisu OS v0.1.0");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    terminal_writestring(" - A bare-metal x86 operating system\n");
    terminal_writestring("  Built from scratch in C | 2026\n\n");

    terminal_setcolor(vga_entry_color(VGA_DARK_GREY, VGA_BLACK));
    for (int i = 0; i < 50; i++) terminal_putchar('-');
    terminal_writestring("\n\n");

    /* Boot status */
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    terminal_writestring("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    terminal_writestring("VGA text mode initialized (80x25)\n");

    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    terminal_writestring("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    terminal_writestring("Kernel loaded at 0x00100000\n");

    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    terminal_writestring("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    terminal_writestring("Stack initialized (16KB)\n");

    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK));
    terminal_writestring("\n  [..] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    terminal_writestring("Keyboard, interrupts, shell coming in Phase 3+\n\n");

    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    terminal_writestring("  misu");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    terminal_writestring(" > ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
    terminal_writestring("Kernel halted. Waiting for Phase 3...\n");

    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}
ENDOFFILE

# ===== linker.ld =====
cat > linker.ld << 'ENDOFFILE'
ENTRY(_start)

SECTIONS
{
    . = 1M;

    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }

    _kernel_end = .;
}
ENDOFFILE

# ===== iso/boot/grub/grub.cfg =====
cat > iso/boot/grub/grub.cfg << 'ENDOFFILE'
set timeout=3
set default=0

menuentry "MyMisu OS v0.1.0" {
    multiboot /boot/mymisu.bin
    boot
}
ENDOFFILE

# ===== Makefile (uses real tabs) =====
cat > Makefile << 'ENDOFMAKE'
CC = i686-elf-gcc
AS = i686-elf-as
LD = i686-elf-gcc

CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Isrc
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

ASM_SRCS = src/boot.s
C_SRCS = src/kernel.c src/vga.c

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

# ===== .gitignore =====
cat > .gitignore << 'ENDOFFILE'
build/
*.o
*.bin
*.iso
iso/boot/mymisu.bin
ENDOFFILE

echo ""
echo "========================================="
echo "  MyMisu OS project files created!"
echo "========================================="
echo ""
echo "Files:"
echo "  src/boot.s      - Multiboot entry point"
echo "  src/vga.h       - VGA driver header"
echo "  src/vga.c       - VGA driver implementation"
echo "  src/kernel.c    - Kernel main"
echo "  linker.ld       - Linker script"
echo "  Makefile         - Build system"
echo "  iso/boot/grub/grub.cfg - GRUB config"
echo "  .gitignore"
echo ""
echo "Next: run 'make' to build!"
