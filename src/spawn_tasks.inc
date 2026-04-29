/* ===== Phase 2 demo kernel tasks =====
 * Tasks print to the shell area but throttled so they don't
 * overwhelm input. Each prints once per ~2 seconds.
 */

static void demo_counter_task(void){
    int n=0;
    process_sleep(3000);
    while(1){
        terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
        kprintf("\n[counter pid=%d n=%d]\n",sys_getpid(),n++);
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
        process_sleep(200);
    }
}

static void demo_spinner_task(void){
    int i=0;
    process_sleep(3000);
    while(1){
        terminal_setcolor(vga_entry_color(VGA_LIGHT_MAGENTA,VGA_BLACK));
        kprintf("\n[spinner pid=%d frame=%d]\n",sys_getpid(),i++);
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
        process_sleep(250);
    }
}

static void demo_ticker_task(void){
    while(1){
        terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));
        kprintf("\n[ticker pid=%d uptime=%d]\n",sys_getpid(),sys_uptime());
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
        process_sleep(300);
    }
}

static void cmd_spawn(int argc,char** argv){
    if(argc<2){
        kprintf("  Usage: spawn <name>\n");
        kprintf("  Available: counter, spinner, ticker, all\n");
        return;
    }
    int pid=-1;
    if(strcmp(argv[1],"counter")==0)      pid=process_create_kernel("counter",demo_counter_task);
    else if(strcmp(argv[1],"spinner")==0) pid=process_create_kernel("spinner",demo_spinner_task);
    else if(strcmp(argv[1],"ticker")==0)  pid=process_create_kernel("ticker",demo_ticker_task);
    else if(strcmp(argv[1],"all")==0){
        process_create_kernel("counter",demo_counter_task);
        process_create_kernel("spinner",demo_spinner_task);
        pid=process_create_kernel("ticker",demo_ticker_task);
    }
    else{
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));
        kprintf("  Unknown task: %s\n",argv[1]);
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
        return;
    }
    if(pid>=0){
        terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));
        kprintf("  Spawned PID %d. Use 'kill <pid>' to stop. Run 'ps' to see state.\n",pid);
    }else{
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));
        kprintf("  Failed to spawn\n");
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
}
