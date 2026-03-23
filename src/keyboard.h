#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

/* Special key codes (non-ASCII, 0x80+) */
#define KEY_UP    1
#define KEY_DOWN  2
#define KEY_LEFT  3
#define KEY_RIGHT 4
#define KEY_ESC   0x1B
#define KEY_PGUP  5
#define KEY_PGDN  6
void keyboard_init(void);
char keyboard_getchar(void);
bool keyboard_haschar(void);

#endif
