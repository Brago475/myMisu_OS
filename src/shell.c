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
