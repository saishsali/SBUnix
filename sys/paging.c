#include <sys/defs.h>
#include <sys/paging.h>
#include <sys/page_descriptor.h>
#include <sys/kprintf.h>
#include <sys/memory.h>

extern Page *page_free_list;

/* Page map level 4 */
PML4 *pml4;

uint64_t virtual_to_physical_address(void *virtual_address) {
    return (uint64_t)virtual_address - KERNBASE;
}

uint64_t physical_to_virtual_address(void *physical_address) {
    return (uint64_t)physical_address + KERNBASE;
}

/* Load page table base address in CR3 register */
void load_cr3() {
    uint64_t cr3 = virtual_to_physical_address(pml4);
    __asm__ volatile(
        "movq %0, %%cr3;"
        :
        : "b" (cr3)
    );
}

/* Get CR3 register value */
uint64_t get_cr3() {
    uint64_t cr3;
    __asm__ volatile(
        "movq %%cr3, %0;"
        : "=r"(cr3)
    );

    return physical_to_virtual_address((void *)cr3);
}

/* Set page table base address in CR3 register */
void set_cr3(uint64_t cr3) {
    pml4 = (PML4 *)cr3;
    cr3 = virtual_to_physical_address((void *)cr3);
    __asm__ volatile(
        "movq %0, %%cr3;"
        :
        : "b" (cr3)
    );
}

/* Allocate Page Directory Pointer Table */
PDPT *allocate_pdpt(PML4 *pml4, uint64_t pml4_index) {
    Page *p = allocate_page();
    PDPT *pdpt = (PDPT *)page_to_physical_address(p);
    uint64_t pdpt_entry = (uint64_t)pdpt;
    pdpt_entry |= RW_FLAG;
    pml4->entries[pml4_index] = pdpt_entry;

    return (PDPT *)(physical_to_virtual_address(pdpt));
}

/* Allocate Page Directory Table */
PDT *allocate_pdt(PDPT *pdpt, uint64_t pdpt_index) {
    Page *p = allocate_page();
    PDT *pdt = (PDT *)page_to_physical_address(p);
    uint64_t pdt_entry = (uint64_t)pdt;
    pdt_entry |= RW_FLAG;
    pdpt->entries[pdpt_index] = pdt_entry;

    return (PDT *)(physical_to_virtual_address(pdt));
}

/* Allocate Page Table */
PT *allocate_pt(PDT *pdt, uint64_t pdt_index) {
    Page *p = allocate_page();
    PT *pt = (PT *)page_to_physical_address(p);
    uint64_t pt_entry = (uint64_t)pt;
    pt_entry |= RW_FLAG;
    pdt->entries[pdt_index] = pt_entry;

    return (PT *)(physical_to_virtual_address(pt));
}

/* Map a Virtual address to a physical address */
void map_page(uint64_t virtual_address, uint64_t physical_address, uint16_t flags) {
    PDPT *pdpt;
    PDT *pdt;
    PT *pt;
    uint64_t pml4_index = PML4_INDEX(virtual_address);
    uint64_t pdpt_index = PDPT_INDEX(virtual_address);
    uint64_t pdt_index = PDT_INDEX(virtual_address);
    uint64_t pt_index = PT_INDEX(virtual_address);

    uint64_t pml4_entry = pml4->entries[pml4_index];
    if (pml4_entry & PTE_P) {
        pdpt = (PDPT *)GET_ADDRESS(pml4_entry);
        pdpt = (PDPT *)physical_to_virtual_address(pdpt);
    } else {
        pdpt = allocate_pdpt(pml4, pml4_index);
    }

    uint64_t pdpt_entry = pdpt->entries[pdpt_index];
    if (pdpt_entry & PTE_P) {
        pdt = (PDT *)GET_ADDRESS(pdpt_entry);
        pdt = (PDT *)physical_to_virtual_address(pdt);
    } else {
        pdt = allocate_pdt(pdpt, pdpt_index);
    }

    uint64_t pdt_entry = pdt->entries[pdt_index];
    if (pdt_entry & PTE_P) {
        pt = (PT *)GET_ADDRESS(pdt_entry);
        pt = (PT *)physical_to_virtual_address(pt);
    } else {
        pt = allocate_pt(pdt, pdt_index);
    }

    pt->entries[pt_index] = physical_address | flags;
}

