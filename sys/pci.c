#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/ahci.h>
#include <sys/io.h>

#define AHCI_CLASS              0x01
#define AHCI_SUBCLASS           0x06
#define SATA_SIG_ATA            0x00000101  // SATA drive
#define SATA_SIG_ATAPI          0xEB140101  // SATAPI drive
#define SATA_SIG_SEMB           0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM             0x96690101  // Port multiplier
#define HBA_PORT_DET_PRESENT    3
#define HBA_PORT_IPM_ACTIVE     1
#define BAR_MEM                 0x20000000

// Check device type
int check_type(hba_port_t *port)
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

// Search disk in ports impelemented
void probe_port(hba_mem_t *abar)
{
    uint32_t pi = abar->pi;
    int i = 0;
    while (i < 32) {
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
        i++;
    }
}

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
