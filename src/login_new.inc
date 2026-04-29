void login_screen(void){
    terminal_clear();
    kprintf("\n\n");

    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
    kprintf("    +============================================================+\n");
    kprintf("    |                                                            |\n");
    kprintf("    |                       |\\---/|                              |\n");
    kprintf("    |                       | ,_, |                              |\n");
    kprintf("    |                        \\_`_/-..--.                         |\n");
    kprintf("    |                     ___/ `   ' ,+ \\                        |\n");
    kprintf("    |                    (__...'  _/  |`._;                      |\n");
    kprintf("    |                      (_,..'(_,.`__)                        |\n");
    kprintf("    |                                                            |\n");
    kprintf("    |                        MyMisu OS                           |\n");
    kprintf("    |              a bare-metal x86 operating system             |\n");
    kprintf("    |                                                            |\n");
    terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
    kprintf("    |                          %s                            |\n", MYMISU_VERSION);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
    kprintf("    +============================================================+\n");

    kprintf("\n");
    int attempts = 0;
    while(1){
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
        kprintf("                       Username: ");
        terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
        char user[32]; int ui=0;
        while(1){
            char c=keyboard_getchar();
            if(c=='\n'){terminal_putchar('\n');user[ui]='\0';break;}
            else if(c=='\b'){if(ui>0){ui--;terminal_putchar('\b');}}
            else if(ui<31&&c>=32&&c<127){user[ui++]=c;terminal_putchar(c);}
        }
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
        kprintf("                       Password: ");
        terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
        char pass[32]; int pi=0;
        while(1){
            char c=keyboard_getchar();
            if(c=='\n'){terminal_putchar('\n');pass[pi]='\0';break;}
            else if(c=='\b'){if(pi>0){pi--;terminal_putchar('\b');}}
            else if(pi<31&&c>=32&&c<127){pass[pi++]=c;terminal_putchar('*');}
        }
        if((strcmp(user,"misu")==0&&strcmp(pass,"misu")==0)||(strcmp(user,"admin")==0&&strcmp(pass,"admin")==0)||(strcmp(user,"james")==0&&strcmp(pass,"1234")==0)){
            terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));
            kprintf("\n                       Authenticated. Welcome, %s.\n",user);
            uint32_t w=timer_get_ticks(); while(timer_get_ticks()-w<150) asm volatile("hlt");
            terminal_clear();
            return;
        } else {
            attempts++;
            terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));
            kprintf("\n                       Invalid credentials (attempt %d).\n", attempts);
            if(attempts >= 3){
                terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
                kprintf("                       Hint: misu/misu or admin/admin\n");
            }
            kprintf("\n");
        }
    }
}
