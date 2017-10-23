#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/mmap.h>

#define PAGE_SIZE       4096
#define PROT_READ       0x1     /* Page can be read.  */
#define PROT_WRITE      0x2     /* Page can be written.  */
#define PROT_EXEC       0x4     /* Page can be executed.  */
#define PROT_NONE       0x0     /* Page can not be accessed.  */
#define MAP_FIXED       0x10    /* Interpret addr exactly.  */
#define MAP_FILE        0
#define MAP_ANONYMOUS   0x20    /* Don't use a file.  */

struct page_descriptor {
    struct page_descriptor *next;
    struct page_descriptor *previous;
}__attribute__((packed));

void create_page_descriptor(uint32_t *modulep, void *physbase, void *physfree) {
    uint64_t num_pages = 0;
    struct smap_t {
        uint64_t base, length;
        uint32_t type;
    }__attribute__((packed)) *smap;
    while (modulep[0] != 0x9001) modulep += modulep[1] + 2;

    // uint64_t *address = NULL;
    struct page_descriptor *freelist=NULL;
    // struct page_descriptor *start_freelist=NULL;

    for(smap = (struct smap_t*)(modulep + 2); smap < (struct smap_t*)((char*)modulep+modulep[1] + 2 * 4); ++smap) {
        if (smap->type == 1 /* memory */ && smap->length != 0) {
            num_pages += smap->length/PAGE_SIZE;
        }
    }

    freelist = mmap (physfree, sizeof(struct page_descriptor) * num_pages, PROT_WRITE|PROT_READ , MAP_ANONYMOUS, 0, 0);
    kprintf("Number of pages: %d %p\n", num_pages, freelist);
}
