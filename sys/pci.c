/*
    References:
    - http://wiki.osdev.org/Pci
*/

#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/ahci.h>
#include <sys/io.h>
#include <sys/string.h>
#include <sys/pci.h>

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

uint32_t pci_remap_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t remap_address) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    outl(0xCF8, address);
    outl(0xCFC, remap_address);

    return (uint32_t)(inl(0xCFC));
}

device_info get_device_info(uint8_t bus, uint8_t slot) {
    uint16_t class_subclass;
    device_info device;

    if ((device.vendor_id = pci_read_word(bus, slot, 0 ,0)) != 0xFFFF) {
        device.device_id = pci_read_word(bus, slot, 0 , 2);
        class_subclass = pci_read_word(bus, slot, 0 , 10);
        device.class_code = (class_subclass & 0xFF00) >> 8;
        device.subclass = class_subclass & 0x00FF;
        device.bus = bus;
        device.slot = slot;
    }

    return device;
}
