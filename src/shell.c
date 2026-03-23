#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "kprintf.h"
#include "string.h"
#include "timer.h"
#include "pmm.h"
#include "fs.h"
#include "process.h"
#include "syscall.h"

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
    kprintf("\n  Available commands:\n\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  help      ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Show this help message\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  clear     ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Clear the screen\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  echo      ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Echo text back to screen\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  info      ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Show system information\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  neofetch  ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("System info with ASCII art\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  meminfo   ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Show memory statistics\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  alloc     ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Allocate a memory page\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  time      ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Show uptime\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  ls        ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("List files and directories\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  cat       ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Read a file (cat <name>)\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  touch     ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Create a file (touch <name>)\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  mkdir     ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Create a directory\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  write     ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Write to file (write <n> <text>)\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  rm        ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Delete a file or directory\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  ps        ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("List running processes\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  spawn     ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Create a process (spawn <name>)\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  kill      ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Kill a process (kill <pid>)\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  syscall   ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Demo system calls live\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  color     ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Change colors (color <fg> <bg>)\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  reboot    ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Reboot the system\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("  panic     ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("Trigger a kernel panic\n\n");
}

static void cmd_neofetch(void) {
    uint32_t ticks = timer_get_ticks();
    uint32_t seconds = ticks / 100;
    uint32_t minutes = seconds / 60;
    seconds %= 60;
    kprintf("\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("       /\\_/\\    ");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("misu@misu-pc\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("      ( o.o )   ");
    terminal_setcolor(vga_entry_color(VGA_DARK_GREY, VGA_BLACK));
    kprintf("----------------\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("       > ^ <    ");
    kprintf("OS:      ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("MyMisu OS v0.3.0\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("      /|   |\\   ");
    kprintf("Kernel:  ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("MisuKernel i686\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("     (_|   |_)  ");
    kprintf("Uptime:  ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("%d min %d sec\n", minutes, seconds);
    kprintf("                ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("Shell:   ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("misu-sh 1.0\n");
    kprintf("                ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("Memory:  ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("%d KB / %d KB\n", pmm_get_used_pages() * 4, pmm_get_total_memory_kb());
    kprintf("                ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("Procs:   ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("%d running\n", process_get_count());
    kprintf("                ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("Files:   ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("%d files, %d dirs\n", fs_get_file_count(), fs_get_dir_count());
    kprintf("                ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("Display: ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("VGA 80x25 (text)\n");
    kprintf("                ");
    for (int i = 0; i < 8; i++) {
        terminal_setcolor(vga_entry_color(VGA_BLACK, (uint8_t)i));
        kprintf("   ");
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("\n                ");
    for (int i = 8; i < 16; i++) {
        terminal_setcolor(vga_entry_color(VGA_BLACK, (uint8_t)i));
        kprintf("   ");
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("\n\n");
}

static void cmd_info(void) {
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("\n  MyMisu OS v0.3.0\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("  Architecture:  x86 (i686)\n");
    kprintf("  Display:       VGA text mode 80x25\n");
    kprintf("  Keyboard:      PS/2 (US QWERTY)\n");
    kprintf("  Timer:         PIT at 100 Hz\n");
    kprintf("  Memory:        %d KB total, %d KB free\n", pmm_get_total_memory_kb(), pmm_get_free_pages() * 4);
    kprintf("  Processes:     %d active\n", process_get_count());
    kprintf("  Filesystem:    Ramdisk (%d files, %d dirs)\n", fs_get_file_count(), fs_get_dir_count());
    kprintf("  System calls:  write, read, mkdir, getpid, uptime\n");
    kprintf("  Built with:    GCC 13.2.0 (i686-elf)\n");
    kprintf("  Built by:      James Mardi, Danny + AI\n\n");
}

static void cmd_meminfo(void) {
    uint32_t total = pmm_get_total_pages();
    uint32_t used = pmm_get_used_pages();
    uint32_t free_p = pmm_get_free_pages();
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("\n  Memory Statistics:\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("  Total:  %d KB (%d MB)\n", total * 4, total * 4 / 1024);
    kprintf("  Pages:  %d (4 KB each)\n", total);
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
    kprintf("  Free:   %d (%d KB)\n", free_p, free_p * 4);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
    kprintf("  Used:   %d (%d KB)\n", used, used * 4);
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("  Bar:    [");
    uint32_t bw = 40;
    uint32_t filled = (total > 0) ? (used * bw / total) : 0;
    for (uint32_t i = 0; i < bw; i++) {
        terminal_setcolor(i < filled ? vga_entry_color(VGA_LIGHT_RED, VGA_BLACK) : vga_entry_color(VGA_GREEN, VGA_BLACK));
        kprintf(i < filled ? "#" : "-");
    }
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("] %d%%\n\n", total > 0 ? used * 100 / total : 0);
}

static void cmd_ls(void) {
    char buf[2048];
    int count = fs_list("/", buf, sizeof(buf));
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("\n  Directory listing (%d entries):\n", count);
    size_t i = 0;
    while (buf[i]) {
        if (buf[i] == 'D' && buf[i+1] == 'I' && buf[i+2] == 'R')
            terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
        else
            terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
        kprintf("  ");
        while (buf[i] && buf[i] != '\n') { terminal_putchar(buf[i]); i++; }
        kprintf("\n");
        if (buf[i] == '\n') i++;
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("\n");
}

static void cmd_cat(int argc, char** argv) {
    if (argc < 2) { kprintf("  Usage: cat <filename>\n"); return; }
    char buf[4096];
    int result = fs_read_file(argv[1], buf, sizeof(buf));
    if (result < 0) {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        kprintf("  File not found: %s\n", argv[1]);
    } else {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
        kprintf("\n%s\n", buf);
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_touch(int argc, char** argv) {
    if (argc < 2) { kprintf("  Usage: touch <filename>\n"); return; }
    if (fs_create_file(argv[1]) == 0) {
        terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
        kprintf("  Created file: %s\n", argv[1]);
    } else {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        kprintf("  Failed (exists or no space)\n");
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_mkdir_shell(int argc, char** argv) {
    if (argc < 2) { kprintf("  Usage: mkdir <dirname>\n"); return; }
    if (fs_mkdir(argv[1]) == 0) {
        terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
        kprintf("  Created directory: %s\n", argv[1]);
    } else {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        kprintf("  Failed (exists or no space)\n");
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_write_file(int argc, char** argv) {
    if (argc < 3) { kprintf("  Usage: write <filename> <text...>\n"); return; }
    char data[1024];
    data[0] = '\0';
    for (int i = 2; i < argc; i++) {
        strcat(data, argv[i]);
        if (i < argc - 1) strcat(data, " ");
    }
    strcat(data, "\n");
    int result = fs_write_file(argv[1], data, strlen(data));
    if (result < 0) {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        kprintf("  File not found: %s (use 'touch' first)\n", argv[1]);
    } else {
        terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
        kprintf("  Wrote %d bytes to %s\n", result, argv[1]);
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_rm(int argc, char** argv) {
    if (argc < 2) { kprintf("  Usage: rm <name>\n"); return; }
    if (fs_delete(argv[1]) == 0) {
        terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
        kprintf("  Deleted: %s\n", argv[1]);
    } else {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        kprintf("  Failed (not found or dir not empty)\n");
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_ps(void) {
    process_t* table = process_get_table();
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("\n  PID  STATE      TICKS    NAME\n");
    terminal_setcolor(vga_entry_color(VGA_DARK_GREY, VGA_BLACK));
    kprintf("  ---  --------   ------   ----\n");
    const char* state_names[] = {"UNUSED", "RUNNING", "READY", "BLOCKED", "DEAD"};
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (table[i].in_use) {
            if (table[i].state == PROC_RUNNING)
                terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
            else if (table[i].state == PROC_READY)
                terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK));
            else
                terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
            kprintf("  %d    %s    %d       %s\n", table[i].pid, state_names[table[i].state], table[i].ticks_used, table[i].name);
        }
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("\n  Total: %d processes\n\n", process_get_count());
}

static void cmd_spawn(int argc, char** argv) {
    if (argc < 2) { kprintf("  Usage: spawn <name>\n"); return; }
    int pid = process_create(argv[1], 5);
    if (pid >= 0) {
        terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
        kprintf("  Created process '%s' (PID %d)\n", argv[1], pid);
    } else {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        kprintf("  Failed (process table full)\n");
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_kill(int argc, char** argv) {
    if (argc < 2) { kprintf("  Usage: kill <pid>\n"); return; }
    uint32_t pid = (uint32_t)atoi(argv[1]);
    if (pid <= 1) {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        kprintf("  Cannot kill kernel or shell!\n");
    } else {
        process_terminate(pid);
        terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
        kprintf("  Killed process %d\n", pid);
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_syscall_demo(void) {
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("\n  System Call Demo:\n\n");
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK));
    kprintf("  Calling sys_write(STDOUT, \"Hello from syscall!\", 20)...\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
    kprintf("  > ");
    int32_t result = sys_write(STDOUT, "Hello from syscall!\n", 20);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("  Return value: %d (bytes written)\n", result);
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK));
    kprintf("\n  Calling sys_write(STDERR, \"Error message!\", 15)...\n");
    kprintf("  > ");
    result = sys_write(STDERR, "Error message!\n", 15);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("  Return value: %d (bytes written)\n", result);
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK));
    kprintf("\n  Calling sys_mkdir(\"syscall_test\")...\n");
    result = sys_mkdir("syscall_test");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("  Return value: %d (%s)\n", result, result == 0 ? "success" : "failed");
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK));
    kprintf("\n  Calling sys_getpid()...\n");
    result = sys_getpid();
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("  Return value: %d (current PID)\n", result);
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK));
    kprintf("\n  Calling sys_uptime()...\n");
    result = sys_uptime();
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("  Return value: %d ticks (%d seconds)\n\n", result, result / 100);
}

static void cmd_time(void) {
    uint32_t ticks = timer_get_ticks();
    uint32_t seconds = ticks / 100;
    uint32_t minutes = seconds / 60;
    seconds %= 60;
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
    if (argc < 3) { kprintf("  Usage: color <fg 0-15> <bg 0-15>\n"); return; }
    int fg = atoi(argv[1]);
    int bg = atoi(argv[2]);
    if (fg < 0 || fg > 15 || bg < 0 || bg > 15) { kprintf("  Invalid (0-15)\n"); return; }
    terminal_setcolor(vga_entry_color((uint8_t)fg, (uint8_t)bg));
    kprintf("  Color changed!\n");
}

static void cmd_alloc(void) {
    uint32_t addr = pmm_alloc_page();
    if (addr) {
        terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK));
        kprintf("  Allocated page at 0x%x\n", addr);
    } else {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK));
        kprintf("  Out of memory!\n");
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
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
    else if (strcmp(argv[0], "neofetch") == 0) cmd_neofetch();
    else if (strcmp(argv[0], "meminfo") == 0) cmd_meminfo();
    else if (strcmp(argv[0], "alloc") == 0) cmd_alloc();
    else if (strcmp(argv[0], "time") == 0) cmd_time();
    else if (strcmp(argv[0], "ls") == 0) cmd_ls();
    else if (strcmp(argv[0], "cat") == 0) cmd_cat(argc, argv);
    else if (strcmp(argv[0], "touch") == 0) cmd_touch(argc, argv);
    else if (strcmp(argv[0], "mkdir") == 0) cmd_mkdir_shell(argc, argv);
    else if (strcmp(argv[0], "write") == 0) cmd_write_file(argc, argv);
    else if (strcmp(argv[0], "rm") == 0) cmd_rm(argc, argv);
    else if (strcmp(argv[0], "ps") == 0) cmd_ps();
    else if (strcmp(argv[0], "spawn") == 0) cmd_spawn(argc, argv);
    else if (strcmp(argv[0], "kill") == 0) cmd_kill(argc, argv);
    else if (strcmp(argv[0], "syscall") == 0) cmd_syscall_demo();
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
            if (c == '\n') { terminal_putchar('\n'); cmd_buffer[cmd_len] = '\0'; break; }
            else if (c == '\b') { if (cmd_len > 0) { cmd_len--; terminal_putchar('\b'); } }
            else if (cmd_len < CMD_BUFFER_SIZE - 1) { cmd_buffer[cmd_len++] = c; terminal_putchar(c); }
        }
        if (cmd_len > 0) execute_command(cmd_buffer);
    }
}
