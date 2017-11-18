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

void read_program_header(task_struct *pcb, Elf64_Ehdr *elf_header, Elf64_Phdr *program_header) {
    uint64_t page_offset, copy_offset = 0, virtual_address;

    if (program_header->p_type != SEGMENT_LOAD) {
        return;
    }

    vma_struct *vma = kmalloc(sizeof(vma_struct));

    if (pcb->mm->head == NULL) {
        pcb->mm->head = vma;
    } else {
        pcb->mm->tail->next = vma;
    }

    pcb->mm->tail = vma;
    vma->mm = pcb->mm;

    vma->start = program_header->p_vaddr;
    vma->end = program_header->p_vaddr + program_header->p_memsz;
    vma->flags = program_header->p_flags;
    vma->next = NULL;
    vma->type = NOTYPE;

    pcb->mm->start_code = program_header->p_vaddr;
    pcb->mm->end_code = program_header->p_vaddr + program_header->p_memsz;
    vma->file = (file *)kmalloc(sizeof(file));
    vma->file->start = (uint64_t)elf_header;
    vma->file->offset = program_header->p_offset;
    vma->file->size = program_header->p_filesz;

    virtual_address = program_header->p_vaddr;
    while (virtual_address < (program_header->p_vaddr + program_header->p_memsz)) {
        // To do: pass permissions as a parameter
        kmalloc_map(PAGE_SIZE, virtual_address);

        page_offset = 0;
        while (page_offset < PAGE_SIZE && copy_offset <= program_header->p_filesz) {
            *((char *)virtual_address + page_offset) = *((char *)elf_header + program_header->p_offset + copy_offset);
            // kprintf("%c", *((char *)virtual_address + page_offset));
            page_offset++;
            copy_offset++;
        }

        virtual_address += PAGE_SIZE;
    }

    if(program_header->p_flags == (FLAG_READ | FLAG_EXECUTE)) {
        vma->file->bss_size = 0;
        vma->type = TEXT;
    } else {
        vma->file->bss_size = program_header->p_memsz - program_header->p_filesz;
        vma->type = DATA;
    }
}

void load_executable(task_struct *pcb, char *filename) {
    Elf64_Ehdr *elf_header = get_elf_header(filename);
    if (elf_header == NULL) {
        return;
    }

    Elf64_Phdr *program_header = (Elf64_Phdr *)((uint64_t)elf_header + elf_header->e_phoff);
    int i;

    for (i = 0; i < elf_header->e_phnum; i++) {
        read_program_header(pcb, elf_header, program_header);
        program_header++;
    }

    /* Add entry point in PCB */
    pcb->entry = elf_header->e_entry;
}
