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
