#!/bin/bash
cd ~/mymisu_os

# First add PGUP/PGDN to keyboard.h
python3 << 'PYEOF'
with open("src/keyboard.h", "r") as f:
    code = f.read()

if "KEY_PGUP" not in code:
    code = code.replace(
        "#define KEY_ESC   0x1B",
        "#define KEY_ESC   0x1B\n#define KEY_PGUP  5\n#define KEY_PGDN  6"
    )
    with open("src/keyboard.h", "w") as f:
        f.write(code)
    print("keyboard.h updated")
else:
    print("keyboard.h already has PGUP/PGDN")
PYEOF

# Add PGUP/PGDN scancodes to keyboard.c
python3 << 'PYEOF'
with open("src/keyboard.c", "r") as f:
    code = f.read()

if "KEY_PGUP" not in code:
    code = code.replace(
        'if (scancode == 0x01) { kb_push(KEY_ESC); return; }',
        'if (scancode == 0x01) { kb_push(KEY_ESC); return; }\n    if (scancode == 0x49) { kb_push(KEY_PGUP); return; }\n    if (scancode == 0x51) { kb_push(KEY_PGDN); return; }'
    )
    with open("src/keyboard.c", "w") as f:
        f.write(code)
    print("keyboard.c updated")
else:
    print("keyboard.c already has PGUP/PGDN")
PYEOF

# Add scrollback to vga.h
python3 << 'PYEOF'
with open("src/vga.h", "r") as f:
    code = f.read()

if "terminal_scroll_back" not in code:
    code = code.replace(
        "void terminal_update_cursor(int x, int y);",
        "void terminal_update_cursor(int x, int y);\nvoid terminal_scroll_back(void);\nvoid terminal_scroll_forward(void);\nvoid terminal_scroll_reset(void);"
    )
    with open("src/vga.h", "w") as f:
        f.write(code)
    print("vga.h updated")
else:
    print("vga.h already has scrollback")
PYEOF

# Add scrollback buffer to vga.c
python3 << 'PYEOF'
with open("src/vga.c", "r") as f:
    code = f.read()

if "scrollback_buf" not in code:
    # Add scrollback buffer vars after existing statics
    code = code.replace(
        "static uint16_t* terminal_buffer;",
        "static uint16_t* terminal_buffer;\n\n#define SCROLLBACK_LINES 200\nstatic uint16_t scrollback_buf[SCROLLBACK_LINES * 80];\nstatic int scrollback_count = 0;\nstatic int scroll_offset = 0;\nstatic int is_scrolled = 0;"
    )

    # Save line to scrollback before it scrolls off in terminal_scroll
    code = code.replace(
        "static void terminal_scroll(void) {\n    /* Move every row up by one */\n    for (size_t y = 1; y < VGA_HEIGHT; y++) {",
        "static void terminal_scroll(void) {\n    /* Save top line to scrollback */\n    if (scrollback_count < SCROLLBACK_LINES) {\n        for (size_t x = 0; x < VGA_WIDTH; x++)\n            scrollback_buf[scrollback_count * 80 + x] = terminal_buffer[x];\n        scrollback_count++;\n    } else {\n        for (int i = 1; i < SCROLLBACK_LINES; i++)\n            for (size_t x = 0; x < 80; x++)\n                scrollback_buf[(i-1)*80+x] = scrollback_buf[i*80+x];\n        for (size_t x = 0; x < VGA_WIDTH; x++)\n            scrollback_buf[(SCROLLBACK_LINES-1)*80+x] = terminal_buffer[x];\n    }\n    /* Move every row up by one */\n    for (size_t y = 1; y < VGA_HEIGHT; y++) {"
    )

    # Add scroll functions at the end before the last closing of update_cursor
    scroll_funcs = '''
void terminal_scroll_back(void) {
    if (scroll_offset < scrollback_count) {
        scroll_offset++;
        is_scrolled = 1;
        /* Redraw screen from scrollback */
        int start = scrollback_count - scroll_offset;
        for (size_t y = 0; y < VGA_HEIGHT; y++) {
            int line = start + (int)y;
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                size_t idx = y * VGA_WIDTH + x;
                if (line >= 0 && line < scrollback_count)
                    terminal_buffer[idx] = scrollback_buf[line * 80 + x];
                else
                    terminal_buffer[idx] = vga_entry(' ', terminal_color);
            }
        }
    }
}

void terminal_scroll_forward(void) {
    if (scroll_offset > 0) {
        scroll_offset--;
        if (scroll_offset == 0) {
            is_scrolled = 0;
            terminal_clear();
            return;
        }
        int start = scrollback_count - scroll_offset;
        for (size_t y = 0; y < VGA_HEIGHT; y++) {
            int line = start + (int)y;
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                size_t idx = y * VGA_WIDTH + x;
                if (line >= 0 && line < scrollback_count)
                    terminal_buffer[idx] = scrollback_buf[line * 80 + x];
                else
                    terminal_buffer[idx] = vga_entry(' ', terminal_color);
            }
        }
    }
}

void terminal_scroll_reset(void) {
    scroll_offset = 0;
    is_scrolled = 0;
}
'''
    code = code + scroll_funcs
    with open("src/vga.c", "w") as f:
        f.write(code)
    print("vga.c updated with scrollback")
