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
#include "ports.h"

#define CMD_BUFFER_SIZE 256
#define MAX_ARGS 16
#define MAX_HISTORY 10
#define SNAKE_W 40
#define SNAKE_H 18
#define SNAKE_MAX 200

static char cmd_buffer[CMD_BUFFER_SIZE];
static int cmd_len = 0;
static char history[MAX_HISTORY][CMD_BUFFER_SIZE];
static int history_count = 0;
static int history_pos = -1;

/* Forward declarations */
static void cmd_help(void);
static void cmd_neofetch(void);
static void cmd_info(void);
static void cmd_meminfo(void);
static void cmd_ls(void);
static void cmd_cat(int,char**);
static void cmd_touch(int,char**);
static void cmd_mkdir_shell(int,char**);
static void cmd_write_file(int,char**);
static void cmd_rm(int,char**);
static void cmd_cd(int,char**);
static void cmd_edit(int,char**);
static void cmd_ps(void);
static void cmd_spawn(int,char**);
static void cmd_kill(int,char**);
static void cmd_syscall_demo(void);
static void cmd_calc(int,char**);
static void cmd_note(int,char**);
static void cmd_cowsay(int,char**);
static void cmd_game(void);
static void cmd_snake(void);
static void cmd_tictactoe(void);
static void cmd_ttt2(void);
static void cmd_hangman(void);
static void cmd_rps(void);
static void cmd_pong(void);
static void cmd_menu(void);
static void cmd_filebrowser(void);
static void cmd_reader(void);
static void cmd_sysmon(void);
static void cmd_history_show(void);
static void cmd_halt(void);
static void cmd_reboot(void);

/* ===== HELPERS ===== */
static void history_add(const char* cmd){if(history_count<MAX_HISTORY){strcpy(history[history_count],cmd);history_count++;}else{for(int i=1;i<MAX_HISTORY;i++)strcpy(history[i-1],history[i]);strcpy(history[MAX_HISTORY-1],cmd);}}

static void vga_put(int x,int y,char c,uint8_t color){uint16_t* buf=(uint16_t*)0xB8000;if(x>=0&&x<80&&y>=0&&y<25)buf[y*80+x]=vga_entry(c,color);}
static void vga_puts(int x,int y,const char* s,uint8_t color){for(int i=0;s[i];i++)vga_put(x+i,y,s[i],color);}

static void draw_box(int x,int y,int w,int h,const char* title){
    uint16_t* buf=(uint16_t*)0xB8000;
    uint8_t bc=vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK);
    for(int i=x;i<x+w;i++){buf[y*80+i]=vga_entry((i==x||i==x+w-1)?'+':'-',bc);buf[(y+h-1)*80+i]=vga_entry((i==x||i==x+w-1)?'+':'-',bc);}
    for(int r=y+1;r<y+h-1;r++){buf[r*80+x]=vga_entry('|',bc);buf[r*80+x+w-1]=vga_entry('|',bc);}
    if(title){int tl=0;while(title[tl])tl++;int tx=x+(w-tl)/2;for(int i=0;title[i];i++)buf[y*80+tx+i]=vga_entry(title[i],vga_entry_color(VGA_WHITE,VGA_BLACK));}
}

static void print_prompt(void){
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("misu");
    terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));kprintf(":");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_BLUE,VGA_BLACK));kprintf("%s",fs_pwd());
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf(" > ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));
}

