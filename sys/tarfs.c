#include <sys/tarfs.h>
#include <sys/utils.h>
#include <sys/defs.h>
#include <sys/string.h>
#include <sys/kprintf.h>

struct posix_header_ustar *get_file(char *filename) {
    char *p = &_binary_tarfs_start;
    int size;
    struct posix_header_ustar *phu;

    while (p < &_binary_tarfs_end) {
        phu = (struct posix_header_ustar*)p;
        if (strcmp(phu->name, filename) == 0) {
            return phu;
        }

        size = oct_to_dec(atoi(phu->size));
        p += sizeof(struct posix_header_ustar);
        if (size == 0) {
            continue;
        }

        // 512 byte aligned address
        p += ROUND_UP(size, BLOCK_SIZE);
    }

    return NULL;
}
