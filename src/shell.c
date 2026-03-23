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

static char cmd_buffer[CMD_BUFFER_SIZE];
static int cmd_len = 0;

static void print_prompt(void) {
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK));
    kprintf("misu");
    terminal_setcolor(vga_entry_color(VGA_DARK_GREY, VGA_BLACK));
    kprintf(":");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_BLUE, VGA_BLACK));
    kprintf("%s", fs_pwd());
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf(" > ");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK));
}

static void cmd_help(void) {
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("\n  === General ===\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  help      "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Show this help\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  clear     "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Clear screen\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  echo      "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Echo text\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  info      "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("System info\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  neofetch  "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("System info + ASCII art\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  version   "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Show version\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  date      "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Date and time\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  time      "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Uptime\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  color     "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Change colors\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("\n  === Files & Directories ===\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  ls        "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("List files\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  pwd       "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Current directory\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  cd        "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Change directory\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  cat       "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("View file\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  touch     "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Create file\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  write     "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Write to file\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  mkdir     "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Create directory\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  rm        "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Delete file/dir\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("\n  === Processes & Memory ===\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  ps        "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("List processes\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  spawn     "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Create process\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  kill      "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Kill process\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  meminfo   "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Memory stats\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  alloc     "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Allocate page\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  syscall   "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Demo system calls\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("\n  === Apps ===\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  calc      "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Calculator (calc 5 + 3)\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  note      "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Quick notes\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  game      "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Number guessing game\n");
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    kprintf("\n  === System ===\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  reboot    "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Reboot\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  shutdown  "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Shut down\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  exit      "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Halt system\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("  panic     "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("Kernel panic\n\n");
}

static void cmd_neofetch(void) {
    uint32_t ticks = timer_get_ticks(); uint32_t seconds = ticks / 100; uint32_t minutes = seconds / 60; seconds %= 60;
    kprintf("\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("       /\\_/\\    "); terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK)); kprintf("misu@misu-pc\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("      ( o.o )   "); terminal_setcolor(vga_entry_color(VGA_DARK_GREY, VGA_BLACK)); kprintf("----------------\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("       > ^ <    "); kprintf("OS:      "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("MyMisu OS v0.3.0\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("      /|   |\\   "); kprintf("Kernel:  "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("MisuKernel i686\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("     (_|   |_)  "); kprintf("Uptime:  "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("%d min %d sec\n", minutes, seconds);
    kprintf("                "); terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("Shell:   "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("misu-sh 1.0\n");
    kprintf("                "); terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("Memory:  "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("%d KB / %d KB\n", pmm_get_used_pages()*4, pmm_get_total_memory_kb());
    kprintf("                "); terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("Procs:   "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("%d running\n", process_get_count());
    kprintf("                "); terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("Files:   "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("%d files, %d dirs\n", fs_get_file_count(), fs_get_dir_count());
    kprintf("                "); terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("Display: "); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("VGA 80x25 (text)\n");
    kprintf("                "); for (int i=0;i<8;i++){terminal_setcolor(vga_entry_color(VGA_BLACK,(uint8_t)i));kprintf("   ");}
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("\n                ");
    for (int i=8;i<16;i++){terminal_setcolor(vga_entry_color(VGA_BLACK,(uint8_t)i));kprintf("   ");}
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("\n\n");
}

static void cmd_info(void) {
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN, VGA_BLACK)); kprintf("\n  MyMisu OS v0.3.0\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("  Architecture:  x86 (i686)\n"); kprintf("  Display:       VGA text mode 80x25\n");
    kprintf("  Keyboard:      PS/2 (US QWERTY)\n"); kprintf("  Timer:         PIT at 100 Hz\n");
    kprintf("  Memory:        %d KB total, %d KB free\n", pmm_get_total_memory_kb(), pmm_get_free_pages()*4);
    kprintf("  Processes:     %d active\n", process_get_count());
    kprintf("  Filesystem:    Ramdisk (%d files, %d dirs)\n", fs_get_file_count(), fs_get_dir_count());
    kprintf("  System calls:  write, read, mkdir, getpid, uptime\n");
    kprintf("  Language:      C (freestanding)\n"); kprintf("  Built with:    GCC 13.2.0 (i686-elf)\n");
    kprintf("  Created by:    James Mardi, Danny + AI\n\n");
}

static void cmd_meminfo(void) {
    uint32_t total=pmm_get_total_pages(),used=pmm_get_used_pages(),free_p=pmm_get_free_pages();
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK)); kprintf("\n  Memory Statistics:\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("  Total:  %d KB (%d MB)\n", total*4, total*4/1024); kprintf("  Pages:  %d (4 KB each)\n", total);
    terminal_setcolor(vga_entry_color(VGA_GREEN, VGA_BLACK)); kprintf("  Free:   %d (%d KB)\n", free_p, free_p*4);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_RED, VGA_BLACK)); kprintf("  Used:   %d (%d KB)\n", used, used*4);
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK)); kprintf("  Bar:    [");
    uint32_t bw=40,filled=(total>0)?(used*bw/total):0;
    for(uint32_t i=0;i<bw;i++){terminal_setcolor(i<filled?vga_entry_color(VGA_LIGHT_RED,VGA_BLACK):vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf(i<filled?"#":"-");}
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK)); kprintf("] %d%%\n\n", total>0?used*100/total:0);
}

static void cmd_ls(void) {
    char buf[2048]; int count=fs_list("/",buf,sizeof(buf));
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK)); kprintf("\n  Directory: %s (%d entries)\n", fs_pwd(), count);
    size_t i=0;
    while(buf[i]){
        if(buf[i]=='D'&&buf[i+1]=='I'&&buf[i+2]=='R') terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
        else terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));
        kprintf("  "); while(buf[i]&&buf[i]!='\n'){terminal_putchar(buf[i]);i++;} kprintf("\n"); if(buf[i]=='\n')i++;
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("\n");
}

static void cmd_cat(int argc, char** argv) {
    if(argc<2){kprintf("  Usage: cat <filename>\n");return;}
    char buf[4096]; int r=fs_read_file(argv[1],buf,sizeof(buf));
    if(r<0){terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  File not found: %s\n",argv[1]);}
    else{terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("\n%s\n",buf);}
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_touch(int argc, char** argv) {
    if(argc<2){kprintf("  Usage: touch <filename>\n");return;}
    if(fs_create_file(argv[1])==0){terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Created: %s\n",argv[1]);}
    else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Failed\n");}
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_mkdir_shell(int argc, char** argv) {
    if(argc<2){kprintf("  Usage: mkdir <dirname>\n");return;}
    if(fs_mkdir(argv[1])==0){terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Created: %s\n",argv[1]);}
    else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Failed\n");}
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_write_file(int argc, char** argv) {
    if(argc<3){kprintf("  Usage: write <file> <text...>\n");return;}
    char data[1024]; data[0]='\0';
    for(int i=2;i<argc;i++){strcat(data,argv[i]);if(i<argc-1)strcat(data," ");}
    strcat(data,"\n");
    int r=fs_write_file(argv[1],data,strlen(data));
    if(r<0){terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Not found: %s\n",argv[1]);}
    else{terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Wrote %d bytes to %s\n",r,argv[1]);}
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_rm(int argc, char** argv) {
    if(argc<2){kprintf("  Usage: rm <name>\n");return;}
    if(fs_delete(argv[1])==0){terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Deleted: %s\n",argv[1]);}
    else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Failed\n");}
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_cd(int argc, char** argv) {
    if(argc<2){fs_cd("/");return;}
    if(fs_cd(argv[1])<0){terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  No such directory: %s\n",argv[1]);terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}
}

static void cmd_ps(void) {
    process_t* t=process_get_table();
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK)); kprintf("\n  PID  STATE      TICKS    NAME\n");
    terminal_setcolor(vga_entry_color(VGA_DARK_GREY, VGA_BLACK)); kprintf("  ---  --------   ------   ----\n");
    const char* sn[]={"UNUSED","RUNNING","READY","BLOCKED","DEAD"};
    for(int i=0;i<MAX_PROCESSES;i++){
        if(t[i].in_use){
            if(t[i].state==PROC_RUNNING) terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));
            else if(t[i].state==PROC_READY) terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));
            else terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
            kprintf("  %d    %s    %d       %s\n",t[i].pid,sn[t[i].state],t[i].ticks_used,t[i].name);
        }
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("\n  Total: %d processes\n\n",process_get_count());
}

static void cmd_spawn(int argc, char** argv) {
    if(argc<2){kprintf("  Usage: spawn <name>\n");return;}
    int pid=process_create(argv[1],5);
    if(pid>=0){terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Created '%s' (PID %d)\n",argv[1],pid);}
    else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Failed\n");}
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_kill(int argc, char** argv) {
    if(argc<2){kprintf("  Usage: kill <pid>\n");return;}
    uint32_t pid=(uint32_t)atoi(argv[1]);
    if(pid<=1){terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Cannot kill kernel/shell!\n");}
    else{process_terminate(pid);terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Killed PID %d\n",pid);}
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_syscall_demo(void) {
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK)); kprintf("\n  System Call Demo:\n\n");
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK)); kprintf("  sys_write(STDOUT, \"Hello from syscall!\", 20)...\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN, VGA_BLACK)); kprintf("  > ");
    int32_t r=sys_write(STDOUT,"Hello from syscall!\n",20);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("  Returned: %d bytes\n",r);
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK)); kprintf("\n  sys_write(STDERR, \"Error!\", 7)...\n  > ");
    r=sys_write(STDERR,"Error!\n",7);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("  Returned: %d bytes\n",r);
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK)); kprintf("\n  sys_mkdir(\"syscall_dir\")...\n");
    r=sys_mkdir("syscall_dir"); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("  Returned: %d\n",r);
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK)); kprintf("\n  sys_getpid()...\n");
    r=sys_getpid(); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("  Returned: PID %d\n",r);
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK)); kprintf("\n  sys_uptime()...\n");
    r=sys_uptime(); terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("  Returned: %d ticks (%d sec)\n\n",r,r/100);
}

static void cmd_calc(int argc, char** argv) {
    if(argc<4){kprintf("  Usage: calc <n> <op> <n>  (ops: + - * /)\n");return;}
    int a=atoi(argv[1]),b=atoi(argv[3]),r=0,ok=1; char op=argv[2][0];
    switch(op){case'+':r=a+b;break;case'-':r=a-b;break;case'*':r=a*b;break;
    case'/':if(b==0){terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Division by zero!\n");return;}r=a/b;break;default:ok=0;}
    if(ok){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("  %d %c %d = %d\n",a,op,b,r);}
    else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Unknown op: %c\n",op);}
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_note(int argc, char** argv) {
    if(argc<2){
        char buf[4096]; int r=fs_read_file("notes.txt",buf,sizeof(buf));
        if(r<0) kprintf("  No notes. Usage: note <text>\n");
        else{terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\n  === Notes ===\n");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("%s\n",buf);}
        return;
    }
    fs_create_file("notes.txt");
    char data[1024]; data[0]='\0';
    for(int i=1;i<argc;i++){strcat(data,argv[i]);if(i<argc-1)strcat(data," ");}
    strcat(data,"\n");
    char existing[4096]; int el=fs_read_file("notes.txt",existing,sizeof(existing));
    if(el>0){strcat(existing,data);fs_write_file("notes.txt",existing,strlen(existing));}
    else fs_write_file("notes.txt",data,strlen(data));
    terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Note saved!\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_game(void) {
    terminal_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK)); kprintf("\n  === Number Guessing Game ===\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK)); kprintf("  Guess 1-20. You have 5 tries.\n\n");
    uint32_t secret=(timer_get_ticks()%20)+1; int att=5;
    while(att>0){
        terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));kprintf("  Guess (%d left): ",att);
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));
        char gb[16];int gi=0;
        while(1){char c=keyboard_getchar();if(c=='\n'){terminal_putchar('\n');gb[gi]='\0';break;}else if(c=='\b'){if(gi>0){gi--;terminal_putchar('\b');}}else if(gi<15){gb[gi++]=c;terminal_putchar(c);}}
        int g=atoi(gb);
        if(g==(int)secret){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("  Correct! It was %d!\n\n",secret);return;}
        else if(g<(int)secret){terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  Too low!\n");}
        else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Too high!\n");}
        att--;
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Game over! It was %d.\n\n",secret);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
}

static void cmd_halt(void) {
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK));
    kprintf("\n  System halted. Power off safely.\n");
    asm volatile("cli; hlt");
}

static void cmd_reboot(void) {
    terminal_setcolor(vga_entry_color(VGA_YELLOW, VGA_BLACK)); kprintf("\n  Rebooting...\n");
    struct{uint16_t limit;uint32_t base;}__attribute__((packed)) null_idt={0,0};
    asm volatile("lidt %0"::"m"(null_idt)); asm volatile("int $0x03");
}

static void execute_command(char* input) {
    char* argv[MAX_ARGS]; int argc=0; char* token=input;
    while(*token&&argc<MAX_ARGS){while(*token==' ')token++;if(*token=='\0')break;argv[argc++]=token;while(*token&&*token!=' ')token++;if(*token){*token='\0';token++;}}
    if(argc==0)return;
    if(strcmp(argv[0],"help")==0) cmd_help();
    else if(strcmp(argv[0],"clear")==0) terminal_clear();
    else if(strcmp(argv[0],"echo")==0){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));for(int i=1;i<argc;i++){kprintf("%s",argv[i]);if(i<argc-1)kprintf(" ");}kprintf("\n");}
    else if(strcmp(argv[0],"info")==0) cmd_info();
    else if(strcmp(argv[0],"neofetch")==0) cmd_neofetch();
    else if(strcmp(argv[0],"version")==0){terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("\n  MyMisu OS v0.3.0\n");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("  Kernel:    MisuKernel i686\n  Shell:     misu-sh 1.0\n  Compiler:  GCC 13.2.0\n  Built:     March 2026\n\n");}
    else if(strcmp(argv[0],"date")==0){outb(0x70,0x00);uint8_t sec=inb(0x71);outb(0x70,0x02);uint8_t min=inb(0x71);outb(0x70,0x04);uint8_t hr=inb(0x71);outb(0x70,0x07);uint8_t day=inb(0x71);outb(0x70,0x08);uint8_t mon=inb(0x71);outb(0x70,0x09);uint8_t yr=inb(0x71);sec=(sec&0x0F)+((sec>>4)*10);min=(min&0x0F)+((min>>4)*10);hr=(hr&0x0F)+((hr>>4)*10);day=(day&0x0F)+((day>>4)*10);mon=(mon&0x0F)+((mon>>4)*10);yr=(yr&0x0F)+((yr>>4)*10);terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("\n  Date: 20%d-%d-%d  Time: %d:%d:%d\n\n",yr,mon,day,hr,min,sec);}
    else if(strcmp(argv[0],"time")==0){uint32_t t=timer_get_ticks(),s=t/100,m=s/60;s%=60;terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("\n  Uptime: %d min %d sec (%d ticks)\n\n",m,s,t);}
    else if(strcmp(argv[0],"color")==0){if(argc<3){kprintf("  Usage: color <fg> <bg>\n");return;}int fg=atoi(argv[1]),bg=atoi(argv[2]);if(fg<0||fg>15||bg<0||bg>15){kprintf("  Invalid\n");return;}terminal_setcolor(vga_entry_color((uint8_t)fg,(uint8_t)bg));kprintf("  Color changed!\n");}
    else if(strcmp(argv[0],"ls")==0) cmd_ls();
    else if(strcmp(argv[0],"pwd")==0){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("  %s\n",fs_pwd());}
    else if(strcmp(argv[0],"cd")==0) cmd_cd(argc,argv);
    else if(strcmp(argv[0],"cat")==0) cmd_cat(argc,argv);
    else if(strcmp(argv[0],"touch")==0) cmd_touch(argc,argv);
    else if(strcmp(argv[0],"write")==0) cmd_write_file(argc,argv);
    else if(strcmp(argv[0],"mkdir")==0) cmd_mkdir_shell(argc,argv);
    else if(strcmp(argv[0],"rm")==0) cmd_rm(argc,argv);
    else if(strcmp(argv[0],"ps")==0) cmd_ps();
    else if(strcmp(argv[0],"spawn")==0) cmd_spawn(argc,argv);
    else if(strcmp(argv[0],"kill")==0) cmd_kill(argc,argv);
    else if(strcmp(argv[0],"meminfo")==0) cmd_meminfo();
    else if(strcmp(argv[0],"alloc")==0){uint32_t a=pmm_alloc_page();if(a){terminal_setcolor(vga_entry_color(VGA_GREEN,VGA_BLACK));kprintf("  Allocated at 0x%x\n",a);}else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Out of memory!\n");}terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));}
    else if(strcmp(argv[0],"syscall")==0) cmd_syscall_demo();
    else if(strcmp(argv[0],"calc")==0) cmd_calc(argc,argv);
    else if(strcmp(argv[0],"note")==0) cmd_note(argc,argv);
    else if(strcmp(argv[0],"game")==0) cmd_game();
    else if(strcmp(argv[0],"reboot")==0) cmd_reboot();
    else if(strcmp(argv[0],"shutdown")==0) cmd_halt();
    else if(strcmp(argv[0],"poweroff")==0) cmd_halt();
    else if(strcmp(argv[0],"exit")==0) cmd_halt();
    else if(strcmp(argv[0],"panic")==0){volatile int x=0;volatile int y=1/x;(void)y;}
    else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Unknown command: %s\n",argv[0]);terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("  Type 'help' for available commands.\n");}
}

void shell_init(void) {
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK));
    kprintf("\n  Type 'help' for a list of commands.\n\n");
}

void shell_run(void) {
    shell_init();
    while(1){
        print_prompt(); cmd_len=0;
        while(1){char c=keyboard_getchar();if(c=='\n'){terminal_putchar('\n');cmd_buffer[cmd_len]='\0';break;}else if(c=='\b'){if(cmd_len>0){cmd_len--;terminal_putchar('\b');}}else if(cmd_len<CMD_BUFFER_SIZE-1){cmd_buffer[cmd_len++]=c;terminal_putchar(c);}}
        if(cmd_len>0)execute_command(cmd_buffer);
    }
}
