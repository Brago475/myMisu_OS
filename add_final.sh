#!/bin/bash
cd ~/mymisu_os

python3 << 'PYEOF'
with open("src/kernel.c", "r") as f:
    code = f.read()

# Replace the login_screen() call with boot_animation() then login
boot_anim = '''
    /* Boot Animation */
    terminal_clear();
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));

    /* Animate the cat drawing in */
    const char* cat_lines[] = {
        "                  |\\\\---/|",
        "                  | ,_, |",
        "                   \\\\_`_/-..--.  ",
        "                ___/ `   ' ,+ \\\\",
        "               (__...'  _/  |`._;  ",
        "                 (_,..'(_,.`__)  ",
    };

    for (int i = 0; i < 6; i++) {
        /* Position in center */
        terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
        kprintf("%s\\n", cat_lines[i]);
        /* Small delay between lines */
        uint32_t d = timer_get_ticks();
        while(timer_get_ticks() - d < 15) asm volatile("hlt");
    }

    kprintf("\\n");
    /* Animate title letter by letter */
    const char* title = "  M y M i s u   O S";
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    for (int i = 0; title[i]; i++) {
        terminal_putchar(title[i]);
        uint32_t d = timer_get_ticks();
        while(timer_get_ticks() - d < 5) asm volatile("hlt");
    }
    kprintf("\\n\\n");

    /* Loading bar */
    terminal_setcolor(vga_entry_color(VGA_DARK_GREY, VGA_BLACK));
    kprintf("  Loading ");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("[");

    const char* steps[] = {"GDT", "IDT", "PIC", "PIT", "KBD", "PMM", "FS ", "SYS"};
    for (int i = 0; i < 8; i++) {
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
        kprintf("===");
        uint32_t d = timer_get_ticks();
        while(timer_get_ticks() - d < 20) asm volatile("hlt");
    }
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("] ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
    kprintf("OK\\n\\n");

    /* Brief pause */
    {uint32_t d = timer_get_ticks();
    while(timer_get_ticks() - d < 80) asm volatile("hlt");}

    /* Login screen */
    login_screen();
'''

# Replace just the login_screen() call
code = code.replace(
    '    /* Login screen */\n    login_screen();',
    boot_anim
)

with open("src/kernel.c", "w") as f:
    f.write(code)
print("Boot animation added to kernel.c")
PYEOF

# Add widgets command to shell.c
python3 << 'PYEOF'
with open("src/shell.c", "r") as f:
    code = f.read()

widget_func = r'''
static void cmd_widgets(void) {
    while(1) {
        terminal_clear();
        draw_box(0, 0, 80, 25, "[ MisuOS Desktop ]");

        /* Clock - big digital style */
        outb(0x70,0x04);uint8_t hr=inb(0x71);outb(0x70,0x02);uint8_t mi=inb(0x71);outb(0x70,0x00);uint8_t sc=inb(0x71);
        hr=(hr&0xF)+((hr>>4)*10);mi=(mi&0xF)+((mi>>4)*10);sc=(sc&0xF)+((sc>>4)*10);
        char tbuf[12];
        tbuf[0]='0'+hr/10;tbuf[1]='0'+hr%10;tbuf[2]=':';
        tbuf[3]='0'+mi/10;tbuf[4]='0'+mi%10;tbuf[5]=':';
        tbuf[6]='0'+sc/10;tbuf[7]='0'+sc%10;tbuf[8]='\0';

        /* Big clock in center */
        draw_box(25, 2, 30, 5, "[ Clock ]");
        vga_puts(33, 4, tbuf, vga_entry_color(VGA_YELLOW, VGA_BLACK));

        /* Date */
        outb(0x70,0x07);uint8_t day=inb(0x71);outb(0x70,0x08);uint8_t mon=inb(0x71);outb(0x70,0x09);uint8_t yr=inb(0x71);
        day=(day&0xF)+((day>>4)*10);mon=(mon&0xF)+((mon>>4)*10);yr=(yr&0xF)+((yr>>4)*10);
        char dbuf[16];
        dbuf[0]='2';dbuf[1]='0';dbuf[2]='0'+yr/10;dbuf[3]='0'+yr%10;dbuf[4]='-';
        dbuf[5]='0'+mon/10;dbuf[6]='0'+mon%10;dbuf[7]='-';
        dbuf[8]='0'+day/10;dbuf[9]='0'+day%10;dbuf[10]='\0';
        vga_puts(32, 5, dbuf, vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));

        /* System stats box */
        draw_box(3, 8, 35, 10, "[ System ]");
        uint32_t tk=timer_get_ticks(),sec=tk/100,mn=sec/60;sec%=60;
        char ub[20];int ui=0;
        if(mn>=10)ub[ui++]='0'+mn/10;ub[ui++]='0'+mn%10;ub[ui++]='m';ub[ui++]=' ';
        if(sec>=10)ub[ui++]='0'+sec/10;ub[ui++]='0'+sec%10;ub[ui++]='s';ub[ui]='\0';
        vga_puts(5, 10, "Uptime:", vga_entry_color(VGA_WHITE, VGA_BLACK));
        vga_puts(14, 10, ub, vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));

        uint32_t total=pmm_get_total_pages(),used=pmm_get_used_pages();
        vga_puts(5, 12, "Memory:", vga_entry_color(VGA_WHITE, VGA_BLACK));
        vga_puts(5, 13, "[", vga_entry_color(VGA_WHITE, VGA_BLACK));
        uint32_t bw=25,fl=(total>0)?(used*bw/total):0;
        for(uint32_t i=0;i<bw;i++) vga_put(6+i,13,i<fl?'#':'-',i<fl?vga_entry_color(VGA_LIGHT_RED,VGA_BLACK):vga_entry_color(VGA_GREEN,VGA_BLACK));
        vga_puts(31, 13, "]", vga_entry_color(VGA_WHITE, VGA_BLACK));

        vga_puts(5, 15, "Procs:", vga_entry_color(VGA_WHITE, VGA_BLACK));
        char pb[4];pb[0]='0'+process_get_count()%10;pb[1]='\0';
        vga_puts(13, 15, pb, vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));

        vga_puts(5, 16, "Files:", vga_entry_color(VGA_WHITE, VGA_BLACK));

        /* Quotes box */
        draw_box(42, 8, 35, 10, "[ Quote ]");
        const char* quotes[] = {
            "The only way to do",
            "great work is to love",
            "what you do. - Jobs",
            "",
            "Talk is cheap. Show",
            "me the code. - Torvalds",
            "",
            "First, solve the",
            "problem. Then, write",
            "the code. - Johnson",
        };
        int qi = (timer_get_ticks() / 500) % 3;
        for(int i = 0; i < 3; i++)
            vga_puts(44, 10 + i, quotes[qi*3+i+((qi*3+i<10)?0:0)], vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));

        /* Cat in corner */
        vga_puts(60, 19, "/\\_/\\", vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
        vga_puts(59, 20, "( o.o )", vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
        vga_puts(60, 21, "> ^ <", vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));

        /* Status bar */
        vga_puts(3, 23, "ESC=exit | Updates every second", vga_entry_color(VGA_DARK_GREY, VGA_BLACK));

        /* Wait ~1 sec or keypress */
        uint32_t start=timer_get_ticks();
        while(timer_get_ticks()-start<100){
            if(keyboard_haschar()){
                char c=keyboard_getchar();
                if(c==KEY_ESC){terminal_clear();return;}
            }
            asm volatile("hlt");
        }
    }
}
'''

