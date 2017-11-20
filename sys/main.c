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
    load_cr3();

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
    create_user_process("bin/ls");

    /* Check sys_mmap and page fault handler */
    // p = sys_mmap(NULL, 4097, 1);
    // char *p = sys_mmap((void *)0x1000, 100, RW_FLAG);
    // strcpy(p, "Hello");
    // kprintf("Accessible after sys_mmap and page fault: %s\n", p);

    // sys_munmap(p, 100);
    // strcpy(p, "World");
    // kprintf("%s\n", p);

    /* Init tarfs and create directory structure */
    init_tarfs();
    // create_threads();

    /* get current working directory */
    // char buf[1024];
    // getcwd(buf, 1024);
    // kprintf("\n getcwd %s", buf);

    // chdir("/../../../rootfs/bin/../etc/../");

    // getcwd(buf, 1024);
    // kprintf("\n getcwd %s", buf);

    /* Open, read and close directory */
    // DIR* dir = opendir("/rootfs/bin");
    // if(dir == NULL) {
    //     kprintf("NULL directory");
    // } else {
    //     kprintf("exists,  %s", dir->node->name);
    // }

    // dentry* curr_dentry = NULL;
    // while((curr_dentry = readdir(dir)) != NULL) {
    //     kprintf("\n name %s", curr_dentry->name);
    // }
    // closedir(dir);

    // kprintf("\n ret node %p", ret->node);
    // close();
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