/* ===== HELP ===== */
static void cmd_help(void){
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
    kprintf("\n  === General ===\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  help      ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Show this help\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  clear     ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Clear screen\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  echo      ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Echo text\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  info      ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("System info\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  neofetch  ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("ASCII art sysinfo\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  version   ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Version\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  date      ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Date/time\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  time      ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Uptime\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  color     ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Change colors\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  history   ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Command history\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  === Files ===\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  ls pwd cd cat touch write edit mkdir rm\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  === System ===\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  ps spawn kill meminfo alloc syscall\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  === Apps ===\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  calc note cowsay menu files reader sysmon\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  === Games ===\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  game snake ttt ttt2 hangman rps pong\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  === Power ===\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  reboot shutdown exit panic\n\n");
}

/* ===== NEOFETCH ===== */
static void cmd_neofetch(void){
    uint32_t t=timer_get_ticks(),s=t/100,m=s/60;s%=60;kprintf("\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("       /\\_/\\    ");terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("misu@misu-pc\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("      ( o.o )   ");terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));kprintf("----------------\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("       > ^ <    ");kprintf("OS:      ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("MyMisu OS v0.5.0\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("      /|   |\\   ");kprintf("Kernel:  ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("MisuKernel i686\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("     (_|   |_)  ");kprintf("Uptime:  ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("%d min %d sec\n",m,s);
    kprintf("                ");terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("Shell:   ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("misu-sh 2.0\n");
    kprintf("                ");terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("Memory:  ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("%d/%d KB\n",pmm_get_used_pages()*4,pmm_get_total_memory_kb());
    kprintf("                ");terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("Procs:   ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("%d\n",process_get_count());
    kprintf("                ");terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("Files:   ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("%d files, %d dirs\n",fs_get_file_count(),fs_get_dir_count());
    kprintf("                ");for(int i=0;i<8;i++){terminal_setcolor(vga_entry_color(VGA_BLACK,(uint8_t)i));kprintf("   ");}
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("\n                ");
    for(int i=8;i<16;i++){terminal_setcolor(vga_entry_color(VGA_BLACK,(uint8_t)i));kprintf("   ");}
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("\n\n");
}

/* ===== COWSAY ===== */
static void cmd_cowsay(int argc,char** argv){
    char msg[256];msg[0]='\0';if(argc<2)strcpy(msg,"Meow!");
    else{for(int i=1;i<argc;i++){strcat(msg,argv[i]);if(i<argc-1)strcat(msg," ");}}
    size_t len=strlen(msg);terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
    kprintf("\n  ");for(size_t i=0;i<len+2;i++)kprintf("-");kprintf("\n");
    kprintf("  | %s |\n",msg);kprintf("  ");for(size_t i=0;i<len+2;i++)kprintf("-");kprintf("\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
    kprintf("    \\   /\\_/\\\n     \\ ( o.o )\n       > ^ <\n\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
}

/* ===== INFO ===== */
static void cmd_info(void){
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("\n  MyMisu OS v0.5.0\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
    kprintf("  Arch: x86 (i686) | VGA 80x25 | PS/2 | PIT 100Hz\n");
    kprintf("  Memory: %d KB total, %d KB free\n",pmm_get_total_memory_kb(),pmm_get_free_pages()*4);
    kprintf("  Procs: %d | FS: %d files, %d dirs\n",process_get_count(),fs_get_file_count(),fs_get_dir_count());
    kprintf("  Syscalls: write,read,mkdir,getpid,uptime\n");
    kprintf("  Built: GCC 13.2.0 | By: James Mardi, Danny + AI\n\n");
}

/* ===== MEMINFO ===== */
static void cmd_meminfo(void){
    uint32_t total=pmm_get_total_pages(),used=pmm_get_used_pages(),free_p=pmm_get_free_pages();
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  Memory:\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("  Total: %d KB (%d MB)\n",total*4,total*4/1024);
    terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Free:  %d pages (%d KB)\n",free_p,free_p*4);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Used:  %d pages (%d KB)\n",used,used*4);
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("  [");
    uint32_t bw=40,f=(total>0)?(used*bw/total):0;
    for(uint32_t i=0;i<bw;i++){terminal_setcolor(i<f?vga_entry_color(VGA_LIGHT_RED,VGA_BLACK):vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf(i<f?"#":"-");}
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("] %d%%\n\n",total>0?used*100/total:0);
}

/* ===== FILE COMMANDS ===== */
static void cmd_ls(void){char buf[2048];int count=fs_list("/",buf,sizeof(buf));terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  %s (%d entries)\n",fs_pwd(),count);size_t i=0;while(buf[i]){if(buf[i]=='D'&&buf[i+1]=='I'&&buf[i+2]=='R')terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));else terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("  ");while(buf[i]&&buf[i]!='\n'){terminal_putchar(buf[i]);i++;}kprintf("\n");if(buf[i]=='\n')i++;}terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("\n");}
static void cmd_cat(int argc,char** argv){if(argc<2){kprintf("  Usage: cat <file>\n");return;}char buf[4096];int r=fs_read_file(argv[1],buf,sizeof(buf));if(r<0){terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Not found: %s\n",argv[1]);}else{terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("\n%s\n",buf);}terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}
static void cmd_touch(int argc,char** argv){if(argc<2){kprintf("  Usage: touch <file>\n");return;}if(fs_create_file(argv[1])==0){terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Created: %s\n",argv[1]);}else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Failed\n");}terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}
static void cmd_mkdir_shell(int argc,char** argv){if(argc<2){kprintf("  Usage: mkdir <dir>\n");return;}if(fs_mkdir(argv[1])==0){terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Created: %s\n",argv[1]);}else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Failed\n");}terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}
static void cmd_write_file(int argc,char** argv){if(argc<3){kprintf("  Usage: write <file> <text>\n");return;}char data[1024];data[0]='\0';for(int i=2;i<argc;i++){strcat(data,argv[i]);if(i<argc-1)strcat(data," ");}strcat(data,"\n");int r=fs_write_file(argv[1],data,strlen(data));if(r<0){terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Not found (touch first)\n");}else{terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Wrote %d bytes\n",r);}terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}
static void cmd_rm(int argc,char** argv){if(argc<2){kprintf("  Usage: rm <n>\n");return;}if(fs_delete(argv[1])==0){terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Deleted: %s\n",argv[1]);}else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Failed\n");}terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}
static void cmd_cd(int argc,char** argv){if(argc<2){fs_cd("/");return;}if(fs_cd(argv[1])<0){terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  No such dir: %s\n",argv[1]);terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}}

/* ===== EDITOR ===== */
static void cmd_edit(int argc,char** argv){
    if(argc<2){kprintf("  Usage: edit <file>\n");return;}
    char buf[4096];buf[0]='\0';int buflen=0;
    int r=fs_read_file(argv[1],buf,sizeof(buf)-1);if(r>0)buflen=r;else{fs_create_file(argv[1]);}
    terminal_clear();terminal_setcolor(vga_entry_color(VGA_BLACK,VGA_LIGHT_CYAN));
    kprintf("  EDIT: %s  | ESC=save                                                  ",argv[1]);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("\n");if(buflen>0)kprintf("%s",buf);
    while(1){char c=keyboard_getchar();
        if(c==KEY_ESC){buf[buflen]='\0';fs_write_file(argv[1],buf,(size_t)buflen);terminal_clear();terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Saved %s (%d bytes)\n",argv[1],buflen);terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));return;}
        else if(c=='\b'){if(buflen>0){buflen--;terminal_putchar('\b');}}
        else if(c=='\n'){if(buflen<4094){buf[buflen++]='\n';terminal_putchar('\n');}}
        else if(c>=32&&c<127){if(buflen<4094){buf[buflen++]=c;terminal_putchar(c);}}}
}

/* ===== PROCESS COMMANDS ===== */
static void cmd_ps(void){process_t* t=process_get_table();terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  PID  STATE    NAME\n");terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));kprintf("  ---  ------   ----\n");const char* sn[]={"----","RUN ","WAIT","BLCK","DEAD"};for(int i=0;i<MAX_PROCESSES;i++){if(t[i].in_use){terminal_setcolor(vga_entry_color(t[i].state==PROC_RUNNING?VGA_LIGHT_GREEN:VGA_LIGHT_GREY,VGA_BLACK));kprintf("  %d    %s     %s\n",t[i].pid,sn[t[i].state],t[i].name);}}terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("\n");}
static void cmd_spawn(int argc,char** argv){if(argc<2){kprintf("  Usage: spawn <n>\n");return;}int p=process_create(argv[1],5);if(p>=0){terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  PID %d: %s\n",p,argv[1]);}else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Failed\n");}terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}
static void cmd_kill(int argc,char** argv){if(argc<2){kprintf("  Usage: kill <pid>\n");return;}uint32_t p=(uint32_t)atoi(argv[1]);if(p<=1){terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Protected!\n");}else{process_terminate(p);terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Killed %d\n",p);}terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}
static void cmd_syscall_demo(void){terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  Syscall Demo:\n");terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));kprintf("  write(1,\"Hi!\",4)=");int32_t r=sys_write(1,"Hi!\n",4);terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("  -> %d\n",r);terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));kprintf("  write(2,\"Err\",4)=");r=sys_write(2,"Err\n",4);terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("  -> %d\n",r);terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));kprintf("  mkdir(\"test\")=");r=sys_mkdir("test");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("%d\n",r);terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));kprintf("  getpid()=");r=sys_getpid();terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("%d\n",r);terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));kprintf("  uptime()=");r=sys_uptime();terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("%d ticks\n\n",r);}

/* ===== APPS ===== */
static void cmd_calc(int argc,char** argv){if(argc<4){kprintf("  calc <n> <op> <n>\n");return;}int a=atoi(argv[1]),b=atoi(argv[3]),r=0,ok=1;char op=argv[2][0];switch(op){case'+':r=a+b;break;case'-':r=a-b;break;case'*':r=a*b;break;case'/':if(b==0){kprintf("  /0!\n");return;}r=a/b;break;default:ok=0;}if(ok){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("  %d %c %d = %d\n",a,op,b,r);}else kprintf("  Bad op\n");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}
static void cmd_note(int argc,char** argv){if(argc<2){char buf[4096];int r=fs_read_file("notes.txt",buf,sizeof(buf));if(r<0)kprintf("  No notes\n");else{terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  Notes:\n");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("%s\n",buf);}return;}fs_create_file("notes.txt");char data[1024];data[0]='\0';for(int i=1;i<argc;i++){strcat(data,argv[i]);if(i<argc-1)strcat(data," ");}strcat(data,"\n");char ex[4096];int el=fs_read_file("notes.txt",ex,sizeof(ex));if(el>0){strcat(ex,data);fs_write_file("notes.txt",ex,strlen(ex));}else fs_write_file("notes.txt",data,strlen(data));terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Saved!\n");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}
static void cmd_history_show(void){terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  History:\n");for(int i=0;i<history_count;i++){kprintf("  %d  %s\n",i+1,history[i]);}kprintf("\n");}

/* ===== GAMES ===== */
static void cmd_game(void){terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  Guess 1-20 (5 tries)\n\n");uint32_t secret=(timer_get_ticks()%20)+1;int att=5;while(att>0){terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));kprintf("  (%d): ",att);terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));char gb[16];int gi=0;while(1){char c=keyboard_getchar();if(c=='\n'){terminal_putchar('\n');gb[gi]='\0';break;}else if(c=='\b'){if(gi>0){gi--;terminal_putchar('\b');}}else if(gi<15&&c>=32&&c<127){gb[gi++]=c;terminal_putchar(c);}}int g=atoi(gb);if(g==(int)secret){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("  Yes! %d!\n\n",secret);return;}if(g<(int)secret)kprintf("  Higher!\n");else kprintf("  Lower!\n");att--;}kprintf("  It was %d\n\n",secret);terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}

static void cmd_snake(void){
    int sx[SNAKE_MAX],sy[SNAKE_MAX],slen=3,dir=3,fx,fy,score=0,over=0;
    for(int i=0;i<slen;i++){sx[i]=SNAKE_W/2-i;sy[i]=SNAKE_H/2;}
    fx=(timer_get_ticks()%((uint32_t)SNAKE_W-2))+1;fy=(timer_get_ticks()/7%((uint32_t)SNAKE_H-2))+1;
    while(!over){terminal_clear();terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("  SNAKE %d | ESC quit\n\n",score);
        for(int y=0;y<SNAKE_H;y++){kprintf("  ");for(int x=0;x<SNAKE_W;x++){if(y==0||y==SNAKE_H-1||x==0||x==SNAKE_W-1){terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));kprintf("#");}else if(x==fx&&y==fy){terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("@");}else{int is=0;for(int i=0;i<slen;i++)if(sx[i]==x&&sy[i]==y)is=1;if(is){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("O");}else kprintf(" ");}}kprintf("\n");}
        uint32_t st=timer_get_ticks();while(timer_get_ticks()-st<10){if(keyboard_haschar()){char c=keyboard_getchar();if(c==KEY_UP&&dir!=1)dir=0;else if(c==KEY_DOWN&&dir!=0)dir=1;else if(c==KEY_LEFT&&dir!=3)dir=2;else if(c==KEY_RIGHT&&dir!=2)dir=3;else if(c==KEY_ESC){over=1;break;}}asm volatile("hlt");}
        if(over)break;for(int i=slen-1;i>0;i--){sx[i]=sx[i-1];sy[i]=sy[i-1];}
        if(dir==0)sy[0]--;else if(dir==1)sy[0]++;else if(dir==2)sx[0]--;else sx[0]++;
        if(sx[0]<=0||sx[0]>=SNAKE_W-1||sy[0]<=0||sy[0]>=SNAKE_H-1)over=1;
        for(int i=1;i<slen;i++)if(sx[0]==sx[i]&&sy[0]==sy[i])over=1;
        if(sx[0]==fx&&sy[0]==fy){score+=10;if(slen<SNAKE_MAX)slen++;fx=(timer_get_ticks()%((uint32_t)SNAKE_W-2))+1;fy=(timer_get_ticks()/7%((uint32_t)SNAKE_H-2))+1;}
    }terminal_clear();terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("\n  GAME OVER! %d\n\n",score);terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
}

static void cmd_tictactoe(void){char b[9];for(int i=0;i<9;i++)b[i]=' ';int turn=0;int w[8][3]={{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};while(1){terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  TTT You=X CPU=O\n\n");for(int r=0;r<3;r++){kprintf("       ");for(int c=0;c<3;c++){char ch=b[r*3+c];if(ch=='X')terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));else if(ch=='O')terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));else{terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));ch='1'+r*3+c;}kprintf(" %c ",ch);terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));if(c<2)kprintf("|");}kprintf("\n");if(r<2)kprintf("       ---+---+---\n");}kprintf("\n");char wn=0;for(int i=0;i<8;i++)if(b[w[i][0]]!=' '&&b[w[i][0]]==b[w[i][1]]&&b[w[i][1]]==b[w[i][2]])wn=b[w[i][0]];if(wn=='X'){kprintf("  You win!\n\n");return;}if(wn=='O'){kprintf("  CPU wins!\n\n");return;}int fl=1;for(int i=0;i<9;i++)if(b[i]==' ')fl=0;if(fl){kprintf("  Draw!\n\n");return;}if(turn==0){kprintf("  1-9: ");char c=keyboard_getchar();terminal_putchar(c);terminal_putchar('\n');int p=c-'1';if(p>=0&&p<9&&b[p]==' '){b[p]='X';turn=1;}}else{int mv=0;for(int p=0;p<2&&!mv;p++){char ch=p?'X':'O';for(int i=0;i<8;i++){int cn=0,em=-1;for(int j=0;j<3;j++){if(b[w[i][j]]==ch)cn++;else if(b[w[i][j]]==' ')em=w[i][j];}if(cn==2&&em>=0){b[em]='O';mv=1;break;}}}if(!mv&&b[4]==' '){b[4]='O';mv=1;}if(!mv){int co[]={0,2,6,8};for(int i=0;i<4&&!mv;i++)if(b[co[i]]==' '){b[co[i]]='O';mv=1;}}if(!mv)for(int i=0;i<9&&!mv;i++)if(b[i]==' '){b[i]='O';mv=1;}turn=0;}}}

static void cmd_ttt2(void){char b[9];for(int i=0;i<9;i++)b[i]=' ';int turn=0;int w[8][3]={{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};while(1){terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  TTT 2P P1=X P2=O\n\n");for(int r=0;r<3;r++){kprintf("       ");for(int c=0;c<3;c++){char ch=b[r*3+c];if(ch=='X')terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));else if(ch=='O')terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));else{terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));ch='1'+r*3+c;}kprintf(" %c ",ch);terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));if(c<2)kprintf("|");}kprintf("\n");if(r<2)kprintf("       ---+---+---\n");}kprintf("\n");char wn=0;for(int i=0;i<8;i++)if(b[w[i][0]]!=' '&&b[w[i][0]]==b[w[i][1]]&&b[w[i][1]]==b[w[i][2]])wn=b[w[i][0]];if(wn=='X'){kprintf("  P1 wins!\n\n");return;}if(wn=='O'){kprintf("  P2 wins!\n\n");return;}int fl=1;for(int i=0;i<9;i++)if(b[i]==' ')fl=0;if(fl){kprintf("  Draw!\n\n");return;}kprintf("  P%d 1-9: ",turn+1);char c=keyboard_getchar();terminal_putchar(c);terminal_putchar('\n');int p=c-'1';if(p>=0&&p<9&&b[p]==' '){b[p]=turn?'O':'X';turn=1-turn;}}}

static void cmd_hangman(void){const char* words[]={"kernel","memory","process","keyboard","interrupt","system","driver","shell","timer","stack"};const char* word=words[timer_get_ticks()%10];int wl=strlen(word);char g[26];int ng=0,lives=6;while(lives>0){terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  HANGMAN ");terminal_setcolor(vga_entry_color(lives>2?VGA_LIGHT_GREEN:VGA_LIGHT_RED,VGA_BLACK));for(int i=0;i<lives;i++)kprintf("<3 ");kprintf("\n  ");int done=1;for(int i=0;i<wl;i++){int f=0;for(int j=0;j<ng;j++)if(g[j]==word[i])f=1;if(f){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("%c ",word[i]);}else{terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));kprintf("_ ");done=0;}}kprintf("\n");if(done){kprintf("  Won! %s\n\n",word);return;}kprintf("  > ");char c=keyboard_getchar();terminal_putchar(c);terminal_putchar('\n');if(c<'a'||c>'z')continue;int al=0;for(int i=0;i<ng;i++)if(g[i]==c)al=1;if(al)continue;g[ng++]=c;int iw=0;for(int i=0;i<wl;i++)if(word[i]==c)iw=1;if(!iw){lives--;kprintf("  Wrong!\n");}else kprintf("  Yes!\n");}kprintf("  Dead! %s\n\n",word);terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}

static void cmd_rps(void){kprintf("\n  RPS Best of 5\n\n");int pw=0,cw=0,rd=1;while(rd<=5&&pw<3&&cw<3){kprintf("  R%d (r/p/s): ",rd);char c=keyboard_getchar();terminal_putchar(c);terminal_putchar('\n');int pl=-1;if(c=='r')pl=0;else if(c=='p')pl=1;else if(c=='s')pl=2;else{kprintf("  r/p/s!\n");continue;}int cpu=(timer_get_ticks()+rd*7)%3;const char* n[]={"Rock","Paper","Scissors"};kprintf("  %s vs %s: ",n[pl],n[cpu]);if(pl==cpu)kprintf("Draw\n");else if((pl==0&&cpu==2)||(pl==1&&cpu==0)||(pl==2&&cpu==1)){kprintf("You!\n");pw++;}else{kprintf("CPU!\n");cw++;}kprintf("  %d-%d\n\n",pw,cw);rd++;}if(pw>cw)kprintf("  You win!\n\n");else if(cw>pw)kprintf("  CPU wins!\n\n");else kprintf("  Draw!\n\n");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}

static void cmd_pong(void){int pw=40,ph=20,p1y=ph/2-2,p2y=ph/2-2,bx=pw/2,by=ph/2,bdx=1,bdy=1,s1=0,s2=0,pad=4;while(1){terminal_clear();terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("  PONG P1(W/S):%d P2(I/K):%d Q=quit\n\n",s1,s2);for(int y=0;y<ph;y++){kprintf("  ");for(int x=0;x<pw;x++){if(y==0||y==ph-1){terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));kprintf("-");}else if(x==1&&y>=p1y&&y<p1y+pad){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("|");}else if(x==pw-2&&y>=p2y&&y<p2y+pad){terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("|");}else if(x==bx&&y==by){terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("O");}else if(x==pw/2){terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));kprintf(":");}else kprintf(" ");}kprintf("\n");}uint32_t st=timer_get_ticks();while(timer_get_ticks()-st<8){if(keyboard_haschar()){char c=keyboard_getchar();if(c=='w'&&p1y>1)p1y--;else if(c=='s'&&p1y<ph-1-pad)p1y++;else if(c=='i'&&p2y>1)p2y--;else if(c=='k'&&p2y<ph-1-pad)p2y++;else if(c=='q'){terminal_clear();kprintf("\n  %d-%d\n\n",s1,s2);return;}}asm volatile("hlt");}bx+=bdx;by+=bdy;if(by<=1||by>=ph-2)bdy=-bdy;if(bx==2&&by>=p1y&&by<p1y+pad)bdx=1;if(bx==pw-3&&by>=p2y&&by<p2y+pad)bdx=-1;if(bx<=0){s2++;bx=pw/2;by=ph/2;bdx=1;}if(bx>=pw-1){s1++;bx=pw/2;by=ph/2;bdx=-1;}if(s1>=5||s2>=5){terminal_clear();kprintf("\n  %s wins! %d-%d\n\n",s1>s2?"P1":"P2",s1,s2);return;}}terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}

/* ===== UI APPS ===== */
static void cmd_filebrowser(void){
    while(1){char listing[2048];int count=fs_list("/",listing,sizeof(listing));(void)count;
        char names[32][40];char types[32];int n=0;size_t i=0;
        while(listing[i]&&n<32){types[n]=(listing[i]=='D')?'D':'F';i+=6;int ni=0;while(listing[i]&&listing[i]!='\n'&&ni<39){names[n][ni++]=listing[i++];}names[n][ni]='\0';if(listing[i]=='\n')i++;n++;}
        int sel=0;
        while(1){terminal_clear();draw_box(5,1,70,23,"[ File Browser ]");
            vga_puts(8,3,"Dir: ",vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));vga_puts(13,3,fs_pwd(),vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
            vga_puts(8,4,"ENTER=open BS=up N=new D=del ESC=exit",vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
            for(int j=0;j<n&&j<16;j++){uint8_t col;if(j==sel)col=vga_entry_color(VGA_BLACK,VGA_LIGHT_CYAN);else if(types[j]=='D')col=vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK);else col=vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK);vga_puts(10,6+j,types[j]=='D'?"DIR  ":"FILE ",col);vga_puts(15,6+j,names[j],col);}
            if(n==0)vga_puts(10,6,"(empty)",vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
            char c=keyboard_getchar();
            if(c==KEY_UP&&sel>0)sel--;else if(c==KEY_DOWN&&sel<n-1)sel++;
            else if(c=='\n'&&n>0){if(types[sel]=='D'){fs_cd(names[sel]);break;}else{terminal_clear();draw_box(5,1,70,23,names[sel]);char fb[4096];int r=fs_read_file(names[sel],fb,sizeof(fb));if(r>0){int row=3,col2=7;for(int fi=0;fb[fi]&&row<23;fi++){if(fb[fi]=='\n'){row++;col2=7;}else{vga_put(col2,row,fb[fi],vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));col2++;if(col2>72){col2=7;row++;}}}}vga_puts(7,24,"Any key to go back",vga_entry_color(VGA_DARK_GREY,VGA_BLACK));keyboard_getchar();}}
            else if(c=='\b'){fs_cd("..");break;}
            else if(c=='n'||c=='N'){terminal_clear();kprintf("\n  Filename: ");char fn[32];int fi=0;while(1){char fc=keyboard_getchar();if(fc=='\n'){fn[fi]='\0';break;}else if(fc=='\b'){if(fi>0){fi--;terminal_putchar('\b');}}else if(fi<31&&fc>=32&&fc<127){fn[fi++]=fc;terminal_putchar(fc);}}if(fi>0)fs_create_file(fn);break;}
            else if((c=='d'||c=='D')&&n>0){fs_delete(names[sel]);break;}
            else if(c==KEY_ESC)return;
        }
    }
}

static void cmd_reader(void){
    terminal_clear();kprintf("\n  File to read: ");char fn[32];int fi=0;
    while(1){char c=keyboard_getchar();if(c=='\n'){fn[fi]='\0';break;}else if(c=='\b'){if(fi>0){fi--;terminal_putchar('\b');}}else if(fi<31&&c>=32&&c<127){fn[fi++]=c;terminal_putchar(c);}}
    char ct[4096];int len=fs_read_file(fn,ct,sizeof(ct));
    if(len<=0){kprintf("\n  Not found!\n");return;}
    int lpp=18,off=0,page=1;
    while(off<len){terminal_clear();draw_box(2,0,76,24,fn);
        int row=2,col=4,lc=0,i=off;
        while(i<len&&lc<lpp){if(ct[i]=='\n'){row++;col=4;lc++;i++;continue;}vga_put(col,row,ct[i],vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));col++;if(col>74){col=4;row++;lc++;}i++;}
        int nxt=i;vga_puts(4,24,"LEFT=prev RIGHT=next ESC=exit",vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
        char c=keyboard_getchar();
        if((c==KEY_RIGHT||c=='\n')&&nxt<len){off=nxt;page++;}
        else if(c==KEY_LEFT&&page>1){page--;off=0;for(int p=0;p<page-1;p++){int lc2=0;int j=off;while(j<len&&lc2<lpp){if(ct[j]=='\n')lc2++;j++;}off=j;}}
        else if(c==KEY_ESC)break;
    }terminal_clear();
}

static void cmd_sysmon(void){
    while(1){terminal_clear();draw_box(0,0,80,25,"[ System Monitor ]");
        vga_puts(3,2,"MyMisu OS v0.5.0",vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
        uint32_t tk=timer_get_ticks(),sec=tk/100,mn=sec/60;sec%=60;
        char ub[16];int ui=0;if(mn>=10)ub[ui++]='0'+mn/10;ub[ui++]='0'+mn%10;ub[ui++]='m';ub[ui++]=' ';if(sec>=10)ub[ui++]='0'+sec/10;ub[ui++]='0'+sec%10;ub[ui++]='s';ub[ui]='\0';
        vga_puts(3,4,"Uptime:",vga_entry_color(VGA_WHITE,VGA_BLACK));vga_puts(12,4,ub,vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));
        vga_puts(3,6,"Memory:",vga_entry_color(VGA_WHITE,VGA_BLACK));
        uint32_t total=pmm_get_total_pages(),used=pmm_get_used_pages();
        vga_puts(3,7,"[",vga_entry_color(VGA_WHITE,VGA_BLACK));uint32_t bw=50,fl=(total>0)?(used*bw/total):0;
        for(uint32_t i=0;i<bw;i++)vga_put(4+i,7,i<fl?'#':'-',i<fl?vga_entry_color(VGA_LIGHT_RED,VGA_BLACK):vga_entry_color(VGA_GREEN,VGA_BLACK));
        vga_puts(54,7,"]",vga_entry_color(VGA_WHITE,VGA_BLACK));
        vga_puts(3,9,"Processes:",vga_entry_color(VGA_WHITE,VGA_BLACK));
        process_t* pt=process_get_table();int row=11;
        vga_puts(3,10,"PID  STATE  NAME",vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
        const char* sn[]={"----","RUN ","WAIT","BLCK","DEAD"};
        for(int i=0;i<MAX_PROCESSES&&row<20;i++){if(pt[i].in_use){uint8_t cl=pt[i].state==PROC_RUNNING?vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK):vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK);char pc='0'+pt[i].pid%10;vga_put(3,row,pc,cl);vga_puts(8,row,sn[pt[i].state],cl);vga_puts(14,row,pt[i].name,cl);row++;}}
        vga_puts(45,4,"Files:",vga_entry_color(VGA_WHITE,VGA_BLACK));
        vga_puts(45,6,"Dirs:",vga_entry_color(VGA_WHITE,VGA_BLACK));
        outb(0x70,0x04);uint8_t hr=inb(0x71);outb(0x70,0x02);uint8_t mi=inb(0x71);outb(0x70,0x00);uint8_t sc=inb(0x71);
        hr=(hr&0x0F)+((hr>>4)*10);mi=(mi&0x0F)+((mi>>4)*10);sc=(sc&0x0F)+((sc>>4)*10);
        char tb[12];tb[0]='0'+hr/10;tb[1]='0'+hr%10;tb[2]=':';tb[3]='0'+mi/10;tb[4]='0'+mi%10;tb[5]=':';tb[6]='0'+sc/10;tb[7]='0'+sc%10;tb[8]='\0';
        vga_puts(60,2,tb,vga_entry_color(VGA_YELLOW,VGA_BLACK));
        vga_puts(3,23,"ESC to exit",vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
        uint32_t st=timer_get_ticks();while(timer_get_ticks()-st<50){if(keyboard_haschar()){char c=keyboard_getchar();if(c==KEY_ESC){terminal_clear();return;}}asm volatile("hlt");}
    }
}


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

static void cmd_menu(void){
    const char* items[]={"Neofetch","File Browser","Book Reader","System Monitor","Desktop Widgets","Calculator","Games","Shell (back)","Shutdown"};int ni=9,sel=0;
    while(1){terminal_clear();draw_box(15,2,50,22,"[ MyMisu OS - Menu ]");
        vga_puts(20,4,"UP/DOWN + ENTER | ESC=back",vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
        for(int i=0;i<ni;i++){uint8_t col=i==sel?vga_entry_color(VGA_BLACK,VGA_LIGHT_CYAN):vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK);
            vga_puts(25,7+i*2,i==sel?" > ":"   ",col);vga_puts(28,7+i*2,items[i],col);int l=0;while(items[i][l])l++;for(int p=l;p<18;p++)vga_put(28+p,7+i*2,' ',col);}
        char c=keyboard_getchar();
        if(c==KEY_UP&&sel>0)sel--;else if(c==KEY_DOWN&&sel<ni-1)sel++;
        else if(c=='\n'){terminal_clear();
            if(sel==0)cmd_neofetch();else if(sel==1)cmd_filebrowser();else if(sel==2)cmd_reader();
            else if(sel==3)cmd_sysmon();else if(sel==4)cmd_widgets();else if(sel==5){kprintf("  Use: calc <n> <op> <n>\n");}
            else if(sel==6){const char* gm[]={"snake","ttt","ttt2","hangman","rps","pong","game"};int ng2=7,gs=0;
                while(1){terminal_clear();draw_box(20,3,40,18,"[ Games ]");for(int i=0;i<ng2;i++)vga_puts(28,6+i*2,gm[i],i==gs?vga_entry_color(VGA_BLACK,VGA_LIGHT_GREEN):vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
                    char gc=keyboard_getchar();if(gc==KEY_UP&&gs>0)gs--;else if(gc==KEY_DOWN&&gs<ng2-1)gs++;else if(gc==KEY_ESC)break;
                    else if(gc=='\n'){terminal_clear();if(gs==0)cmd_snake();else if(gs==1)cmd_tictactoe();else if(gs==2)cmd_ttt2();else if(gs==3)cmd_hangman();else if(gs==4)cmd_rps();else if(gs==5)cmd_pong();else cmd_game();break;}}}
            else if(sel==7)return;else if(sel==8){cmd_halt();return;}
        }else if(c==KEY_ESC)return;
    }
}

/* ===== POWER ===== */
static void cmd_halt(void){terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));kprintf("\n  Halted.\n");asm volatile("cli;hlt");}
static void cmd_reboot(void){terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));kprintf("\n  Rebooting...\n");struct{uint16_t l;uint32_t b;}__attribute__((packed)) ni={0,0};asm volatile("lidt %0"::"m"(ni));asm volatile("int $0x03");}

/* ===== EXECUTE ===== */
static void execute_command(char* input){
    char* argv[MAX_ARGS];int argc=0;char* tok=input;
    while(*tok&&argc<MAX_ARGS){while(*tok==' ')tok++;if(!*tok)break;argv[argc++]=tok;while(*tok&&*tok!=' ')tok++;if(*tok){*tok='\0';tok++;}}
    if(!argc)return;
    if(strcmp(argv[0],"help")==0){terminal_enable_pager();cmd_help();terminal_disable_pager();}
    else if(strcmp(argv[0],"clear")==0)terminal_clear();
    else if(strcmp(argv[0],"echo")==0){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));for(int i=1;i<argc;i++){kprintf("%s",argv[i]);if(i<argc-1)kprintf(" ");}kprintf("\n");}
    else if(strcmp(argv[0],"info")==0)cmd_info();
    else if(strcmp(argv[0],"neofetch")==0)cmd_neofetch();
    else if(strcmp(argv[0],"version")==0){terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("\n  MyMisu OS v0.5.0\n");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("  MisuKernel i686 | misu-sh 2.0 | GCC 13.2.0\n\n");}
    else if(strcmp(argv[0],"date")==0){outb(0x70,0x00);uint8_t sec=inb(0x71);outb(0x70,0x02);uint8_t min=inb(0x71);outb(0x70,0x04);uint8_t hr=inb(0x71);outb(0x70,0x07);uint8_t day=inb(0x71);outb(0x70,0x08);uint8_t mon=inb(0x71);outb(0x70,0x09);uint8_t yr=inb(0x71);sec=(sec&0xF)+((sec>>4)*10);min=(min&0xF)+((min>>4)*10);hr=(hr&0xF)+((hr>>4)*10);day=(day&0xF)+((day>>4)*10);mon=(mon&0xF)+((mon>>4)*10);yr=(yr&0xF)+((yr>>4)*10);kprintf("\n  20%d-%d-%d %d:%d:%d\n\n",yr,mon,day,hr,min,sec);}
    else if(strcmp(argv[0],"time")==0){uint32_t t=timer_get_ticks(),s=t/100,m=s/60;s%=60;kprintf("\n  %dm %ds\n\n",m,s);}
    else if(strcmp(argv[0],"color")==0){if(argc<3)kprintf("  color <fg> <bg>\n");else{int fg=atoi(argv[1]),bg=atoi(argv[2]);if(fg>=0&&fg<16&&bg>=0&&bg<16){terminal_setcolor(vga_entry_color((uint8_t)fg,(uint8_t)bg));kprintf("  Done!\n");}else kprintf("  0-15!\n");}}
    else if(strcmp(argv[0],"history")==0)cmd_history_show();
    else if(strcmp(argv[0],"ls")==0)cmd_ls();
    else if(strcmp(argv[0],"pwd")==0)kprintf("  %s\n",fs_pwd());
    else if(strcmp(argv[0],"cd")==0)cmd_cd(argc,argv);
    else if(strcmp(argv[0],"cat")==0)cmd_cat(argc,argv);
    else if(strcmp(argv[0],"touch")==0)cmd_touch(argc,argv);
    else if(strcmp(argv[0],"write")==0)cmd_write_file(argc,argv);
    else if(strcmp(argv[0],"edit")==0)cmd_edit(argc,argv);
    else if(strcmp(argv[0],"mkdir")==0)cmd_mkdir_shell(argc,argv);
    else if(strcmp(argv[0],"rm")==0)cmd_rm(argc,argv);
    else if(strcmp(argv[0],"ps")==0)cmd_ps();
    else if(strcmp(argv[0],"spawn")==0)cmd_spawn(argc,argv);
    else if(strcmp(argv[0],"kill")==0)cmd_kill(argc,argv);
    else if(strcmp(argv[0],"meminfo")==0)cmd_meminfo();
    else if(strcmp(argv[0],"alloc")==0){uint32_t a=pmm_alloc_page();if(a)kprintf("  0x%x\n",a);else kprintf("  OOM!\n");}
    else if(strcmp(argv[0],"syscall")==0)cmd_syscall_demo();
    else if(strcmp(argv[0],"calc")==0)cmd_calc(argc,argv);
    else if(strcmp(argv[0],"note")==0)cmd_note(argc,argv);
    else if(strcmp(argv[0],"cowsay")==0)cmd_cowsay(argc,argv);
    else if(strcmp(argv[0],"game")==0)cmd_game();
    else if(strcmp(argv[0],"snake")==0)cmd_snake();
    else if(strcmp(argv[0],"ttt")==0)cmd_tictactoe();
    else if(strcmp(argv[0],"ttt2")==0)cmd_ttt2();
    else if(strcmp(argv[0],"hangman")==0)cmd_hangman();
    else if(strcmp(argv[0],"rps")==0)cmd_rps();
    else if(strcmp(argv[0],"pong")==0)cmd_pong();
    else if(strcmp(argv[0],"menu")==0)cmd_menu();
    else if(strcmp(argv[0],"files")==0)cmd_filebrowser();
    else if(strcmp(argv[0],"reader")==0)cmd_reader();
    else if(strcmp(argv[0],"sysmon")==0)cmd_sysmon();
    else if(strcmp(argv[0],"widgets")==0)cmd_widgets();
    else if(strcmp(argv[0],"reboot")==0)cmd_reboot();
    else if(strcmp(argv[0],"shutdown")==0||strcmp(argv[0],"poweroff")==0||strcmp(argv[0],"exit")==0)cmd_halt();
    else if(strcmp(argv[0],"panic")==0){terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_RED));kprintf("\n  KERNEL PANIC!\n");asm volatile("cli;hlt");}
    else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Unknown: %s\n",argv[0]);terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("  Type 'help'\n");}
}

