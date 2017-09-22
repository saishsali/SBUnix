#ifndef _IO_H
#define _IO_H
#include <sys/defs.h>

unsigned char inb (unsigned short int port);
uint32_t inl(uint16_t port);
void outl (uint16_t port, uint32_t data);

#endif
