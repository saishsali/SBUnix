unsigned char _x86_64_asm_inb(unsigned short int port);

unsigned char inb (unsigned short int port) {
    unsigned char ch = _x86_64_asm_inb(port);
    return ch;
}