/* Get page table entry for a virtual address */
void *get_page_table_entry(void *virtual_address) {
    PDPT *pdpt;
    PDT *pdt;
    PT *pt;
    uint64_t pml4_index = PML4_INDEX((uint64_t)virtual_address);
    uint64_t pdpt_index = PDPT_INDEX((uint64_t)virtual_address);
    uint64_t pdt_index = PDT_INDEX((uint64_t)virtual_address);
    uint64_t pt_index = PT_INDEX((uint64_t)virtual_address);

    uint64_t pml4_entry = pml4->entries[pml4_index];
    if (pml4_entry & PTE_P) {
        pdpt = (PDPT *)GET_ADDRESS(pml4_entry);
        pdpt = (PDPT *)physical_to_virtual_address(pdpt);
    } else {
        return NULL;
    }

    uint64_t pdpt_entry = pdpt->entries[pdpt_index];
    if (pdpt_entry & PTE_P) {
        pdt = (PDT *)GET_ADDRESS(pdpt_entry);
        pdt = (PDT *)physical_to_virtual_address(pdt);
    } else {
        return NULL;
    }

    uint64_t pdt_entry = pdt->entries[pdt_index];
    if (pdt_entry & PTE_P) {
        pt = (PT *)GET_ADDRESS(pdt_entry);
        pt = (PT *)physical_to_virtual_address(pt);
    } else {
        return NULL;
    }

    return &pt->entries[pt_index];
}

/*
    Map kernel memory from virtual address KERNBASE + physbase => physical address physbase to
    Virtual address KERNBASE + physfree => physical address physfree
*/
void map_kernel_memory(uint64_t physbase, uint64_t physfree) {
    uint64_t kernel_virtual_address = (uint64_t)(KERNBASE + physbase);
    uint64_t pml4_index = PML4_INDEX(kernel_virtual_address);
    uint64_t pdpt_index = PDPT_INDEX(kernel_virtual_address);
    uint64_t pdt_index = PDT_INDEX(kernel_virtual_address);

    Page *p = allocate_page();
    pml4 = (PML4 *)page_to_virtual_address(p);
    PDPT *pdpt = allocate_pdpt(pml4, pml4_index);
    PDT *pdt = allocate_pdt(pdpt, pdpt_index);
    PT *pt = allocate_pt(pdt, pdt_index);

    uint64_t physical_address = physbase;
    uint64_t virtual_address  = kernel_virtual_address;

    while (physical_address < physfree) {
        uint64_t pt_index = PT_INDEX(virtual_address);
        pt->entries[pt_index] = physical_address | RW_FLAG;
        physical_address += PAGE_SIZE;
        virtual_address += PAGE_SIZE;
    }
}

/* Map the entire available memory starting from 0x0 to last physical address */
void map_available_memory(uint64_t last_physical_address) {
    uint64_t physical_address = 0x0UL;
    uint64_t virtual_address = KERNBASE;

    while (physical_address < last_physical_address) {
        map_page(virtual_address, physical_address, RW_FLAG);
        virtual_address += PAGE_SIZE;
        physical_address += PAGE_SIZE;
    }
    /* map the video memory physical address to the virtual address */
    map_page((uint64_t)(KERNBASE + VIDEO_MEMORY), VIDEO_MEMORY, RW_FLAG);
}

/* Setup page tables and load cr3 */
void setup_page_tables(uint64_t physbase, uint64_t physfree, uint64_t last_physical_address) {
    map_kernel_memory(physbase, physfree);
    map_available_memory(last_physical_address);
    load_cr3();
}

void *set_user_address_space() {
    PML4 *new_pml4 = (PML4 *)kmalloc(sizeof(PML4));
    PML4 *current_pml4 = (PML4 *)get_cr3();
    new_pml4->entries[511] = current_pml4->entries[511];

    return (void *)new_pml4;
}

/* Free memory allocated for the page tables */
void remove_page_tables(uint64_t cr3) {
    int pml4_index, pdpt_index, pdt_index;
    uint64_t pml4_entry, pdpt_entry, pdt_entry;
    PML4 *task_pml4 = (PML4 *)cr3;
    PDPT *pdpt;
    PDT *pdt;
    PT* pt;

    for (pml4_index = 0; pml4_index < 511; pml4_index++) {
        pml4_entry = task_pml4->entries[pml4_index];
        if (!pml4_entry) {
            continue;
        }
        pdpt = (PDPT *)physical_to_virtual_address((PDPT *)GET_ADDRESS(pml4_entry));

        for (pdpt_index = 0; pdpt_index < 512; pdpt_index++) {
            pdpt_entry = pdpt->entries[pdpt_index];
            if (!pdpt_entry) {
                continue;
            }
            pdt = (PDT *)physical_to_virtual_address((PDT *)GET_ADDRESS(pdpt_entry));

            for (pdt_index = 0; pdt_index < 512; pdt_index++) {
                pdt_entry = pdt->entries[pdt_index];
                if (!pdt_entry) {
                    continue;
                }
                pt = (PT *)physical_to_virtual_address((PT *)GET_ADDRESS(pdt_entry));

                free_kernel_memory(pt);
            }
            free_kernel_memory(pdt);
        }
        free_kernel_memory(pdpt);
    }
    free_kernel_memory((void *)cr3);
}
