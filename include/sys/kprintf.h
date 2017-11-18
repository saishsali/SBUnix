#ifndef __KPRINTF_H
#define __KPRINTF_H

void kprintf(const char *fmt, ...);
void kprintf_pos(int row, int column, const char *fmt, ...);
void clear_screen();

void kprintf_backspace(char buf[1024], int curr_scan);

#endif