/* ===== LOGIN ===== */
void login_screen(void){
    terminal_clear();terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
    kprintf("\n\n\n\n\n");
    kprintf("                  |\\---/|\n");
    kprintf("                  | ,_, |\n");
    kprintf("                   \\_`_/-..--.  \n");
    kprintf("                ___/ `   ' ,+ \\\n");
    kprintf("               (__...'  _/  |`._;  \n");
    kprintf("                 (_,..'(_,.`__)  \n\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("              Welcome to MyMisu OS\n\n");
    terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));kprintf("         --------------------------------\n\n");
    while(1){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("           Username: ");terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
        char user[32];int ui=0;while(1){char c=keyboard_getchar();if(c=='\n'){terminal_putchar('\n');user[ui]='\0';break;}else if(c=='\b'){if(ui>0){ui--;terminal_putchar('\b');}}else if(ui<31&&c>=32&&c<127){user[ui++]=c;terminal_putchar(c);}}
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("           Password: ");terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
        char pass[32];int pi=0;while(1){char c=keyboard_getchar();if(c=='\n'){terminal_putchar('\n');pass[pi]='\0';break;}else if(c=='\b'){if(pi>0){pi--;terminal_putchar('\b');}}else if(pi<31&&c>=32&&c<127){pass[pi++]=c;terminal_putchar('*');}}
        if((strcmp(user,"misu")==0&&strcmp(pass,"misu")==0)||(strcmp(user,"admin")==0&&strcmp(pass,"admin")==0)||(strcmp(user,"james")==0&&strcmp(pass,"1234")==0)){
            terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("\n           Welcome, %s!\n",user);
            uint32_t w=timer_get_ticks();while(timer_get_ticks()-w<150)asm volatile("hlt");terminal_clear();return;}
        else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("\n           Invalid.\n\n");}
    }
}

