#include <sys/defs.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/pic.h>
#include <sys/kprintf.h>
#include <sys/tarfs.h>
#include <sys/ahci.h>
#include <sys/page_descriptor.h>
#include <sys/ahci.h>
#include <sys/paging.h>
#include <sys/memory.h>
#include <sys/process.h>
#include <sys/tarfs.h>
#include <sys/elf64.h>
#include <sys/syscall.h>
#include <sys/string.h>

#define INITIAL_STACK_SIZE 4096
uint8_t initial_stack[INITIAL_STACK_SIZE]__attribute__((aligned(16)));
uint32_t* loader_stack;
extern char kernmem, physbase;

void start(uint32_t *modulep, void *physbase, void *physfree) {
    uint64_t last_physical_address = 0;
    clear_screen();
    struct smap_t {
        uint64_t base, length;
        uint32_t type;
    }__attribute__((packed)) *smap;
    while (modulep[0] != 0x9001) modulep += modulep[1] + 2;
    for (smap = (struct smap_t*)(modulep + 2); smap < (struct smap_t*)((char*)modulep+modulep[1] + 2 * 4); ++smap) {
        if (smap->type == 1 /* memory */ && smap->length != 0) {
            kprintf("Available Physical Memory [%p-%p]\n", smap->base, smap->base + smap->length);
            page_init(smap->base, (smap->base + smap->length), (uint64_t)physbase, (uint64_t)physfree);
            last_physical_address = smap->base + smap->length;
        }
    }

    kprintf("physbase %p\n", (uint64_t)physbase);
    kprintf("physfree %p\n", (uint64_t)physfree);
    kprintf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);

    /* Setup Paging and load CR3 */
    setup_page_tables((uint64_t)physbase, (uint64_t)physfree, last_physical_address);

    /* Init tarfs and create directory structure */
    init_tarfs();

    /* Free initial pages (0 to physbase) used by the bootloader */
    // deallocate_initial_pages((uint64_t)physbase);

    /* Test kmalloc */
    // char *temp = (char *)kmalloc(20);
    // temp[0] = 'a';
    // temp[1] = '\0';
    // kprintf("Allocation works %s\n", temp);

    /* Create threads and switch to ring 3 */
    // create_threads();

    /* AHCI controller */
    // init_pci();

    /* Create user process and load its executable*/
    // create_user_process("bin/ls");
    create_idle_process();

    /* Create user process, load its executable and switch to ring 3 */
    task_struct *pcb = create_user_process("bin/sbush");

    switch_to_user_mode(pcb);
}

void boot(void) {
    // note: function changes rsp, local stack variables can't be practically used
    register char *temp1;
    for(temp1 = (char*)0xb8001; temp1 < (char*)0xb8000+160*25; temp1 += 2) *temp1 = 7 /* white */;
    __asm__(
        "cli;"
        "movq %%rsp, %0;"
        "movq %1, %%rsp;"
        :"=g"(loader_stack)
        :"r"(&initial_stack[INITIAL_STACK_SIZE])
    );
    init_gdt();
    init_idt();
    init_pic();

    start(
        (uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
        (uint64_t*)&physbase,
        (uint64_t*)(uint64_t)loader_stack[4]
    );

    while(1) __asm__ volatile ("hlt");
}
