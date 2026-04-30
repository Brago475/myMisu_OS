static void cmd_help(void){
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
    kprintf("\n  MyMisu OS  -  Command Reference\n");
    terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
    kprintf("  ----------------------------------------------------------\n");

    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("\n  SYSTEM\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
    kprintf("    help      Show this command reference\n");
    kprintf("    clear     Clear the screen\n");
    kprintf("    info      System info (uptime, memory, processes)\n");
    kprintf("    neofetch  ASCII art system summary\n");
    kprintf("    version   Print OS version\n");
    kprintf("    date      Show current tick / uptime as date\n");
    kprintf("    time      Show uptime in HH:MM:SS\n");
    kprintf("    history   Show command history\n");
    kprintf("    color     Change colors:  color <fg> <bg>  (0-15)\n");
    kprintf("    colors    Show all 16 VGA colors\n");
    kprintf("    echo      Echo text back\n");

    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("\n  FILES\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
    kprintf("    ls        List files in current directory\n");
    kprintf("    pwd       Print current directory path\n");
    kprintf("    cd        Change directory:  cd <name>  or  cd ..\n");
    kprintf("    cat       Show file contents:  cat <file>\n");
    kprintf("    touch     Create empty file:  touch <name>\n");
    kprintf("    write     Write text to file:  write <file> <text>\n");
    kprintf("    edit      Open text editor:  edit <file>\n");
    kprintf("    mkdir     Create directory:  mkdir <name>\n");
    kprintf("    rm        Delete file or empty dir:  rm <name>\n");

    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("\n  PROCESSES\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
    kprintf("    ps        Quick process list\n");
    kprintf("    top       Live process viewer (refreshes every 500ms)\n");
    kprintf("    spawn     Start a kernel task:  spawn <name>\n");
    kprintf("              names: counter, spinner, ticker, all\n");
    kprintf("    kill      Stop a process:  kill <pid>\n");
    kprintf("    meminfo   Show memory usage details\n");
    kprintf("    syscall   Run all 19 syscalls and report PASS/FAIL\n");

    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("\n  APPS\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
    kprintf("    menu      Visual menu (arrow keys)\n");
    kprintf("    files     File browser\n");
    kprintf("    reader    Read text files page by page\n");
    kprintf("    sysmon    System monitor dashboard\n");
    kprintf("    calc      Calculator:  calc <a> <op> <b>\n");
    kprintf("    note      Notepad:  note add <text>  /  note list\n");
    kprintf("    cowsay    ASCII cow says something:  cowsay <text>\n");

    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("\n  GAMES\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
    kprintf("    snake     Classic snake (arrow keys, Q to quit)\n");
    kprintf("    ttt       Tic-tac-toe vs unbeatable CPU (numpad 1-9)\n");
    kprintf("    ttt2      Tic-tac-toe two-player\n");
    kprintf("    hangman   Word guessing game\n");
    kprintf("    rps       Rock paper scissors vs CPU (best of 5)\n");
    kprintf("    pong      Two-player pong (W/S vs I/K)\n");

    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("\n  POWER\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
    kprintf("    reboot    Restart the system\n");
    kprintf("    shutdown  Halt the CPU\n");
    kprintf("    exit      Same as shutdown\n");
    kprintf("    panic     Trigger a kernel panic (demo only)\n");
    kprintf("\n");
    terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
    kprintf("  Tip: try  spawn all  then  top  to see preemptive multitasking.\n\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
}

static void cmd_colors(void){
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
    kprintf("\n  VGA Color Palette\n");
    terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
    kprintf("  -----------------\n");
    const char* names[16] = {
        "0  black",          "1  blue",           "2  green",         "3  cyan",
        "4  red",            "5  magenta",        "6  brown",         "7  light grey",
        "8  dark grey",      "9  light blue",     "10 light green",   "11 light cyan",
        "12 light red",      "13 light magenta",  "14 yellow",        "15 white"
    };
    for(int i=0;i<16;i++){
        terminal_setcolor(vga_entry_color((uint8_t)i,VGA_BLACK));
        kprintf("    %s", names[i]);
        terminal_setcolor(vga_entry_color(VGA_BLACK,(uint8_t)i));
        kprintf("        ");
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
        kprintf("\n");
    }
    kprintf("\n");
    terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
    kprintf("  Use:  color <fg> <bg>     example:  color 14 0   (yellow on black)\n");
    kprintf("  Reset: color 7 0\n\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
}
