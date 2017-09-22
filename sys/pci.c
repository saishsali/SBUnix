#include <sys/kprintf.h>
#include <sys/pci.h>
#include <sys/defs.h>
#include <sys/ahci.h>
#include <sys/io.h>

#define AHCI_CLASS 0x01
#define AHCI_SUBCLASS 0x06

#define SATA_SIG_ATA    0x00000101  // SATA drive
#define SATA_SIG_ATAPI  0xEB140101  // SATAPI drive
#define SATA_SIG_SEMB   0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM 0x96690101  // Port multiplier
#define HBA_PORT_DET_PRESENT 3
#define HBA_PORT_IPM_ACTIVE 1

// Check device type
static int check_type(hba_port_t *port)
{
    uint32_t ssts = port->ssts;
    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;

    if (det != HBA_PORT_DET_PRESENT)    // Check drive status
        return AHCI_DEV_NULL;
    if (ipm != HBA_PORT_IPM_ACTIVE)
        return AHCI_DEV_NULL;

    switch (port->sig)
    {
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        default:
            return AHCI_DEV_SATA;
    }
}

void probe_port(hba_mem_t *abar)
{
    // Search disk in impelemented ports
    uint32_t pi = abar->pi;
    kprintf("\n herer --- %x", abar->cap);
    int i = 0;
    while (i<32) {
        if (pi & 1) {
            int dt = check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA) {
                kprintf("SATA drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_SATAPI) {
                kprintf("SATAPI drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_SEMB) {
                kprintf("SEMB drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_PM) {
                kprintf("PM drive found at port %d\n", i);
            }
            else {
                kprintf("No drive found at port %d\n", i);
            }
        }
        pi >>= 1;
        i ++;
    }
}

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
    tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xFFFF);
    return (tmp);
}

uint64_t pci_read_bar (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint64_t tmp = 0;

    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    outl(0xCF8, address);
    tmp = (uint64_t)(inl(0xCFC));
    return (tmp);
}


uint16_t get_device_info(uint8_t bus, uint8_t device) {
    uint16_t vendor_id, device_id;
    uint64_t bar5, class, sub_class;
    if ((vendor_id = pci_read_word(bus, device, 0 ,0)) != 0xFFFF) {
        device_id = pci_read_word(bus, device, 0 , 2);
        class = (pci_read_bar(bus, device, 0 , 8) & 0xFFFF0000) >> 24;
        sub_class = (pci_read_bar(bus, device, 0 ,9) & 0xFF0000) >> 16;
        if (class == AHCI_CLASS && sub_class == AHCI_SUBCLASS) {
            kprintf("AHCI controller found\n");
            kprintf("Vendor ID: %x, Device ID: %x\n", vendor_id, device_id);
            bar5 = pci_read_bar(bus, device, 0 , 0x24);
            probe_port((hba_mem_t *)(0xFFFFFFFF00000000 + (uint64_t)bar5));
        }
    }
    return -1;
}

void check_all_buses() {
 	uint8_t bus = 0, slot;

    do {
    	for (slot = 0; slot < 32; slot++)
            get_device_info(bus, slot);
        bus++;
    } while (bus != 0);
 }
