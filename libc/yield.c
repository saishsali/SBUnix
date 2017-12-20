void yield() {
    __asm__ __volatile__(
       "movq $2, %%rax;"
       "int $0x80;"
       : : :
   );
}