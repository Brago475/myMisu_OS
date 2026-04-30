#include "klog.h"
#include "string.h"
#include "timer.h"
#include "kprintf.h"
#include "vga.h"

static klog_entry_t klog_buf[KLOG_MAX_ENTRIES];
static int klog_head = 0;
static int klog_total = 0;

void klog_init(void){
    memset(klog_buf, 0, sizeof(klog_buf));
    klog_head = 0;
    klog_total = 0;
}

void klog(klog_level_t level, const char* msg){
    klog_entry_t* e = &klog_buf[klog_head];
    e->tick = timer_get_ticks();
    e->level = level;
    int i = 0;
    while(msg[i] && i < KLOG_LINE_LEN - 1){ e->msg[i] = msg[i]; i++; }
    e->msg[i] = 0;
    klog_head = (klog_head + 1) % KLOG_MAX_ENTRIES;
    klog_total++;
}

int klog_count(void){
    return klog_total < KLOG_MAX_ENTRIES ? klog_total : KLOG_MAX_ENTRIES;
}

void klog_print(void){
    int count = klog_count();
    int start = klog_total < KLOG_MAX_ENTRIES ? 0 : klog_head;

    for(int i = 0; i < count; i++){
        klog_entry_t* e = &klog_buf[(start + i) % KLOG_MAX_ENTRIES];

        terminal_setcolor(vga_entry_color(VGA_DARK_GREY,VGA_BLACK));
        kprintf("  [");
        uint32_t s = e->tick / 100;
        uint32_t cs = (e->tick % 100);
        kprintf("%d.%d", s, cs);
        kprintf("]  ");

        switch(e->level){
            case KLOG_OK:   terminal_setcolor(vga_entry_color(VGA_LIGHT_GREEN,VGA_BLACK)); kprintf("OK   "); break;
            case KLOG_INFO: terminal_setcolor(vga_entry_color(VGA_LIGHT_CYAN,VGA_BLACK));  kprintf("INFO "); break;
            case KLOG_WARN: terminal_setcolor(vga_entry_color(VGA_YELLOW,VGA_BLACK));      kprintf("WARN "); break;
            case KLOG_ERR:  terminal_setcolor(vga_entry_color(VGA_LIGHT_RED,VGA_BLACK));   kprintf("ERR  "); break;
        }

        terminal_setcolor(vga_entry_color(VGA_LIGHT_GREY,VGA_BLACK));
        kprintf("%s\n", e->msg);
    }
}
