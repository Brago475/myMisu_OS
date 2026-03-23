#include "vga.h"
#include "keyboard.h"

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;
static int pager_enabled = 0;
static int pager_lines = 0;

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_buffer = (uint16_t*) VGA_MEMORY;
    for (size_t y = 0; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    terminal_update_cursor(0, 0);
}

void terminal_clear(void) {
    terminal_row = 0;
    terminal_column = 0;
    pager_lines = 0;
    for (size_t y = 0; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    terminal_update_cursor(0, 0);
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

static void terminal_scroll(void) {
    for (size_t y = 1; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            terminal_buffer[(y-1) * VGA_WIDTH + x] = terminal_buffer[y * VGA_WIDTH + x];
    for (size_t x = 0; x < VGA_WIDTH; x++)
        terminal_buffer[(VGA_HEIGHT-1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        terminal_row++;
        if (pager_enabled) {
            pager_lines++;
            if (pager_lines >= VGA_HEIGHT - 2) {
                /* Show pager prompt */
                uint8_t saved = terminal_color;
                terminal_color = vga_entry_color(VGA_BLACK, VGA_LIGHT_GREY);
                const char* msg = " -- Press any key for more, Q to stop -- ";
                if (terminal_row >= VGA_HEIGHT) {
                    terminal_scroll();
                    terminal_row = VGA_HEIGHT - 1;
                }
                size_t idx = terminal_row * VGA_WIDTH + terminal_column;
                for (int i = 0; msg[i]; i++)
                    terminal_buffer[idx + i] = vga_entry(msg[i], terminal_color);
                terminal_color = saved;
                /* Wait for key */
                char k = keyboard_getchar();
                /* Clear the pager line */
                for (size_t x = 0; x < VGA_WIDTH; x++)
                    terminal_buffer[terminal_row * VGA_WIDTH + x] = vga_entry(' ', saved);
                terminal_column = 0;
                pager_lines = 0;
                if (k == 'q' || k == 'Q') {
                    pager_enabled = 0;
                    return;
                }
            }
        }
    } else if (c == '\r') {
        terminal_column = 0;
    } else if (c == '\t') {
        terminal_column = (terminal_column + 4) & ~3;
        if (terminal_column >= VGA_WIDTH) { terminal_column = 0; terminal_row++; }
    } else if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
            terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] = vga_entry(' ', terminal_color);
        }
    } else {
        terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] = vga_entry(c, terminal_color);
        terminal_column++;
        if (terminal_column >= VGA_WIDTH) { terminal_column = 0; terminal_row++; }
    }
    if (terminal_row >= VGA_HEIGHT) {
        terminal_scroll();
        terminal_row = VGA_HEIGHT - 1;
    }
    terminal_update_cursor(terminal_column, terminal_row);
}

void terminal_putchar_color(char c, uint8_t color) {
    uint8_t saved = terminal_color;
    terminal_color = color;
    terminal_putchar(c);
    terminal_color = saved;
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* str) {
    size_t i = 0;
    while (str[i] != '\0') { terminal_putchar(str[i]); i++; }
}

void terminal_update_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;
    outb(0x3D4, 14);
    outb(0x3D5, pos >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, pos & 0xFF);
}

int terminal_get_row(void) {
    return (int)terminal_row;
}

void terminal_enable_pager(void) {
    pager_enabled = 1;
    pager_lines = 0;
}

void terminal_disable_pager(void) {
    pager_enabled = 0;
    pager_lines = 0;
}
