unsigned char inb (unsigned short int port) {
	unsigned char val;
	__asm__ __volatile__(
		"inb %1, %0;"
		: "=r" (val)
		: "Nd" (port)
	);
	return  val;
}