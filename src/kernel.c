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