void shell_init(void){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("\n  Type 'help' or 'menu' for visual navigation.\n\n");}

void shell_run(void){
    shell_init();
    while(1){print_prompt();cmd_len=0;history_pos=history_count;
        while(1){char c=keyboard_getchar();
            if(c=='\n'){terminal_putchar('\n');cmd_buffer[cmd_len]='\0';break;}
            else if(c=='\b'){if(cmd_len>0){cmd_len--;terminal_putchar('\b');}}
            else if(c==KEY_UP){if(history_pos>0){history_pos--;while(cmd_len>0){terminal_putchar('\b');cmd_len--;}strcpy(cmd_buffer,history[history_pos]);cmd_len=strlen(cmd_buffer);terminal_writestring(cmd_buffer);}}
            else if(c==KEY_DOWN){if(history_pos<history_count-1){history_pos++;while(cmd_len>0){terminal_putchar('\b');cmd_len--;}strcpy(cmd_buffer,history[history_pos]);cmd_len=strlen(cmd_buffer);terminal_writestring(cmd_buffer);}else{history_pos=history_count;while(cmd_len>0){terminal_putchar('\b');cmd_len--;}}}
            else if(c>=32&&c<127&&cmd_len<CMD_BUFFER_SIZE-1){cmd_buffer[cmd_len++]=c;terminal_putchar(c);}
        }
        if(cmd_len>0){history_add(cmd_buffer);execute_command(cmd_buffer);}
    }
}
