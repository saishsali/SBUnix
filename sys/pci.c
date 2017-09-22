#include <sys/kprintf.h>
#include <sys/pci.h>
#include <sys/defs.h>

#define AHCI_CLASS 0x01
#define AHCI_SUBCLASS 0x02


uint32_t inl(uint16_t address) {
	uint32_t val;
	__asm__(
		"inl %w1, %0;"
		: "=a" (val)
		: "Nd" (address)
	);
	return  val;
}
void outl (uint16_t port, uint32_t data) {
	__asm__( "outl %0, %w1" : : "a"(data), "Nd"(port) );
} 

uint16_t pci_read_word (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;

    /* create configuration address as per Figure 1 */
    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    /* write out the address */
    outl(0xCF8, address);
     /* read in the data */
    /* (offset & 2) * 8) = 0 will choose the first word of the 32 bits register */
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



uint16_t get_device_info(uint8_t bus, uint8_t device) {
    uint16_t vendor_id, device_id, sub_class, class;
    uint32_t bar5;
    if ((vendor_id = pci_read_word(bus, device, 0 ,0)) != 0xFFFF) {
        device_id = pci_read_word(bus, device, 0 , 2);
        class = (pci_read_word(bus, device, 0 , 8) & 0xFF00) >> 8;
        sub_class = pci_read_word(bus, device, 0 ,9) & 0x00FF;
        if (class == AHCI_CLASS && sub_class == AHCI_SUBCLASS) {
            kprintf("\n Ahci controller found");
            kprintf("\n vendor id %x, devcie id %x", vendor_id, device_id);
            bar5 = pci_read_bar(bus, device, 0 , 0x24);
            kprintf("\n First disk connected address %x\n",bar5);
        }
        return vendor_id;
    }
    return -1;
}

void check_all_buses() {
 	int bus, device;
    uint16_t vendor_id;
 	for (bus = 0; bus < 256; bus++) {
    	for (device = 0; device < 32; device++) {
    		vendor_id = get_device_info(bus, device);
            if (vendor_id == -1)
                continue;
    	}
    }
 }
