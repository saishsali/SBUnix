// http://wiki.osdev.org/ELF
#include <sys/elf64.h>
#include <sys/tarfs.h>
#include <sys/page_descriptor.h>
#include <sys/paging.h>
#include <sys/process.h>
#include <sys/kprintf.h>
#include <sys/memory.h>
#include <sys/string.h>

Elf64_Ehdr *get_elf_header(char *filename) {
    struct posix_header_ustar *phu = get_file(filename);

    if (phu == NULL) {
        return NULL;
    }

    return (Elf64_Ehdr *)(phu + 1);
}

int is_elf_file(Elf64_Ehdr *elf_header)
{
    if (elf_header == NULL)
        return 0;

    // Check magic number (4 bytes): Magic number - 0x7F, then 'ELF' in ASCII
    if (elf_header->e_ident[1] == 'E' && elf_header->e_ident[2] == 'L' && elf_header->e_ident[3] == 'F')
        return 1;

    return 0;
}

uint64_t read_program_header(task_struct *pcb, Elf64_Ehdr *elf_header, Elf64_Phdr *program_header, uint64_t current_cr3) {
    uint64_t page_offset, copy_offset = 0;
    uint64_t virtual_address, vm_type;

    if (program_header->p_type != SEGMENT_LOAD) {
        return 0;
    }

    // Flags: 1 = executable, 2 = writable, 4 = readable
    if (program_header->p_flags == 5) {
        vm_type = TEXT;
    } else if (program_header->p_flags == 6) {
        vm_type = DATA;
    } else {
        vm_type = NOTYPE;
    }

    add_vma(
        pcb,
        program_header->p_vaddr,
        program_header->p_memsz,
        program_header->p_flags,
        vm_type
    );

    virtual_address = program_header->p_vaddr;

    set_cr3(pcb->cr3);
    while (virtual_address < (program_header->p_vaddr + program_header->p_memsz)) {
        kmalloc_map(PAGE_SIZE, virtual_address, program_header->p_flags | PTE_P);

        page_offset = 0;
        while (page_offset < PAGE_SIZE && copy_offset <= program_header->p_filesz) {
            *((char *)virtual_address + page_offset) = *((char *)elf_header + program_header->p_offset + copy_offset);
            page_offset++;
            copy_offset++;
        }
        virtual_address += PAGE_SIZE;
    }

    /*
        Alternative to above code:
        kmalloc_map(program_header->p_memsz, virtual_address, program_header->p_flags | PTE_P);
        memcpy((void*) virtual_address, (void*) elf_header + program_header->p_offset, program_header->p_filesz);
        memset((void *)virtual_address + program_header->p_filesz, 0, program_header->p_memsz - program_header->p_filesz);
    */
    set_cr3(current_cr3);

    return virtual_address;
}

void load_executable(task_struct *pcb, char *filename) {
    Elf64_Ehdr *elf_header = get_elf_header(filename);
    if (elf_header == NULL || is_elf_file(elf_header) == 0) {
        return;
    }

    Elf64_Phdr *program_header = (Elf64_Phdr *)((uint64_t)elf_header + elf_header->e_phoff);
    int i;
    uint64_t current_cr3 = get_cr3(), load_section_end_address, max_address = 0;

    /* Add entry point in PCB */
    pcb->entry = elf_header->e_entry;

    // Load segment data into memory
    for (i = 0; i < elf_header->e_phnum; i++) {
        load_section_end_address = read_program_header(pcb, elf_header, program_header, current_cr3);
        if (load_section_end_address > max_address) {
            max_address = load_section_end_address;
        }
        program_header++;
    }

    // Create VMA for HEAP
    add_vma(pcb, max_address, PAGE_SIZE, RW, HEAP);

    // Create VMA for STACK
    add_vma(pcb, STACK_START - STACK_SIZE, PAGE_SIZE, RW, STACK);

    set_cr3(pcb->cr3);
    kmalloc_map(PAGE_SIZE, STACK_START - PAGE_SIZE, RW_FLAG);
    set_cr3(current_cr3);

    pcb->u_rsp = STACK_START - 0x08;
}
