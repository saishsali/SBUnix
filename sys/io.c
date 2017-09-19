unsigned char _x86_64_inb(unsigned short int port);

unsigned char inb (unsigned short int port) {
    unsigned char ch = _x86_64_inb(port);
    return ch;
}
