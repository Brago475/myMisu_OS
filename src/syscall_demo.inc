static void cmd_syscall_demo(void){
    char buf[64]; int32_t r; int fd;
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
    kprintf("\n  Syscall Test Suite (19 calls via int 0x80)\n");
    kprintf("  ------------------------------------------\n");

    #define PASS_FAIL(cond) do { \
        if (cond) { terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK)); kprintf("PASS\n"); } \
        else      { terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));   kprintf("FAIL\n"); } \
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK)); \
    } while(0)

    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
    kprintf("  [I/O]\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
    kprintf("  write(STDOUT)..."); r=sys_write(1,"hi\n",3); PASS_FAIL(r==3);
    kprintf("  write(STDERR)..."); r=sys_write(2,"er\n",3); PASS_FAIL(r==3);
    kprintf("  uptime()........"); r=sys_uptime(); PASS_FAIL(r>=0);
    kprintf("  getpid()........"); r=sys_getpid(); PASS_FAIL(r>=0);

    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
    kprintf("  [FILESYSTEM]\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
    kprintf("  mkdir(t1)......."); r=sys_mkdir("t1"); PASS_FAIL(r==0);
    kprintf("  rmdir(t1)......."); r=sys_rmdir("t1"); PASS_FAIL(r==0);
    kprintf("  open(O_CREAT)..."); fd=sys_open("sctest",O_CREAT|O_RDWR); PASS_FAIL(fd>=3);
    kprintf("  write(fd)......."); r=sys_write(fd,"abc",3); PASS_FAIL(r==3);
    kprintf("  lseek(SEEK_SET)."); r=sys_lseek(fd,0,SEEK_SET); PASS_FAIL(r==0);
    kprintf("  read(fd)........"); r=sys_read(fd,buf,3); buf[3]=0; PASS_FAIL(r==3);
    kprintf("  close(fd)......."); r=sys_close(fd); PASS_FAIL(r==0);
    kprintf("  stat(sctest)...."); { fs_stat_t st; r=sys_stat("sctest",&st); PASS_FAIL(r==0&&st.size==3); }
    kprintf("  unlink(sctest).."); r=sys_unlink("sctest"); PASS_FAIL(r==0);
    kprintf("  getcwd().......:"); r=sys_getcwd(buf,sizeof(buf)); kprintf(" %s ",buf); PASS_FAIL(r>0);

    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
    kprintf("  [PROCESS]\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
    kprintf("  yield()........."); r=sys_yield(); PASS_FAIL(r==0);
    kprintf("  fork() (stub)..."); r=sys_fork(); PASS_FAIL(r==-ENOSYS);
    kprintf("  execve() (stub)."); r=sys_execve("x"); PASS_FAIL(r==-ENOSYS);
    kprintf("  waitpid() (stub)"); r=sys_waitpid(0); PASS_FAIL(r==-ENOSYS);
    kprintf("  kill(invalid).."); r=sys_kill(0); PASS_FAIL(r==-EPERM);

    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
    kprintf("\n  19 syscalls exercised.\n\n");
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
    #undef PASS_FAIL
}
