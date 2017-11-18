#include <sys/kprintf.h>
#include <sys/io.h>
#include <sys/keyboard.h>
#include <sys/memcpy.h>

#define ROW 24
#define COLUMN 55
#define CONTROL_SC 29
#define LEFT_SHIFT_SC 0x2A
#define RIGHT_SHIFT_SC 0x36
#define SIZE 0x80

char output_buf[1024];
static volatile int scan_flag = 0;
static volatile int scan_len = 0;
static volatile int max_scan_len = 0;

// Scancode to ASCII mapping (src: https://gist.github.com/davazp/d2fde634503b2a5bc664)
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
    '|', 'Z', 'X', 'C', 'V', 'B', 'N',         /* 49 */
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
        return 1;
    }

    return 0;
}

void keyboard_interrupt() {
    int scancode = inb(0x60);
    char ch1 = 0, ch2 = 0;
    int flag = 0;

    // If a key is pressed
    if (scancode < SIZE) {

        output_buf[scan_len] = scancode_ascii[scancode];

        if(scancode == BACKSPACE) {
            if(scan_len > 0) {
                output_buf[scan_len - 1] = '\0';
                kprintf_backspace(output_buf, scan_len);
                scan_len--;
            }

        } else {
            kprintf("%c", output_buf[scan_len]);
            flag = 1;
        }

        if(scancode == ENTER) {
            scan_flag = 0;

            // mark the enter scancode as null
            output_buf[scan_len] = '\0';
            if(max_scan_len < scan_len + 1) {
                max_scan_len = scan_len + 1;
            }
        } else if (shift_key(scancode)) { // If a shift key is pressed
            return;
        } else if (control_key(scancode)) { // If a control key is pressed
            return;
        } else if (shift == 1) { // If a shift key was pressed
            ch1 = scancode_ascii_shift[scancode];
            shift = 0;
        } else if (control == 1) { // If a control key was pressed
            ch1 = '^';
            ch2 = scancode_ascii_shift[scancode];
            control = 0;
        } else {
            ch1 = scancode_ascii[scancode];
        }

        kprintf_pos(ROW, COLUMN, "Last pressed glyph: %c%c", ch1, ch2);
    }
    if(flag)
        scan_len++;
}

int scanf(void *buff, int len) {
    scan_flag = 1;
    __asm__ __volatile__("sti;");

    while (scan_flag == 1);

    if (len <  scan_len)
        scan_len = len;

    memcpy((void *)buff, (void *)output_buf, scan_len);

    int temp = max_scan_len;
    while(temp >= 0) {
        output_buf[temp--] = '\0';
    }

    temp = scan_len;
    scan_len = 0;
    return temp;
}