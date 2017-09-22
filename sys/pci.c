#include <sys/kprintf.h>
#include <sys/io.h>
#define AHCI_CLASS 0x01
#define AHCI_SUBCLASS 0x02

uint16_t pci_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;

    /*
        - Bit 31: Enable bit
        - Bits 30 - 24: Reserved
        - Bits 23 - 16: Bus Number
        - Bits 15 - 11: Device Numbe
        - Bits 10 - 8: Function Number
        - Bits 7 - 2: Register Number
        - Bits 1 - 0: 00
    */
    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address to CONFIG_ADDRESS I/O location (0xCF8) */
    outl(0xCF8, address);

    /* read in the data from CONFIG_DATA I/O location (0xCFC) */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    // return (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
    tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);

    return (tmp);
}

uint32_t pci_read_bar (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;

    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address */
    outl(0xCF8, address);
     /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
    tmp = (uint32_t)(inl(0xCFC) >> (offset & 2) * 8);
    return (tmp);
}

void get_device_info(uint8_t bus, uint8_t device) {
    uint16_t vendor_id, device_id, sub_class, class;
    uint32_t bar5;
    if ((vendor_id = pci_read_word(bus, device, 0 ,0)) != 0xFFFF) {
        device_id = pci_read_word(bus, device, 0 , 2);
        class = (pci_read_word(bus, device, 0 , 8) & 0xFF00) >> 8;
        sub_class = pci_read_word(bus, device, 0 ,9) & 0x00FF;
        if (class == AHCI_CLASS && sub_class == AHCI_SUBCLASS) {
            kprintf("AHCI controller found\n");
            kprintf("Vendor ID: %x, Device ID: %x\n", vendor_id, device_id);
            bar5 = pci_read_bar(bus, device, 0 , 0x24);
            kprintf("First disk connected address: %x\n",bar5);
        }
    }
}

void check_all_buses() {
 	uint8_t bus = 0, slot;

    do {
    	for (slot = 0; slot < 32; slot++)
            get_device_info(bus, slot);
        bus++;
    } while (bus != 0);
 }
