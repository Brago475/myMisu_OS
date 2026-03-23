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
