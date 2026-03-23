#!/bin/bash
cd ~/mymisu_os

python3 << 'PYEOF'
with open("src/shell.c", "r") as f:
    code = f.read()

apps = r'''
static void draw_box(int x, int y, int w, int h, const char* title) {
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    /* Top border */
    for(int i=0;i<VGA_WIDTH;i++){
        size_t idx = y * VGA_WIDTH + i;
        if(i>=x&&i<x+w){
            uint16_t* buf=(uint16_t*)0xB8000;
            if(i==x) buf[idx]=vga_entry('+',vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
            else if(i==x+w-1) buf[idx]=vga_entry('+',vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
            else buf[idx]=vga_entry('-',vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
        }
    }
    /* Bottom */
    for(int i=x;i<x+w;i++){
        uint16_t* buf=(uint16_t*)0xB8000;
        size_t idx=(y+h-1)*VGA_WIDTH+i;
        if(i==x||i==x+w-1) buf[idx]=vga_entry('+',vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
        else buf[idx]=vga_entry('-',vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
    }
    /* Sides */
    for(int r=y+1;r<y+h-1;r++){
        uint16_t* buf=(uint16_t*)0xB8000;
        buf[r*VGA_WIDTH+x]=vga_entry('|',vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
        buf[r*VGA_WIDTH+x+w-1]=vga_entry('|',vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
    }
    /* Title */
    if(title){
        int tlen=0;while(title[tlen])tlen++;
        int tx=x+(w-tlen)/2;
        uint16_t* buf=(uint16_t*)0xB8000;
        for(int i=0;title[i];i++)
            buf[y*VGA_WIDTH+tx+i]=vga_entry(title[i],vga_entry_color(VGA_WHITE,VGA_BLACK));
    }
}

static void vga_put(int x, int y, char c, uint8_t color) {
    uint16_t* buf=(uint16_t*)0xB8000;
    if(x>=0&&x<80&&y>=0&&y<25)
        buf[y*80+x]=vga_entry(c,color);
}

static void vga_puts(int x, int y, const char* s, uint8_t color) {
    for(int i=0;s[i];i++) vga_put(x+i,y,s[i],color);
}

/* ===== MAIN MENU UI ===== */
static void cmd_menu(void) {
    const char* items[]={"Neofetch","File Browser","Book Reader","System Monitor","Calculator","Games","Shell (back)","Shutdown"};
    int nitems=8;
    int sel=0;
    while(1){
        terminal_clear();
        draw_box(15,2,50,22,"[ MyMisu OS - Main Menu ]");
        vga_puts(20,4,"Navigate with UP/DOWN, ENTER to select",vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
        for(int i=0;i<nitems;i++){
            if(i==sel){
                vga_puts(25,7+i*2," > ",vga_entry_color(VGA_BLACK,VGA_LIGHT_CYAN));
                vga_puts(28,7+i*2,items[i],vga_entry_color(VGA_BLACK,VGA_LIGHT_CYAN));
                /* Pad to clear */
                int len=0;while(items[i][len])len++;
                for(int p=len;p<20;p++) vga_put(28+p,7+i*2,' ',vga_entry_color(VGA_BLACK,VGA_LIGHT_CYAN));
            } else {
                vga_puts(25,7+i*2,"   ",vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
                vga_puts(28,7+i*2,items[i],vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
            }
        }
        char c=keyboard_getchar();
        if(c==KEY_UP&&sel>0)sel--;
        else if(c==KEY_DOWN&&sel<nitems-1)sel++;
        else if(c=='\n'){
            terminal_clear();
            if(sel==0)cmd_neofetch();
            else if(sel==1)cmd_filebrowser();
            else if(sel==2)cmd_reader();
            else if(sel==3)cmd_sysmon();
            else if(sel==4){kprintf("  Use: calc <n> <op> <n>\n\n");}
            else if(sel==5){
                const char* games[]={"snake","ttt","ttt2","hangman","rps","pong","game"};
                terminal_clear();
                draw_box(20,3,40,18,"[ Games ]");
                int gs=0;int ng=7;
                while(1){
                    for(int i=0;i<ng;i++){
                        if(i==gs) vga_puts(28,6+i*2,games[i],vga_entry_color(VGA_BLACK,VGA_LIGHT_GREEN));
                        else vga_puts(28,6+i*2,games[i],vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
                    }
                    char gc=keyboard_getchar();
                    if(gc==KEY_UP&&gs>0)gs--;
                    else if(gc==KEY_DOWN&&gs<ng-1)gs++;
                    else if(gc==KEY_ESC)break;
                    else if(gc=='\n'){
                        terminal_clear();
                        if(gs==0)cmd_snake();
                        else if(gs==1)cmd_tictactoe();
                        else if(gs==2)cmd_ttt2();
                        else if(gs==3)cmd_hangman();
                        else if(gs==4)cmd_rps();
                        else if(gs==5)cmd_pong();
                        else if(gs==6)cmd_game();
                        break;
                    }
                }
            }
            else if(sel==6)return;
            else if(sel==7){cmd_halt();return;}
            if(sel!=5) continue; /* loop back to menu unless games broke out */
        }
        else if(c==KEY_ESC)return;
    }
}

/* ===== FILE BROWSER ===== */
static void cmd_filebrowser(void) {
    while(1){
        char listing[2048];
        int count=fs_list("/",listing,sizeof(listing));
        /* Parse into entries */
        char names[32][40];
        char types[32];
        int n=0;
        size_t i=0;
        while(listing[i]&&n<32){
            types[n]=(listing[i]=='D')?'D':'F';
            i+=6; /* skip "DIR  " or "FILE " */
            int ni=0;
            while(listing[i]&&listing[i]!='\n'&&ni<39){names[n][ni++]=listing[i++];}
            names[n][ni]='\0';
            if(listing[i]=='\n')i++;
            n++;
        }

        int sel=0;
        while(1){
            terminal_clear();
            draw_box(5,1,70,23,"[ File Browser ]");
            vga_puts(8,3,"Directory: ",vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
            vga_puts(19,3,fs_pwd(),vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
            vga_puts(8,4,"ENTER=open  BACKSPACE=up  N=new file  D=delete  ESC=exit",vga_entry_color(VGA_DARK_GREY,VGA_BLACK));

            for(int j=0;j<n&&j<16;j++){
                uint8_t color;
                if(j==sel) color=vga_entry_color(VGA_BLACK,VGA_LIGHT_CYAN);
                else if(types[j]=='D') color=vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK);
                else color=vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK);

                vga_puts(10,6+j,types[j]=='D'?"DIR  ":"FILE ",color);
                vga_puts(15,6+j,names[j],color);
                /* Pad */
                int len=0;while(names[j][len])len++;
                for(int p=len;p<50;p++)vga_put(15+p,6+j,' ',color);
            }
            if(n==0) vga_puts(10,6,"(empty directory)",vga_entry_color(VGA_DARK_GREY,VGA_BLACK));

            char c=keyboard_getchar();
            if(c==KEY_UP&&sel>0)sel--;
            else if(c==KEY_DOWN&&sel<n-1)sel++;
            else if(c=='\n'&&n>0){
                if(types[sel]=='D'){
                    fs_cd(names[sel]);
                    break; /* refresh listing */
                } else {
                    /* Show file contents */
                    terminal_clear();
                    draw_box(5,1,70,23,names[sel]);
                    char fbuf[4096];
                    int r=fs_read_file(names[sel],fbuf,sizeof(fbuf));
                    if(r>0){
                        int row=3,col=7;
                        for(int fi=0;fbuf[fi]&&row<23;fi++){
                            if(fbuf[fi]=='\n'){row++;col=7;}
                            else{vga_put(col,row,fbuf[fi],vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));col++;if(col>72){col=7;row++;}}
                        }
                    } else vga_puts(7,3,"(empty file)",vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
                    vga_puts(7,24,"Press any key to go back",vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
                    keyboard_getchar();
                }
            }
            else if(c=='\b'){fs_cd("..");break;}
            else if(c=='n'||c=='N'){
                terminal_clear();
                terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
                kprintf("\n  New filename: ");
                char fname[32];int fi=0;
                while(1){char fc=keyboard_getchar();if(fc=='\n'){fname[fi]='\0';break;}else if(fc=='\b'){if(fi>0){fi--;terminal_putchar('\b');}}else if(fi<31&&fc>=32&&fc<127){fname[fi++]=fc;terminal_putchar(fc);}}
                if(fi>0) fs_create_file(fname);
                break;
            }
            else if(c=='d'||c=='D'){
                if(n>0){fs_delete(names[sel]);break;}
            }
            else if(c==KEY_ESC) return;
        }
    }
}

/* ===== BOOK READER ===== */
static void cmd_reader(void) {
    terminal_clear();
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
    kprintf("\n  Enter filename to read: ");
    char fname[32];int fi=0;
    while(1){char c=keyboard_getchar();if(c=='\n'){fname[fi]='\0';break;}else if(c=='\b'){if(fi>0){fi--;terminal_putchar('\b');}}else if(fi<31&&c>=32&&c<127){fname[fi++]=c;terminal_putchar(c);}}

    char content[4096];
    int len=fs_read_file(fname,content,sizeof(content));
    if(len<=0){
        terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));
        kprintf("\n  File not found: %s\n",fname);
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
        return;
    }

    /* Paginate content */
    int lines_per_page=18;
    int offset=0;
    int page=1;

    while(offset<len){
        terminal_clear();
        draw_box(2,0,76,24,fname);

        /* Render text */
        int row=2,col=4;
        int start=offset;
        int line_count=0;
        int i=offset;
        while(i<len&&line_count<lines_per_page){
            if(content[i]=='\n'){row++;col=4;line_count++;i++;continue;}
            vga_put(col,row,content[i],vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
            col++;
            if(col>74){col=4;row++;line_count++;}
            i++;
        }
        int next_offset=i;

        /* Status bar */
        vga_puts(4,23,"LEFT=prev  RIGHT=next  ESC=exit",vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
        /* Page indicator */
        terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
        /* crude page display */
        vga_puts(65,23,"Page ",vga_entry_color(VGA_WHITE,VGA_BLACK));

        char c=keyboard_getchar();
        if(c==KEY_RIGHT||c=='\n'){
            if(next_offset<len){offset=next_offset;page++;}
        }
        else if(c==KEY_LEFT){
            /* Go back - crude, restart and count forward */
            if(page>1){
                page--;
                offset=0;
                for(int p=0;p<page-1;p++){
                    int lc=0;int j=offset;
                    while(j<len&&lc<lines_per_page){
                        if(content[j]=='\n')lc++;
                        else{int tc=4;while(j<len&&content[j]!='\n'){tc++;j++;if(tc>74){lc++;tc=4;}}continue;}
                        j++;
                    }
                    offset=j;
                }
            }
        }
        else if(c==KEY_ESC) break;
    }
    terminal_clear();
}

/* ===== SYSTEM MONITOR ===== */
static void cmd_sysmon(void) {
    while(1){
        terminal_clear();
        draw_box(0,0,80,25,"[ System Monitor - ESC to exit ]");

        /* OS Info */
        vga_puts(3,2,"MyMisu OS v0.5.0",vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));

        /* Uptime */
        uint32_t ticks=timer_get_ticks();
        uint32_t sec=ticks/100;
        uint32_t min=sec/60;sec%=60;
        vga_puts(3,4,"Uptime:",vga_entry_color(VGA_WHITE,VGA_BLACK));
        /* Manual number print to VGA */
        char ubuf[32];
        int ui=0;
        /* minutes */
        if(min>0){int m=min;if(m>=10)ubuf[ui++]='0'+m/10;ubuf[ui++]='0'+m%10;ubuf[ui++]='m';ubuf[ui++]=' ';}
        int s=sec;if(s>=10)ubuf[ui++]='0'+s/10;ubuf[ui++]='0'+s%10;ubuf[ui++]='s';ubuf[ui]='\0';
        vga_puts(12,4,ubuf,vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));

        /* Memory */
        vga_puts(3,6,"Memory:",vga_entry_color(VGA_WHITE,VGA_BLACK));
        uint32_t total=pmm_get_total_pages(),used=pmm_get_used_pages(),free_p=pmm_get_free_pages();
        /* Bar */
        vga_puts(3,7,"[",vga_entry_color(VGA_WHITE,VGA_BLACK));
        uint32_t bw=50;
        uint32_t filled=(total>0)?(used*bw/total):0;
        for(uint32_t i=0;i<bw;i++){
            vga_put(4+i,7,i<filled?'#':'-',i<filled?vga_entry_color(VGA_LIGHT_RED,VGA_BLACK):vga_entry_color(VGA_GREEN,VGA_BLACK));
        }
        vga_puts(54,7,"]",vga_entry_color(VGA_WHITE,VGA_BLACK));

        /* Memory numbers */
        vga_puts(3,8,"Used: ",vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));
        vga_puts(3,9,"Free: ",vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));
        vga_puts(3,10,"Total:",vga_entry_color(VGA_WHITE,VGA_BLACK));

        /* Processes */
        vga_puts(3,12,"Processes:",vga_entry_color(VGA_WHITE,VGA_BLACK));
        process_t* pt=process_get_table();
        int row=14;
        vga_puts(3,13,"PID  STATE      NAME",vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
        const char* sn[]={"----","RUN ","WAIT","BLCK","DEAD"};
        for(int i=0;i<MAX_PROCESSES&&row<22;i++){
            if(pt[i].in_use){
                uint8_t col=pt[i].state==PROC_RUNNING?vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK):vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK);
                char pid_c='0'+pt[i].pid%10;
                vga_put(3,row,pid_c,col);
                vga_puts(8,row,sn[pt[i].state],col);
                vga_puts(15,row,pt[i].name,col);
                row++;
            }
        }

        /* Filesystem */
        vga_puts(45,12,"Filesystem:",vga_entry_color(VGA_WHITE,VGA_BLACK));
        vga_puts(45,14,"Files:",vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));
        vga_puts(45,15,"Dirs: ",vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));

        /* RTC */
        outb(0x70,0x04);uint8_t hr=inb(0x71);outb(0x70,0x02);uint8_t mn=inb(0x71);outb(0x70,0x00);uint8_t sc=inb(0x71);
        hr=(hr&0x0F)+((hr>>4)*10);mn=(mn&0x0F)+((mn>>4)*10);sc=(sc&0x0F)+((sc>>4)*10);
        char tbuf[12];
        tbuf[0]='0'+hr/10;tbuf[1]='0'+hr%10;tbuf[2]=':';
        tbuf[3]='0'+mn/10;tbuf[4]='0'+mn%10;tbuf[5]=':';
        tbuf[6]='0'+sc/10;tbuf[7]='0'+sc%10;tbuf[8]='\0';
        vga_puts(60,2,tbuf,vga_entry_color(VGA_YELLOW,VGA_BLACK));

        /* Wait ~500ms or keypress */
        uint32_t start=timer_get_ticks();
        while(timer_get_ticks()-start<50){
            if(keyboard_haschar()){
                char c=keyboard_getchar();
                if(c==KEY_ESC){terminal_clear();return;}
            }
            asm volatile("hlt");
        }
    }
}
'''

