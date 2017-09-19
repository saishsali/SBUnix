#include <sys/kprintf.h>
#include <sys/io.h>
#define ROW 24
#define COLUMN 77
#define CONTROL_SC 29
#define LEFT_SHIFT_SC 0x2A
#define RIGHT_SHIFT_SC 0x36
#define SIZE 0x80

// Scancode to ASCII mapping
unsigned char scancode_ascii[SIZE] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
    '9', '0', '-', '=', '\b',   /* Backspace */
    '\t',           /* Tab */
    'q', 'w', 'e', 'r', /* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',   /* Enter key */
    0,          /* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',   /* 39 */
    '\'', '`',   0,     /* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',         /* 49 */
    'm', ',', '.', '/',   0,                /* Right shift */
    '*',
    0,  /* Alt */
    ' ',    /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
    '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
    '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

// Scancode to ASCII mapping for shift keys
unsigned char scancode_ascii_shift[SIZE] =
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', /* 9 */
    '(', ')', '_', '+', '\b',   /* Backspace */
    '\t',           /* Tab */
    'Q', 'W', 'E', 'R', /* 19 */
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',   /* Enter key */
    0,          /* 29   - Control */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',   /* 39 */
    '"', '~',   0,     /* Left shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'n',         /* 49 */
    'M', '<', '>', '?',   0,                /* Right shift */
    '*',
    0,  /* Alt */
    ' ',    /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
    '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
    '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};

int control = 0, shift = 0;

int shift_key(int scancode) {
    if (scancode == LEFT_SHIFT_SC || scancode == RIGHT_SHIFT_SC) {
        shift = 1;
        return 1;
    }

    return 0;
}

int control_key(int scancode) {
    if (scancode == CONTROL_SC) {
        control = 1;
        kprintf_pos(ROW, COLUMN, "^");
        return 1;
    }

    return 0;
}

void keyboard_interrupt() {
    int scancode = inb(0x60), y = COLUMN;
    char c = 0;

    if (scancode < SIZE) {
        if (shift_key(scancode)) {
            return;
        } else if (control_key(scancode)) {
            return;
        }

        if (shift == 1) {
            c = scancode_ascii_shift[scancode];
            shift = 0;
        } else {
            if (control == 1) {
                y++;
                control = -1;
            } else if (control == -1) {
                kprintf_pos(ROW, COLUMN + 1, " ");
            }
            c = scancode_ascii[scancode];
        }

        kprintf_pos(ROW, y, "%c", c);
    }
}
