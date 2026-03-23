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

    gdt_init();
    idt_init();
    syscall_init();
    process_init();
    timer_init(100);
    keyboard_init();
    if (magic == 0x2BADB002) pmm_init(mbi);
    fs_init();
    asm volatile("sti");

    login_screen();

    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("\n");
    kprintf("  __  __       __  __ _              ___  ____  \n");
    kprintf(" |  \\/  |_   _|  \\/  (_)___ _   _  / _ \\/ ___| \n");
    kprintf(" | |\\/| | | | | |\\/| | / __| | | || | | \\___ \\ \n");
    kprintf(" | |  | | |_| | |  | | \\__ \\ |_| || |_| |___) |\n");
    kprintf(" |_|  |_|\\__, |_|  |_|_|___/\\__,_| \\___/|____/ \n");
    kprintf("         |___/\n\n");

    kprintf("                  |\\---/|\n");
    kprintf("                  | ,_, |\n");
    kprintf("                   \\_`_/-..--.  \n");
    kprintf("                ___/ `   ' ,+ \\\n");
    kprintf("               (__...'  _/  |`._;  \n");
    kprintf("                 (_,..'(_,.`__)  \n");

    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("\n  v0.5.0");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf(" | bare-metal x86 | Built with AI\n\n");

    terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  [OK] ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("All systems initialized\n");
    terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  [OK] ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Memory: %d KB (%d pages free)\n",pmm_get_total_memory_kb(),pmm_get_free_pages());
    terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  [OK] ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Ramdisk: %d files, %d dirs\n",fs_get_file_count(),fs_get_dir_count());

    shell_run();
}
