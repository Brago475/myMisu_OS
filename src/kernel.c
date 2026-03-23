#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "kprintf.h"
#include "shell.h"
#include "pmm.h"
#include "multiboot.h"
#include "syscall.h"
#include "fs.h"
#include "process.h"

void kernel_main(unsigned long magic, unsigned long addr) {
    multiboot_info_t* mbi = (multiboot_info_t*) addr;

    terminal_initialize();

    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("\n");
    kprintf("  __  __       __  __ _              ___  ____  \n");
    kprintf(" |  \\/  |_   _|  \\/  (_)___ _   _  / _ \\/ ___| \n");
    kprintf(" | |\\/| | | | | |\\/| | / __| | | || | | \\___ \\ \n");
    kprintf(" | |  | | |_| | |  | | \\__ \\ |_| || |_| |___) |\n");
    kprintf(" |_|  |_|\\__, |_|  |_|_|___/\\__,_| \\___/|____/ \n");
    kprintf("         |___/                                  \n");

    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("\n  MyMisu OS v0.3.0");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf(" - A bare-metal x86 operating system\n\n");

    gdt_init();
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("GDT initialized\n");

    idt_init();
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("IDT initialized (256 entries)\n");

    syscall_init();
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("System calls registered (int 0x80)\n");

    process_init();
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Process manager initialized\n");

    timer_init(100);
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("PIT timer at 100 Hz\n");

    keyboard_init();
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("PS/2 keyboard driver loaded\n");

    if (magic == 0x2BADB002) {
        pmm_init(mbi);
        terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
        kprintf("  [OK] ");
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
        kprintf("Physical memory: %d KB (%d pages free)\n",
                pmm_get_total_memory_kb(), pmm_get_free_pages());
    }

    fs_init();
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Ramdisk filesystem: %d files, %d dirs\n",
            fs_get_file_count(), fs_get_dir_count());

    asm volatile("sti");
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  [OK] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Interrupts enabled\n");

    shell_run();
}
