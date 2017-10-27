#include <sys/defs.h>
#include <sys/paging.h>
#include <sys/page_descriptor.h>

extern char kernmem;

PDPT *allocate_pdpt(PML4 *pml4, uint64_t pml4_index) {
    Page *p = allocate_page();
    PDPT *pdpt = (PDPT *)page_to_physical_address(p);
    uint64_t pdpt_entry = (uint64_t)pdpt;
    pdpt_entry |= (PTE_P | PTE_W | PTE_U);
    pml4->entries[pml4_index] = pdpt_entry;

    return pdpt;
}

PDT *allocate_pdt(PML4 *pdpt, uint64_t pdpt_index) {
    Page *p = allocate_page();
    PDT *pdt = (PDT *)page_to_physical_address(p);
    uint64_t pdt_entry = (uint64_t)pdt;
    pdt_entry |= (PTE_P | PTE_W | PTE_U);
    pdpt->entries[pdpt_index] = pdt_entry;

    return pdt;
}

void setup_page_tables(uint64_t physbase, uint64_t physfree) {
    uint64_t kernel_virtual_address = (uint64_t)&kernmem;
    uint64_t pml4_index = PML4_INDEX((uint64_t)kernel_virtual_address);
    uint64_t pdpt_index = PDPT_INDEX((uint64_t)kernel_virtual_address);
    uint64_t pdt_index = PDT_INDEX((uint64_t)kernel_virtual_address);

    Page *p = allocate_page();
    PML4 *pml4 = (PML4 *)page_to_physical_address(p);
    PDPT *pdpt = allocate_pdpt(pml4, pml4_index);
    PDT *pdt = allocate_pdp(pdpt, pdpt_index);
}