# Insert before execute_command
code = code.replace('static void execute_command', apps + '\nstatic void execute_command')

# Add dispatchers
code = code.replace(
    'else if(strcmp(argv[0],"pong")==0)cmd_pong();',
    'else if(strcmp(argv[0],"pong")==0)cmd_pong();\n    else if(strcmp(argv[0],"menu")==0)cmd_menu();\n    else if(strcmp(argv[0],"files")==0)cmd_filebrowser();\n    else if(strcmp(argv[0],"reader")==0)cmd_reader();\n    else if(strcmp(argv[0],"sysmon")==0)cmd_sysmon();'
)

# Add help entries
code = code.replace(
    'kprintf("  pong      ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Pong 2P (W/S vs I/K)\\n");',
    'kprintf("  pong      ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Pong 2P (W/S vs I/K)\\n");\n    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  menu      ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Visual menu (arrow keys)\\n");\n    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  files     ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("File browser\\n");\n    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  reader    ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Book reader\\n");\n    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  sysmon    ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("System monitor (live)\\n");'
)

with open("src/shell.c", "w") as f:
    f.write(code)
print("UI features patched!")
PYEOF

echo ""
echo "========================================="
echo "  UI Features Added!"
echo "========================================="
echo ""
echo "New commands:"
echo "  menu    - Visual main menu (arrow keys + enter)"
echo "  files   - File browser (arrows, enter, backspace)"
echo "  reader  - Kindle-style book reader (left/right pages)"
echo "  sysmon  - Live system monitor (auto-refreshing)"
echo ""
echo "Run: make clean && make && make run"
