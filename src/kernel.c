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
#include "version.h"
#include "klog.h"

void kernel_main(unsigned long magic, unsigned long addr) {
    multiboot_info_t* mbi = (multiboot_info_t*) addr;
    terminal_initialize();
    klog_init();

    gdt_init();
    klog(KLOG_OK, "GDT loaded - 5 segments");
    idt_init();
    klog(KLOG_OK, "IDT installed - 256 vectors, PIC remapped to 32-47");
    syscall_init();
    klog(KLOG_OK, "Syscalls registered - 19 calls via int 0x80");
    process_init();
    klog(KLOG_OK, "Process subsystem initialized - 16 slots");
    timer_init(100);
    klog(KLOG_OK, "Timer started - PIT 8253 at 100Hz");
    keyboard_init();
    klog(KLOG_OK, "PS/2 keyboard ready");
    if (magic == 0x2BADB002) pmm_init(mbi);
    fs_init();
    klog(KLOG_OK, "Ramdisk filesystem mounted");
    asm volatile("sti");

    login_screen();
    terminal_clear();
    /* Title */
    kprintf("\n\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
    kprintf("                              MyMisu OS\n");
    terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
    kprintf("                    bare-metal x86 operating system\n\n");

    /* System status block */
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
    kprintf("  +--------------------------------------------------------------+\n");
    kprintf("  |  System Status                                               |\n");
    kprintf("  +--------------------------------------------------------------+\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));

    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("     [OK]  ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("GDT loaded\n");

    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("     [OK]  ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("IDT installed (256 vectors, PIC remapped to 32-47)\n");

    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("     [OK]  ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Timer @ 100Hz (PIT 8253)\n");

    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("     [OK]  ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("PS/2 keyboard ready\n");

    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("     [OK]  ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Memory       %d KB (%d free pages)\n",pmm_get_total_memory_kb(),pmm_get_free_pages());

    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("     [OK]  ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Ramdisk      %d files, %d dirs (16 fds available)\n",fs_get_file_count(),fs_get_dir_count());

    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("     [OK]  ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Syscalls     19 registered (int 0x80)\n");

    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("     [OK]  ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Scheduler    round-robin, 100ms slice (preemptive)\n");

    /* Cat at bottom center */
    kprintf("\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
    kprintf("                          |\\---/|\n");
    kprintf("                          | ,_, |\n");
    kprintf("                           \\_`_/-..--.\n");
    kprintf("                        ___/ `   \' ,+ \\\n");
    kprintf("                       (__...\'  _/  |`._;\n");
    kprintf("                         (_,..\'(_,.`__)\n");

    /* Version bottom right */
    terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
    kprintf("\n                                                         %s\n", MYMISU_VERSION);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));

    process_set_scheduling_enabled(true);
    klog(KLOG_OK, "Scheduler online - round-robin preemptive 100ms");
    shell_run();
}
