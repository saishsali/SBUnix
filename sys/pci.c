#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/io.h>
#include <sys/string.h>
#include <sys/ahci.h>

#define AHCI_CLASS              0x01
#define AHCI_SUBCLASS           0x06
#define BAR_MEM                 0x20000000


uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    /*  Create configuration address:
        - Bit 31: Enable bit
        - Bits 30 - 24: Reserved
        - Bits 23 - 16: Bus Number
        - Bits 15 - 11: Device Numbe
        - Bits 10 - 8: Function Number
        - Bits 7 - 2: Register Number
        - Bits 1 - 0: 00

        0x80000000: To make enable bit 1
        offset & 0xfc: Two lowest bits are always 0 to make sure reads and writes are 32 bits aligned
    */
    address = (uint32_t)(((uint32_t)0x80000000) | (lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc));

    /* write out the address to CONFIG_ADDRESS I/O location (0xCF8) */
    outl(0xCF8, address);

    /* read in the data from CONFIG_DATA I/O location (0xCFC) */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    return (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
}

uint64_t pci_read_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    outl(0xCF8, address);

    return (uint32_t)(inl(0xCFC));
}

uint32_t remap_bar(uint32_t address) {
    outl(0xCFC, address);

    return (uint32_t)(inl(0xCFC));
}

void device_info(uint8_t bus, uint8_t device) {
    uint16_t vendor_id, device_id, class_subclass;
    uint32_t bar5;

    if ((vendor_id = pci_read_word(bus, device, 0 ,0)) != 0xFFFF) {
        device_id = pci_read_word(bus, device, 0 , 2);
        class_subclass = pci_read_word(bus, device, 0 , 10);

        if (((class_subclass & 0xFF00) >> 8) == AHCI_CLASS && (class_subclass & 0x00FF) == AHCI_SUBCLASS) {
            kprintf("AHCI controller found\n");
            kprintf("Vendor ID: %x, Device ID: %x\n", vendor_id, device_id);
            bar5 = pci_read_bar(bus, device, 0 , 0x24);

            // Move the bar5 (beyond physical memory space) to a place you can read (within physical memory space)
            bar5 = remap_bar(BAR_MEM);

            // Convert Physical address to virtual address
            probe_port((hba_mem_t *)((uint64_t)(0xffffffff80000000 + bar5)));
        }
    }
}

void init_pci() {
 	uint8_t bus = 0, slot;

    // 256 buses, each with up to 32 devices
    do {
    	for (slot = 0; slot < 32; slot++)
            device_info(bus, slot);
        bus++;
    } while (bus != 0);
 }
