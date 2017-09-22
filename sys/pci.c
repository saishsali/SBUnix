#include <sys/kprintf.h>
#include <sys/io.h>

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

void device_info(uint8_t bus, uint8_t slot) {
    uint16_t vendor, device;

    if ((vendor = pci_read_word(bus, slot, 0, 0)) != 0xFFFF) {
        device = pci_read_word(bus, slot, 0, 2);
        if (device == 0x2922) {
            kprintf("AHCI Controller located\nVendor ID: %x, Device ID: %x \n", vendor, device);
        }
    }
}

void check_all_buses() {
 	uint8_t bus = 0, slot;

    do {
    	for (slot = 0; slot < 32; slot++)
            device_info(bus, slot);
        bus++;
    } while (bus != 0);
 }