else:
    print("vga.c already has scrollback")
PYEOF

# Update shell.c - add PGUP/PGDN handling + 2p ttt + pong
python3 << 'PYEOF'
with open("src/shell.c", "r") as f:
    code = f.read()

# Add PGUP/PGDN handling in shell_run input loop
if "KEY_PGUP" not in code:
    code = code.replace(
        "else if(c==KEY_DOWN){",
        "else if(c==KEY_PGUP){terminal_scroll_back();}\n            else if(c==KEY_PGDN){terminal_scroll_forward();}\n            else if(c==KEY_DOWN){"
    )

# Add 2-player ttt
if "cmd_ttt2" not in code:
    ttt2 = '''
static void cmd_ttt2(void) {
    char board[9];
    for(int i=0;i<9;i++) board[i]=' ';
    int turn=0;
    int w[8][3]={{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
    while(1) {
        terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
        kprintf("\\n  Tic-Tac-Toe 2P | P1=X  P2=O\\n\\n");
        for(int r=0;r<3;r++) {
            kprintf("       ");
            for(int c=0;c<3;c++) {
                char ch=board[r*3+c];
                if(ch=='X') terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));
                else if(ch=='O') terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
                else {terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));ch='1'+r*3+c;}
                kprintf(" %c ",ch);
                terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
                if(c<2) kprintf("|");
            }
            kprintf("\\n");
            if(r<2) kprintf("       ---+---+---\\n");
        }
        kprintf("\\n");
        char winner=0;
        for(int i=0;i<8;i++){
            if(board[w[i][0]]!=' '&&board[w[i][0]]==board[w[i][1]]&&board[w[i][1]]==board[w[i][2]])
                winner=board[w[i][0]];
        }
        if(winner=='X'){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("  Player 1 wins!\\n\\n");return;}
        if(winner=='O'){terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  Player 2 wins!\\n\\n");return;}
        int full=1;for(int i=0;i<9;i++)if(board[i]==' ')full=0;
        if(full){terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));kprintf("  Draw!\\n\\n");return;}
        terminal_setcolor(vga_entry_color(turn==0?VGA_LIGHT_GREEN:VGA_LIGHT_CYAN,VGA_BLACK));
        kprintf("  Player %d pick 1-9: ",turn+1);
        char c=keyboard_getchar();terminal_putchar(c);terminal_putchar('\\n');
        int pos=c-'1';
        if(pos>=0&&pos<9&&board[pos]==' '){board[pos]=(turn==0)?'X':'O';turn=1-turn;}
    }
}

static void cmd_pong(void) {
    int pw=40,ph=20;
    int p1y=ph/2-2,p2y=ph/2-2;
    int bx=pw/2,by=ph/2,bdx=1,bdy=1;
    int s1=0,s2=0;
    int paddle_h=4;
    while(1){
        terminal_clear();
        terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
        kprintf("  PONG | P1(W/S): %d  P2(I/K): %d | Q=quit\\n\\n",s1,s2);
        for(int y=0;y<ph;y++){
            kprintf("  ");
            for(int x=0;x<pw;x++){
                if(y==0||y==ph-1){terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));kprintf("-");}
                else if(x==1&&y>=p1y&&y<p1y+paddle_h){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("|");}
                else if(x==pw-2&&y>=p2y&&y<p2y+paddle_h){terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("|");}
                else if(x==bx&&y==by){terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("O");}
                else if(x==pw/2){terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));kprintf(":");}
                else kprintf(" ");
            }
            kprintf("\\n");
        }
        uint32_t start=timer_get_ticks();
        while(timer_get_ticks()-start<8){
            if(keyboard_haschar()){
                char c=keyboard_getchar();
                if(c=='w'&&p1y>1)p1y--;
                else if(c=='s'&&p1y<ph-1-paddle_h)p1y++;
                else if(c=='i'&&p2y>1)p2y--;
                else if(c=='k'&&p2y<ph-1-paddle_h)p2y++;
                else if(c=='q'){terminal_clear();terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));kprintf("\\n  Final: P1 %d - %d P2\\n\\n",s1,s2);terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));return;}
            }
            asm volatile("hlt");
        }
        bx+=bdx;by+=bdy;
        if(by<=1||by>=ph-2)bdy=-bdy;
        if(bx==2&&by>=p1y&&by<p1y+paddle_h)bdx=1;
        if(bx==pw-3&&by>=p2y&&by<p2y+paddle_h)bdx=-1;
        if(bx<=0){s2++;bx=pw/2;by=ph/2;bdx=1;}
        if(bx>=pw-1){s1++;bx=pw/2;by=ph/2;bdx=-1;}
        if(s1>=5||s2>=5){
            terminal_clear();
            terminal_setcolor(vga_entry_color(s1>s2?VGA_LIGHT_GREEN:VGA_LIGHT_CYAN,VGA_BLACK));
            kprintf("\\n  %s wins! %d-%d\\n\\n",s1>s2?"Player 1":"Player 2",s1,s2);
            terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));return;
        }
    }
}
'''
    code = code.replace('static void execute_command', ttt2 + '\nstatic void execute_command')

    # Add dispatchers
    if '"ttt2"' not in code:
        code = code.replace(
            'else if(strcmp(argv[0],"rps")==0)cmd_rps();',
            'else if(strcmp(argv[0],"rps")==0)cmd_rps();\n    else if(strcmp(argv[0],"ttt2")==0)cmd_ttt2();\n    else if(strcmp(argv[0],"pong")==0)cmd_pong();'
        )

    # Add help entries
    if '"ttt2"' not in code or 'Tic-Tac-Toe 2P' not in code:
        code = code.replace(
            'kprintf("  rps       ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Rock Paper Scissors\\n");',
            'kprintf("  rps       ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Rock Paper Scissors\\n");\n    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  ttt2      ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Tic-Tac-Toe 2 players\\n");\n    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  pong      ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Pong 2P (W/S vs I/K)\\n");'
        )

with open("src/shell.c", "w") as f:
    f.write(code)
print("shell.c updated with scrollback + games")
PYEOF

echo ""
echo "========================================="
echo "  Features added!"
echo "========================================="
echo ""
echo "  Scrollback: Page Up / Page Down"
echo "  ttt2  - Tic-Tac-Toe 2 players"
echo "  pong  - Pong! P1=W/S  P2=I/K  Q=quit"
echo ""
echo "Run: make clean && make && make run"