# Insert before cmd_menu
code = code.replace('static void cmd_menu(void){', widget_func + '\nstatic void cmd_menu(void){')

# Add dispatcher
code = code.replace(
    'else if(strcmp(argv[0],"sysmon")==0)cmd_sysmon();',
    'else if(strcmp(argv[0],"sysmon")==0)cmd_sysmon();\n    else if(strcmp(argv[0],"widgets")==0)cmd_widgets();'
)

# Add to help
code = code.replace(
    'kprintf("  menu files reader sysmon\\n");',
    'kprintf("  menu files reader sysmon widgets\\n");'
)

# Add widgets to menu
code = code.replace(
    '    const char* items[]={"Neofetch","File Browser","Book Reader","System Monitor","Calculator","Games","Shell (back)","Shutdown"};int ni=8,sel=0;',
    '    const char* items[]={"Neofetch","File Browser","Book Reader","System Monitor","Desktop Widgets","Calculator","Games","Shell (back)","Shutdown"};int ni=9,sel=0;'
)

# Fix menu dispatch indices
code = code.replace(
    'if(sel==0)cmd_neofetch();else if(sel==1)cmd_filebrowser();else if(sel==2)cmd_reader();\n            else if(sel==3)cmd_sysmon();else if(sel==4){kprintf("  Use: calc <n> <op> <n>\\n");}',
    'if(sel==0)cmd_neofetch();else if(sel==1)cmd_filebrowser();else if(sel==2)cmd_reader();\n            else if(sel==3)cmd_sysmon();else if(sel==4)cmd_widgets();else if(sel==5){kprintf("  Use: calc <n> <op> <n>\\n");}'
)
code = code.replace(
    'else if(sel==5){const char* gm[]',
    'else if(sel==6){const char* gm[]'
)
code = code.replace(
    'else if(sel==6)return;else if(sel==7){cmd_halt();return;}',
    'else if(sel==7)return;else if(sel==8){cmd_halt();return;}'
)

with open("src/shell.c", "w") as f:
    f.write(code)
print("Widgets + menu update added to shell.c")
PYEOF

echo ""
echo "========================================="
echo "  Final Features Added!"
echo "========================================="
echo ""
echo "1. Boot animation (animated cat + loading bar)"
echo "2. Desktop widgets (widgets command - clock, stats, quotes)"
echo "3. USB boot - see instructions below"
echo ""
echo "=== USB BOOT INSTRUCTIONS ==="
echo "1. Copy ISO to Windows desktop:"
echo "   cp ~/mymisu_os/build/mymisu.iso /mnt/c/Users/james/Desktop/"
echo ""
echo "2. Download Rufus: https://rufus.ie"
echo "3. Plug in a USB drive"
echo "4. Open Rufus -> Select mymisu.iso -> Flash"
echo "5. Reboot PC -> Enter BIOS (F2/F12/DEL)"
echo "6. Boot from USB -> MisuOS boots on real hardware!"
echo ""
echo "Run: make clean && make && make run"
