#!/bin/bash
# MyMisu OS - Phase 7: Physical Memory Manager
echo "Adding Physical Memory Manager..."

# ===== src/multiboot.h =====
cat > src/multiboot.h << 'EOF'
#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

#define MULTIBOOT_MAGIC 0x2BADB002

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} __attribute__((packed)) multiboot_info_t;

typedef struct {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t;

#endif
EOF

# ===== src/pmm.h =====
cat > src/pmm.h << 'EOF'
#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include "multiboot.h"

void pmm_init(multiboot_info_t* mbi);
uint32_t pmm_alloc_page(void);
void pmm_free_page(uint32_t addr);
uint32_t pmm_get_total_pages(void);
uint32_t pmm_get_used_pages(void);
uint32_t pmm_get_free_pages(void);
uint32_t pmm_get_total_memory_kb(void);

#endif
EOF

# ===== src/pmm.c =====
cat > src/pmm.c << 'EOF'
#include "pmm.h"
#include "string.h"
#include "kprintf.h"

#define PAGE_SIZE 4096
#define MAX_PAGES 32768    /* Support up to 128MB */
#define BITMAP_SIZE (MAX_PAGES / 8)

static uint8_t bitmap[BITMAP_SIZE];
static uint32_t total_pages = 0;
static uint32_t used_pages = 0;

static inline void pmm_set_bit(uint32_t page) {
    bitmap[page / 8] |= (1 << (page % 8));
}

static inline void pmm_clear_bit(uint32_t page) {
    bitmap[page / 8] &= ~(1 << (page % 8));
}

static inline int pmm_test_bit(uint32_t page) {
    return bitmap[page / 8] & (1 << (page % 8));
}

void pmm_init(multiboot_info_t* mbi) {
    /* Mark all pages as used (safe default) */
    memset(bitmap, 0xFF, BITMAP_SIZE);
    used_pages = MAX_PAGES;

    /* Use mem_upper from multiboot (KB above 1MB) */
    uint32_t total_kb = mbi->mem_lower + mbi->mem_upper + 1024;
    total_pages = total_kb / 4;  /* 4KB per page */
    if (total_pages > MAX_PAGES) total_pages = MAX_PAGES;

    /* Walk the multiboot memory map */
    if (mbi->flags & (1 << 6)) {
        multiboot_mmap_entry_t* entry = (multiboot_mmap_entry_t*) mbi->mmap_addr;
        uint32_t mmap_end = mbi->mmap_addr + mbi->mmap_length;

        while ((uint32_t)entry < mmap_end) {
            if (entry->type == 1) {  /* Available RAM */
                uint32_t base = (uint32_t) entry->addr;
                uint32_t length = (uint32_t) entry->len;

                /* Align to page boundaries */
                uint32_t start = (base + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
                uint32_t end = (base + length) & ~(PAGE_SIZE - 1);

                for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
                    uint32_t page = addr / PAGE_SIZE;
                    if (page < MAX_PAGES && page > 0) {
                        pmm_clear_bit(page);
                        used_pages--;
                    }
                }
            }
            entry = (multiboot_mmap_entry_t*)((uint32_t)entry + entry->size + sizeof(entry->size));
        }
    }

    /* Reserve first 1MB (BIOS, bootloader, etc.) */
    for (uint32_t i = 0; i < 256; i++) {  /* 256 pages = 1MB */
        if (!pmm_test_bit(i)) {
            pmm_set_bit(i);
            used_pages++;
        }
    }

    /* Reserve kernel pages (1MB to _kernel_end) */
    extern uint32_t _kernel_end;
    uint32_t kernel_end_page = ((uint32_t)&_kernel_end) / PAGE_SIZE + 1;
    for (uint32_t i = 256; i <= kernel_end_page && i < MAX_PAGES; i++) {
        if (!pmm_test_bit(i)) {
            pmm_set_bit(i);
            used_pages++;
        }
    }
}

uint32_t pmm_alloc_page(void) {
    for (uint32_t i = 0; i < MAX_PAGES / 8; i++) {
        if (bitmap[i] != 0xFF) {
            for (uint8_t bit = 0; bit < 8; bit++) {
                if (!(bitmap[i] & (1 << bit))) {
                    uint32_t page = i * 8 + bit;
                    pmm_set_bit(page);
                    used_pages++;
                    return page * PAGE_SIZE;
                }
            }
        }
    }
    return 0;  /* Out of memory */
}

void pmm_free_page(uint32_t addr) {
    uint32_t page = addr / PAGE_SIZE;
    if (page < MAX_PAGES && pmm_test_bit(page)) {
        pmm_clear_bit(page);
        used_pages--;
    }
}

uint32_t pmm_get_total_pages(void) { return total_pages; }
uint32_t pmm_get_used_pages(void)  { return used_pages; }
uint32_t pmm_get_free_pages(void)  { return total_pages > used_pages ? total_pages - used_pages : 0; }
uint32_t pmm_get_total_memory_kb(void) { return total_pages * 4; }
EOF

# ===== Update src/kernel.c =====
cat > src/kernel.c << 'EOF'
#include "vga.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "keyboard.h"
#include "kprintf.h"
#include "shell.h"
#include "pmm.h"
#include "multiboot.h"

void kernel_main(unsigned long magic, unsigned long addr) {
    multiboot_info_t* mbi = (multiboot_info_t*) addr;

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
    kprintf("\n  MyMisu OS v0.2.0");
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

    /* Initialize physical memory manager */
    if (magic == 0x2BADB002) {
        pmm_init(mbi);
        terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
        kprintf("  [OK] ");
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
        kprintf("Physical memory: %d KB total, %d KB free (%d pages)\n",
                pmm_get_total_memory_kb(),
                pmm_get_free_pages() * 4,
                pmm_get_free_pages());
    } else {
        terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK));
        kprintf("  [!!] ");
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
        kprintf("Multiboot magic invalid - PMM skipped\n");
    }

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

