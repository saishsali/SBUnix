// http://wiki.osdev.org/ELF
#include <sys/elf64.h>
#include <sys/tarfs.h>
#include <sys/process.h>
#include <sys/page_descriptor.h>
#include <sys/paging.h>

Elf64_Ehdr *get_elf_header(char *filename) {
    struct posix_header_ustar *phu = get_file(filename);

    if (phu == NULL)
        return NULL;

    return (Elf64_Ehdr *)(phu + 1);
}

void load_executable(task_struct *pcb, char *filename) {
    Elf64_Ehdr *elf_header = get_elf_header(filename);
    Elf64_Phdr *program_header = (Elf64_Phdr *)((uint64_t)elf_header + elf_header->e_phoff);
    Page *p;
    uint64_t page_offset, copy_offset, va;
    int i;

    for (i = 0; i < elf_header->e_phnum; i++) {
        if (program_header->p_type == SEGMENT_LOAD) {
            copy_offset = 0;
            for (va = program_header->p_vaddr; va < (program_header->p_vaddr + program_header->p_memsz); va += PAGE_SIZE) {
                p = allocate_page();
                // To do: pass permissions as a parameter
                map_page(va, page_to_physical_address(p));

                for (page_offset = 0; page_offset < PAGE_SIZE && copy_offset <= program_header->p_filesz; page_offset++, copy_offset++) {
                    *((char *)va + page_offset) = *((char *)elf_header + copy_offset);
                }
            }
        }
        program_header++;
    }
}
