#include <sys/kprintf.h>
#include <sys/pci.h>
#include <sys/defs.h>


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

uint16_t get_device_info(uint8_t bus, uint8_t device) {
    uint16_t vendor_id, device_id, class;
    if ((vendor_id = pci_read_word(bus, device, 0 ,0)) != 0xFFFF) {
        device_id = pci_read_word(bus, device, 0 ,2);
        class = pci_read_word(bus, device, 0 ,9);
        kprintf("device  %x. vendor_id %x.  class %x\n", device_id, vendor_id, class);
        return vendor_id;
    }
    return -1;
}

void check_all_buses() {
 	uint8_t bus, device;
    uint16_t vendor_id;
 	for (bus = 0; bus < 255; bus++) {
    	for (device = 0; device < 32; device++) {
    		vendor_id = get_device_info(bus, device);
            if (vendor_id == -1)
                continue;
    	}
    }
 }
