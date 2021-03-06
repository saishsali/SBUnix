/*
    References:
    - http://rayseyfarth.com/asm/pdf/ch04-memory-mapping.pdf
    - http://wiki.osdev.org/Paging
*/
#ifndef _PAGING_H
#define _PAGING_H

/* Slide no 5: http://rayseyfarth.com/asm/pdf/ch04-memory-mapping.pdf */
#define PML4_INDEX(x) ((x >> 39) & 0x1FF)
#define PDPT_INDEX(x) ((x >> 30) & 0x1FF)
#define PDT_INDEX(x) ((x >> 21) & 0x1FF)
#define PT_INDEX(x) ((x >> 12) & 0x1FF)

/* Page table/directory entry flags */
#define PTE_P       0x001   // Present
#define PTE_W       0x002   // Writeable
#define PTE_U       0x004   // User
#define PTE_PWT     0x008   // Write-Through
#define PTE_PCD     0x010   // Cache-Disable
#define PTE_A       0x020   // Accessed
#define PTE_D       0x040   // Dirty
#define PTE_PS      0x080   // Page Size
#define PTE_MBZ     0x180   // Bits must be zero
#define PTE_COW     0x200   // Copy-on-write

#define RX_FLAG     (PTE_P | PTE_U)
#define RW_FLAG     (PTE_P | PTE_U | PTE_W)

#define PROT_EXEC   0x001
#define PROT_WRITE  0x002
#define PROT_READ   0x004

/* Last 12 bits are used for flags */
#define GET_ADDRESS(x) (x & 0xFFFFFFFFFFFFF000)

#define SET_READ_ONLY(x) (*x = *x & 0xFFFFFFFFFFFFFFFD)

#define SET_WRITABLE(x) (*x = *x | PTE_W)

#define GET_FLAGS(x) (x & 0xFFF)

#define SET_COPY_ON_WRITE(x) (*x = *x | PTE_COW)

#define UNSET_COPY_ON_WRITE(x) (*x = *x & 0xFFFFFFFFFFFFFDFF)

/* Video memory physical address */
#define VIDEO_MEMORY 0xb8000

/* Page Map Level 4 */
struct PML4 {
    uint64_t entries[512];
};

// Page directory pointer table
struct PDPT {
    uint64_t entries[512];
};

// Page directory table
struct PDT {
    uint64_t entries[512];
};

// Page table
struct PT {
    uint64_t entries[512];
};

typedef struct PML4 PML4;
typedef struct PDPT PDPT;
typedef struct PDT PDT;
typedef struct PT PT;

void setup_page_tables(uint64_t physbase, uint64_t physfree, uint64_t last_physical_address);

void map_page(uint64_t virtual_address, uint64_t physical_address, uint16_t flags);

void load_cr3();

uint64_t get_cr3();

void set_cr3(uint64_t cr3);

void *set_user_address_space();

void *get_page_table_entry(void *virtual_address);

uint64_t virtual_to_physical_address(void *virtual_address);

void remove_page_tables(uint64_t cr3);

#endif
