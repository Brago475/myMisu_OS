#!/bin/bash
cd ~/mymisu_os

python3 << 'PYEOF'
import re

with open("src/shell.c", "r") as f:
    code = f.read()

games = '''
static void cmd_tictactoe(void) {
    char board[9];
    for(int i=0;i<9;i++) board[i]=' ';
    int turn=0;
    int w[8][3]={{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
    while(1) {
        terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
        kprintf("\\n  Tic-Tac-Toe | You=X  CPU=O\\n\\n");
        for(int r=0;r<3;r++) {
            kprintf("       ");
            for(int c=0;c<3;c++) {
                char ch=board[r*3+c];
                if(ch=='X') terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));
                else if(ch=='O') terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));
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
        if(winner=='X'){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("  You win!\\n\\n");return;}
        if(winner=='O'){terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  CPU wins!\\n\\n");return;}
        int full=1;for(int i=0;i<9;i++)if(board[i]==' ')full=0;
        if(full){terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));kprintf("  Draw!\\n\\n");return;}
        if(turn==0) {
            terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
            kprintf("  Pick 1-9: ");
            char c=keyboard_getchar();
            terminal_putchar(c);terminal_putchar('\\n');
            int pos=c-'1';
            if(pos>=0&&pos<9&&board[pos]==' '){board[pos]='X';turn=1;}
        } else {
            int moved=0;
            for(int p=0;p<2&&!moved;p++){
                char ch=(p==0)?'O':'X';
                for(int i=0;i<8;i++){
                    int cnt=0,empty=-1;
                    for(int j=0;j<3;j++){
                        if(board[w[i][j]]==ch)cnt++;
                        else if(board[w[i][j]]==' ')empty=w[i][j];
                    }
                    if(cnt==2&&empty>=0){board[empty]='O';moved=1;break;}
                }
            }
            if(!moved&&board[4]==' '){board[4]='O';moved=1;}
            if(!moved){int corners[]={0,2,6,8};for(int i=0;i<4&&!moved;i++)if(board[corners[i]]==' '){board[corners[i]]='O';moved=1;}}
            if(!moved){for(int i=0;i<9&&!moved;i++)if(board[i]==' '){board[i]='O';moved=1;}}
            turn=0;
        }
    }
}

static void cmd_hangman(void) {
    const char* words[]={"kernel","memory","process","keyboard","interrupt","system","driver","shell","timer","stack"};
    const char* word=words[timer_get_ticks()%10];
    int wlen=strlen(word);
    char guessed[26];int nguessed=0;
    int lives=6;
    while(lives>0){
        terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
        kprintf("\\n  HANGMAN | Lives: ");
        terminal_setcolor(vga_entry_color(lives>2?VGA_LIGHT_GREEN:VGA_LIGHT_RED,VGA_BLACK));
        for(int i=0;i<lives;i++)kprintf("<3 ");
        kprintf("\\n\\n  Word: ");
        int done=1;
        for(int i=0;i<wlen;i++){
            int found=0;
            for(int j=0;j<nguessed;j++)if(guessed[j]==word[i])found=1;
            if(found){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("%c ",word[i]);}
            else{terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));kprintf("_ ");done=0;}
        }
        kprintf("\\n");
        if(done){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("\\n  You won! Word: %s\\n\\n",word);return;}
        terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
        kprintf("  Used: ");for(int i=0;i<nguessed;i++)kprintf("%c ",guessed[i]);
        terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));
        kprintf("\\n\\n  Guess: ");
        char c=keyboard_getchar();terminal_putchar(c);terminal_putchar('\\n');
        if(c<'a'||c>'z')continue;
        int already=0;for(int i=0;i<nguessed;i++)if(guessed[i]==c)already=1;
        if(already){kprintf("  Already guessed!\\n");continue;}
        guessed[nguessed++]=c;
        int inwrd=0;for(int i=0;i<wlen;i++)if(word[i]==c)inwrd=1;
        if(!inwrd){lives--;terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  Wrong!\\n");}
        else{terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("  Correct!\\n");}
    }
    terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));
    kprintf("\\n  Dead! Word: %s\\n\\n",word);
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
}

static void cmd_rps(void) {
    terminal_setcolor(vga_entry_color(VGA_WHITE,VGA_BLACK));
    kprintf("\\n  Rock Paper Scissors! Best of 5.\\n\\n");
    int pw=0,cw=0,round=1;
    while(round<=5&&pw<3&&cw<3){
        terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));
        kprintf("  Round %d - (r)ock (p)aper (s)cissors: ",round);
        char c=keyboard_getchar();terminal_putchar(c);terminal_putchar('\\n');
        int player=-1;
        if(c=='r')player=0;else if(c=='p')player=1;else if(c=='s')player=2;
        else{kprintf("  Use r/p/s\\n");continue;}
        int cpu=(timer_get_ticks()+round*7)%3;
        const char* names[]={"Rock","Paper","Scissors"};
        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
        kprintf("\\n  You: %s vs CPU: %s\\n",names[player],names[cpu]);
        if(player==cpu){terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));kprintf("  Draw!\\n\\n");}
        else if((player==0&&cpu==2)||(player==1&&cpu==0)||(player==2&&cpu==1)){
            terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("  You win!\\n\\n");pw++;}
        else{terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  CPU wins!\\n\\n");cw++;}
        terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
        kprintf("  Score: You %d - %d CPU\\n\\n",pw,cw);round++;
    }
    if(pw>cw){terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK));kprintf("  You win!\\n\\n");}
    else if(cw>pw){terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));kprintf("  CPU wins!\\n\\n");}
    else{terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));kprintf("  Draw!\\n\\n");}
    terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
}
'''

# Insert games before execute_command
code = code.replace('static void execute_command', games + '\nstatic void execute_command')

# Add dispatchers
code = code.replace(
    'else if(strcmp(argv[0],"snake")==0)cmd_snake();',
    'else if(strcmp(argv[0],"snake")==0)cmd_snake();\n    else if(strcmp(argv[0],"ttt")==0)cmd_tictactoe();\n    else if(strcmp(argv[0],"hangman")==0)cmd_hangman();\n    else if(strcmp(argv[0],"rps")==0)cmd_rps();'
)

# Add help entries
code = code.replace(
    'kprintf("  snake     ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Snake game!\\n");',
    'kprintf("  snake     ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Snake game!\\n");\n    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  ttt       ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Tic-Tac-Toe\\n");\n    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  hangman   ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Guess the word\\n");\n    terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));kprintf("  rps       ");terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));kprintf("Rock Paper Scissors\\n");'
)

with open("src/shell.c", "w") as f:
    f.write(code)

print("Games patched in!")
PYEOF

echo ""
echo "New commands:"
echo "  ttt      - Tic-Tac-Toe vs CPU"
echo "  hangman  - Guess the word (OS-themed words)"
echo "  rps      - Rock Paper Scissors best of 5"
echo ""
echo "Run: make clean && make && make run"
