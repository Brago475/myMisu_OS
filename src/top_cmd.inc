/* ===== Phase 3: live top command ===== */

/* Helpers since kprintf has no %-Nd or %-Ns padding */
static void pad_str(const char* s, int width){
    int n = 0;
    while(s[n]) n++;
    terminal_writestring(s);
    for(int i=n;i<width;i++) terminal_putchar(' ');
}
static void pad_int(int v, int width){
    char buf[16]; int len = 0;
    if(v == 0){ buf[len++] = '0'; }
    else{
        int x = v < 0 ? -v : v;
        while(x > 0 && len < 15){ buf[len++] = '0' + (x % 10); x /= 10; }
        if(v < 0) buf[len++] = '-';
    }
    /* reverse */
    for(int i=0;i<len/2;i++){ char t=buf[i]; buf[i]=buf[len-1-i]; buf[len-1-i]=t; }
    buf[len] = 0;
    terminal_writestring(buf);
    for(int i=len;i<width;i++) terminal_putchar(' ');
}

static void cmd_top(void){
    process_t* tab = process_get_table();
    uint32_t prev_ticks[MAX_PROCESSES] = {0};
    for(int i=0;i<MAX_PROCESSES;i++) prev_ticks[i] = tab[i].ticks_used;

    while(1){
        if(keyboard_haschar()){
            char c = keyboard_getchar();
            if(c == KEY_ESC) break;
        }

        terminal_clear();

        terminal_setcolor(vga_entry_color(VGA_BLACK,VGA_LIGHT_CYAN));
        kprintf(" MyMisu OS - top                                      Press ESC to exit ");
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
        kprintf("\n");

        uint32_t up = sys_uptime();
        uint32_t up_s = up / 100;
        terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
        kprintf("  uptime: ");
        terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
        kprintf("%ds (%d ticks)", up_s, up);
        terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
        kprintf("   memory: ");
        terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
        kprintf("%d/%d KB free", pmm_get_free_pages()*4, pmm_get_total_memory_kb());
        terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
        kprintf("   procs: ");
        terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
        kprintf("%d\n\n", process_get_count());

        terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));
        kprintf("  PID   NAME              STATE       CPU%%    TICKS\n");
        terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
        kprintf("  ----- ----------------- ----------- ------- --------\n");
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));

        for(int i=0;i<MAX_PROCESSES;i++){
            if(!tab[i].in_use) continue;
            uint32_t delta = tab[i].ticks_used - prev_ticks[i];
            uint32_t cpu_pct = delta * 2;
            if(cpu_pct > 100) cpu_pct = 100;

            kprintf("  ");
            terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
            pad_int((int)tab[i].pid, 6);
            terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
            pad_str(tab[i].name, 18);

            const char* st = "?";
            uint8_t stcolor = VGA_LIGHT_GREY;
            switch(tab[i].state){
                case PROC_RUNNING:    st="RUNNING";    stcolor=VGA_LIGHT_GREEN; break;
                case PROC_READY:      st="READY";      stcolor=VGA_LIGHT_CYAN;  break;
                case PROC_BLOCKED:    st="BLOCKED";    stcolor=VGA_YELLOW;      break;
                case PROC_TERMINATED: st="TERMINATED"; stcolor=VGA_LIGHT_RED;   break;
                default:              st="UNUSED";     stcolor=VGA_DARK_GREY;   break;
            }
            terminal_setcolor(vga_entry_color(stcolor,VGA_BLACK));
            pad_str(st, 12);

            if(cpu_pct >= 50)      terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));
            else if(cpu_pct >= 20) terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));
            else                   terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));
            pad_int((int)cpu_pct, 4);
            kprintf("%%   ");

            terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
            kprintf("%d\n", tab[i].ticks_used);
        }

        for(int i=0;i<MAX_PROCESSES;i++) prev_ticks[i] = tab[i].ticks_used;

        terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
        kprintf("\n  Refreshing every 500ms. Spawn tasks first to see scheduling activity.\n");
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));

        process_sleep(50);
    }

    terminal_clear();
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
    kprintf("\n  exited top.\n");
}