# ===== Update src/shell.c — add meminfo command =====
cat > src/shell.c << 'EOF'
#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "kprintf.h"
#include "string.h"
#include "timer.h"
#include "pmm.h"

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

    const char* cmds[][2] = {
        {"help",    "Show this help message"},
        {"clear",   "Clear the screen"},
        {"echo",    "Echo text back to screen"},
        {"info",    "Show system information"},
        {"meminfo", "Show memory statistics"},
        {"alloc",   "Allocate a memory page (demo)"},
        {"free",    "Free a memory page (usage: free <addr>)"},
        {"time",    "Show uptime"},
        {"color",   "Change colors (usage: color <fg> <bg>)"},
        {"reboot",  "Reboot the system"},
        {"panic",   "Trigger a kernel panic (test)"},
        {0, 0}
    };

    for (int i = 0; cmds[i][0]; i++) {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
        kprintf("  %-9s", cmds[i][0]);
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
        kprintf("- %s\n", cmds[i][1]);
    }
    kprintf("\n");
}

static void cmd_info(void) {
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("\n  MyMisu OS v0.2.0\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("  Architecture:  x86 (i686)\n");
    kprintf("  Display:       VGA text mode 80x25\n");
    kprintf("  Keyboard:      PS/2 (US QWERTY)\n");
    kprintf("  Timer:         PIT at 100 Hz\n");
    kprintf("  Memory:        %d KB total, %d KB free\n",
            pmm_get_total_memory_kb(), pmm_get_free_pages() * 4);
    kprintf("  Built with:    GCC 13.2.0 (i686-elf)\n");
    kprintf("  Built by:      James Mardi, Danny + AI\n");
    kprintf("\n");
}

static void cmd_meminfo(void) {
    uint32_t total = pmm_get_total_pages();
    uint32_t used = pmm_get_used_pages();
    uint32_t free_p = pmm_get_free_pages();

    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("\n  Memory Statistics:\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("  Total memory:  %d KB (%d MB)\n", total * 4, total * 4 / 1024);
    kprintf("  Total pages:   %d (4 KB each)\n", total);

    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  Free pages:    %d (%d KB)\n", free_p, free_p * 4);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
    kprintf("  Used pages:    %d (%d KB)\n", used, used * 4);

    /* Visual bar */
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("  Usage:         [");
    uint32_t bar_width = 40;
    uint32_t filled = (total > 0) ? (used * bar_width / total) : 0;
    for (uint32_t i = 0; i < bar_width; i++) {
        if (i < filled) {
            terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
            kprintf("#");
        } else {
            terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
            kprintf("-");
        }
    }
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    uint32_t pct = (total > 0) ? (used * 100 / total) : 0;
    kprintf("] %d%%\n\n", pct);
}

static void cmd_alloc(void) {
    uint32_t addr = pmm_alloc_page();
    if (addr) {
        terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
        kprintf("  Allocated page at 0x%x (%d KB)\n", addr, addr / 1024);
    } else {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        kprintf("  Out of memory!\n");
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_free(int argc, char** argv) {
    if (argc < 2) {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
        kprintf("  Usage: free <address_in_decimal>\n");
        kprintf("  (Use 'alloc' first to get an address)\n");
        return;
    }
    uint32_t addr = (uint32_t) atoi(argv[1]);
    pmm_free_page(addr);
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  Freed page at 0x%x\n", addr);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
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
    struct { uint16_t limit; uint32_t base; } __attribute__((packed)) null_idt = {0, 0};
    asm volatile ("lidt %0" : : "m"(null_idt));
    asm volatile ("int $0x03");
}

static void cmd_panic(void) {
    volatile int x = 0;
    volatile int y = 1 / x;
    (void) y;
}

static void execute_command(char* input) {
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

    if (strcmp(argv[0], "help") == 0) cmd_help();
    else if (strcmp(argv[0], "clear") == 0) terminal_clear();
    else if (strcmp(argv[0], "echo") == 0) cmd_echo(argc, argv);
    else if (strcmp(argv[0], "info") == 0) cmd_info();
    else if (strcmp(argv[0], "meminfo") == 0) cmd_meminfo();
    else if (strcmp(argv[0], "alloc") == 0) cmd_alloc();
    else if (strcmp(argv[0], "free") == 0) cmd_free(argc, argv);
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

# ===== Update Makefile =====
cat > Makefile << 'ENDOFMAKE'
CC = i686-elf-gcc
AS = i686-elf-as
LD = i686-elf-gcc

CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -Isrc
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

ASM_SRCS = src/boot.s src/gdt_flush.s src/isr.s
C_SRCS = src/kernel.c src/vga.c src/gdt.c src/idt.c src/timer.c \
         src/keyboard.c src/string.c src/kprintf.c src/shell.c src/pmm.c

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
echo "  Phase 7: Memory Manager added!"
echo "========================================="
echo ""
echo "New files:"
echo "  src/multiboot.h  - Multiboot info structures"
echo "  src/pmm.c/h      - Physical memory manager"
echo ""
echo "Updated:"
echo "  src/kernel.c      - Initializes PMM, shows memory at boot"
echo "  src/shell.c       - Added meminfo, alloc, free commands"
echo "  Makefile           - Builds pmm.c"
echo ""
echo "New commands:"
echo "  meminfo  - Show memory stats with visual bar"
echo "  alloc    - Allocate a 4KB page (demo)"
echo "  free     - Free an allocated page"
echo ""
echo "Run 'make clean && make && make run' to test!"
