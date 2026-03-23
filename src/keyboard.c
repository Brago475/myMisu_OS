#include "keyboard.h"
#include "idt.h"
#include "ports.h"

#define KB_BUFFER_SIZE 256

static char kb_buffer[KB_BUFFER_SIZE];
static volatile uint16_t kb_read = 0;
static volatile uint16_t kb_write = 0;
static bool shift_pressed = false;
static bool caps_lock = false;

/* US QWERTY scancode to ASCII (Set 1) */
static const char scancode_normal[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,  'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,  '\\','z','x','c','v','b','n','m',',','.','/',0,
    '*', 0, ' '
};

static const char scancode_shift[128] = {
    0,  27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,  'A','S','D','F','G','H','J','K','L',':','"','~',
    0,  '|','Z','X','C','V','B','N','M','<','>','?',0,
    '*', 0, ' '
};

static void keyboard_callback(registers_t* regs) {
    (void) regs;
    uint8_t scancode = inb(0x60);

    /* Key release (high bit set) */
    if (scancode & 0x80) {
        uint8_t released = scancode & 0x7F;
        if (released == 0x2A || released == 0x36) shift_pressed = false;
        return;
    }

    /* Special keys */
    if (scancode == 0x2A || scancode == 0x36) { shift_pressed = true; return; }
    if (scancode == 0x3A) { caps_lock = !caps_lock; return; }

    /* Convert scancode to ASCII */
    char c = 0;
    if (scancode < 128) {
        bool use_shift = shift_pressed;
        /* Caps lock only affects letters */
        if (caps_lock && scancode_normal[scancode] >= 'a' && scancode_normal[scancode] <= 'z')
            use_shift = !use_shift;

        c = use_shift ? scancode_shift[scancode] : scancode_normal[scancode];
    }

    if (c == 0) return;

    /* Push into ring buffer */
    uint16_t next = (kb_write + 1) % KB_BUFFER_SIZE;
    if (next != kb_read) {
        kb_buffer[kb_write] = c;
        kb_write = next;
    }
}

void keyboard_init(void) {
    register_interrupt_handler(33, keyboard_callback);
}

bool keyboard_haschar(void) {
    return kb_read != kb_write;
}

char keyboard_getchar(void) {
    while (kb_read == kb_write) {
        asm volatile("hlt");  /* Wait for interrupt */
    }
    char c = kb_buffer[kb_read];
    kb_read = (kb_read + 1) % KB_BUFFER_SIZE;
    return c;
}
