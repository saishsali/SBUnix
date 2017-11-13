#include <sys/defs.h>
#include <sys/paging.h>
#include <sys/page_descriptor.h>
#include <sys/kprintf.h>

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

/* Allocate Page Directory Pointer Table */
PDPT *allocate_pdpt(PML4 *pml4, uint64_t pml4_index) {
    Page *p = allocate_page();
    PDPT *pdpt = (PDPT *)page_to_physical_address(p);
    uint64_t pdpt_entry = (uint64_t)pdpt;
    pdpt_entry |= (PTE_P | PTE_W | PTE_U);
    pml4->entries[pml4_index] = pdpt_entry;

    return (PDPT *)(physical_to_virtual_address(pdpt));
}

/* Allocate Page Directory Table */
PDT *allocate_pdt(PDPT *pdpt, uint64_t pdpt_index) {
    Page *p = allocate_page();
    PDT *pdt = (PDT *)page_to_physical_address(p);
    uint64_t pdt_entry = (uint64_t)pdt;
    pdt_entry |= (PTE_P | PTE_W | PTE_U);
    pdpt->entries[pdpt_index] = pdt_entry;

    return (PDT *)(physical_to_virtual_address(pdt));
}

/* Allocate Page Table */
PT *allocate_pt(PDT *pdt, uint64_t pdt_index) {
    Page *p = allocate_page();
    PT *pt = (PT *)page_to_physical_address(p);
    uint64_t pt_entry = (uint64_t)pt;
    pt_entry |= (PTE_P | PTE_W | PTE_U);
    pdt->entries[pdt_index] = pt_entry;

    return (PT *)(physical_to_virtual_address(pt));
}

/* Map a Virtual address to a physical address */
void map_page(uint64_t virtual_address, uint64_t physical_address) {
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

    pt->entries[pt_index] = physical_address | (PTE_P | PTE_W | PTE_U);
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
        pt->entries[pt_index] = physical_address | (PTE_P | PTE_W | PTE_U);
        physical_address += PAGE_SIZE;
        virtual_address += PAGE_SIZE;
    }
}

/* Map the entire available memory starting from 0x0 to last physical address */
void map_available_memory(uint64_t last_physical_address) {
    uint64_t physical_address = 0x0UL;
    uint64_t virtual_address = KERNBASE;

    while (physical_address < last_physical_address) {
        map_page(virtual_address, physical_address);
        virtual_address += PAGE_SIZE;
        physical_address += PAGE_SIZE;
    }
    /* map the video memory physical address to the virtual address */
    map_page((uint64_t)(KERNBASE + VIDEO_MEMORY), VIDEO_MEMORY);
}

/* Setup page tables */
void setup_page_tables(uint64_t physbase, uint64_t physfree, uint64_t last_physical_address) {
    map_kernel_memory(physbase, physfree);
    map_available_memory(last_physical_address);
}
