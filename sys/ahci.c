/*
    References:
    - https://www.intel.com/content/www/us/en/io/serial-ata/serial-ata-ahci-spec-rev1-3-1.html
    - http://wiki.osdev.org/AHCI
*/

#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/ahci.h>
#include <sys/io.h>
#include <sys/string.h>

#define SATA_SIG_ATA            0x00000101  // SATA drive
#define SATA_SIG_ATAPI          0xEB140101  // SATAPI drive
#define SATA_SIG_SEMB           0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM             0x96690101  // Port multiplier

#define HBA_PORT_DET_PRESENT    3
#define HBA_PORT_IPM_ACTIVE     1

#define AHCI_BASE               0x800000    // 4M

#define ATA_DEV_BUSY            0x80
#define ATA_DEV_DRQ             0x08
#define ATA_CMD_READ_DMA_EX     0x25
#define ATA_CMD_WRITE_DMA_EX    0x35
#define ATA_DEV_BUSY            0x80
#define ATA_DEV_DRQ             0x08

#define NUM_BLOCKS              100
#define BLOCK_SIZE              4096      // 4KB
#define AHCI_CLASS              0x01
#define AHCI_SUBCLASS           0x06
#define BAR_MEM                 0xA6000

// AHCI Base Memory Register
hba_mem_t *abar;

// Add a delay
void delay() {
    int spin = 0;

    while (spin < 1000000) {
        spin++;
    }
}

// Find a free command list slot
int find_cmdslot(hba_port_t *port) {
    // Check device status (DS) and command issue (CI) for each command slot status. If SACT and CI is not set, the slot is free.
    uint32_t slots = (port->sact | port->ci);
    int i;

    // Bit 12:08 - Number of command slots (1 to 32 maximum)
    uint8_t num_slots = (abar->cap >> 8) & 0x1F;

    for (i = 0; i < num_slots; i++) {
        if ((slots & 1) == 0)
            return i;

        // Next command slot status
        slots >>= 1;
    }
    kprintf("Cannot find free command slot\n");

    return -1;
}

int read(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint8_t *buf) {
    port->is_rwc = (uint32_t)-1;   // Clear pending interrupt bits
    int i;
    int slot = find_cmdslot(port);
    if (slot == -1)
        return 0;

    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)port->clb;
    cmdheader += slot;

    // Command FIS size
    cmdheader->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);

    // Read from device
    cmdheader->w = 0;

    // PRDT entries count
    cmdheader->prdtl = (uint16_t)((count - 1) >> 3) + 1;

    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t*)(cmdheader->ctba);
    memset(cmdtbl, 0, sizeof(hba_cmd_tbl_t) + (cmdheader->prdtl - 1) * sizeof(hba_prdt_entry_t));

    // 4K bytes (8 sectors) per PRDT
    for (i = 0; i < cmdheader->prdtl - 1; i++) {
        cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
        cmdtbl->prdt_entry[i].dbc = 4 * 1024; // 4K bytes
        cmdtbl->prdt_entry[i].i = 1;
        buf += 4 * 1024;  // 4K words
        count -= 8;    // 8 sectors
    }

    // Last entry
    cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
    cmdtbl->prdt_entry[i].dbc = count << 9;   // 512 bytes per sector
    cmdtbl->prdt_entry[i].i = 1;

    // Setup command
    fis_reg_h2d_t *cmdfis = (fis_reg_h2d_t*)(&cmdtbl->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;  // Command
    cmdfis->command = ATA_CMD_READ_DMA_EX;

    cmdfis->lba0 = (uint8_t)startl;
    cmdfis->lba1 = (uint8_t)(startl >> 8);
    cmdfis->lba2 = (uint8_t)(startl >> 16);
    cmdfis->device = 1 << 6;  // LBA mode

    cmdfis->lba3 = (uint8_t)(startl >> 24);
    cmdfis->lba4 = (uint8_t)starth;
    cmdfis->lba5 = (uint8_t)(starth >> 8);

    cmdfis->count = count;

    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)));
    port->ci = 1 << slot; // Issue command

    // Wait for completion
    while (1) {
        // In some longer duration reads, it may be helpful to spin on the DPS bit
        // in the PxIS port field as well (1 << 5)
        if (!(port->ci & (1 << slot))) {
            break;
        }

        // Task file error
        if (port->is_rwc & HBA_PxIS_TFES) {
            kprintf("Read disk error\n");
            return 0;
        }
    }

    // Check again
    if (port->is_rwc & HBA_PxIS_TFES) {
        kprintf("Read disk error\n");
        return 0;
    }

    return 1;
}


int write1(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint8_t *buf) {
    port->is_rwc = (uint32_t)-1;       // Clear pending interrupt bits
    int i;
    int slot = find_cmdslot(port);
    if (slot == -1)
        return 0;

    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)port->clb;
    cmdheader += slot;

    // Command FIS size
    cmdheader->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);

    // Write to device
    cmdheader->w = 1;

    // PRDT entries count
    cmdheader->prdtl = (uint16_t)((count - 1) >> 3) + 1;

    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t*)(cmdheader->ctba);
    memset(cmdtbl, 0, sizeof(hba_cmd_tbl_t) + (cmdheader->prdtl - 1) * sizeof(hba_prdt_entry_t));

    // 4K bytes (8 sectors) per PRDT
    for (i = 0; i < cmdheader->prdtl - 1; i++) {
        cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
        cmdtbl->prdt_entry[i].dbc = 4 * 1024; // 4K bytes
        cmdtbl->prdt_entry[i].i = 1;
        buf += 4 * 1024;  // 4K words
        count -= 8;    // 8 sectors
    }

    // Last entry
    cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
    cmdtbl->prdt_entry[i].dbc = count << 9;   // 512 bytes per sector
    cmdtbl->prdt_entry[i].i = 1;

    // Setup command
    fis_reg_h2d_t *cmdfis = (fis_reg_h2d_t*)(&cmdtbl->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;  // Command
    cmdfis->command = ATA_CMD_WRITE_DMA_EX;

    cmdfis->lba0 = (uint8_t)startl;
    cmdfis->lba1 = (uint8_t)(startl >> 8);
    cmdfis->lba2 = (uint8_t)(startl >> 16);
    cmdfis->device = 1 << 6;  // LBA mode

    cmdfis->lba3 = (uint8_t)(startl >> 24);
    cmdfis->lba4 = (uint8_t)starth;
    cmdfis->lba5 = (uint8_t)(starth >> 8);

    cmdfis->count = count;

    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)));

    port->ci = 1 << slot; // Issue command

    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit
        // in the PxIS port field as well (1 << 5)
        if (!(port->ci & (1 << slot))) {
            break;
        }

        // Task file error
        if (port->is_rwc & HBA_PxIS_TFES) {
            kprintf("Write disk error\n");
            return 0;
        }
    }

    // Check again
    if (port->is_rwc & HBA_PxIS_TFES) {
        kprintf("Write disk error\n");
        return 0;
    }

    return 1;
}

// Serial ATA AHCI 1.3.1 Specification (Section 10.4.2)
void port_reset(hba_port_t *port) {
    uint32_t flag;

    // Invoke a COMRESET on the interface and start a re-establishment of Phy layer communications
    // Transitions to both Partial and Slumber states disabled
    port->sctl = 0x301;
    delay();

    // Clearing PxSCTL.DET to 0h; this ensures that at least one COMRESET signal is sent over the interface
    port->sctl = 0x300;
    delay();

    // Set Spin-Up Device (SUD)
    port->cmd |= 0x00000002;
    delay();

    // Set Power On Device (POD)
    port->cmd |= 0x00000004;
    delay();

    // Set Interface Communication Control (ICC)
    flag = port->cmd;
    flag &= 0x0FFFFFFF;
    flag |= 0x10000000;
    port->cmd = flag;
    delay();

    // Write all 1s to the PxSERR register to clear any bits that were set as part of the port reset
    port->serr_rwc = 0xFFFFFFFF;
    delay();

    // Write all 1s to the PxIS register to clear any bits that were set as part of the port reset
    port->is_rwc = 0xFFFFFFFF;
    delay();

    // Wait for communication to be re-established
    while ((port->ssts & 0x0F) != 3);
    delay();
}

// Start command engine (Section 3.3.7 Serial ATA AHCI 1.3.1 Specification)
void start_cmd(hba_port_t *port) {
    port_reset(port);

    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR);

    // Set FRE (bit4)
    port->cmd |= HBA_PxCMD_FRE;
    delay();

    // Set ST (bit0)
    port->cmd |= HBA_PxCMD_ST;
    delay();
}

// Stop command engine
void stop_cmd(hba_port_t *port) {
    // Clear ST (bit0)
    port->cmd &= ~HBA_PxCMD_ST;
    delay();
    while (port->cmd & HBA_PxCMD_CR);

    // Clear FRE (bit4)
    port->cmd &= ~HBA_PxCMD_FRE;
    delay();
    while (port->cmd & HBA_PxCMD_FR);
}

void port_rebase(hba_port_t *port, int portno) {
    int i;

    stop_cmd(port); // Stop command engine

    // Command list offset: 1K*portno
    // Command list entry size = 32
    // Command list entry maxim count = 32
    // Command list maxim size = 32*32 = 1K per port
    port->clb = AHCI_BASE + (portno << 10);
    memset((void*)(port->clb), 0, 1024);

    // FIS offset: 32K+256*portno
    // FIS entry size = 256 bytes per port
    port->fb = AHCI_BASE + (32 << 10) + (portno << 8);
    memset((void*)(port->fb), 0, 256);

    // Command table offset: 40K + 8K*portno
    // Command table size = 256*32 = 8K per port
    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)(port->clb);
    for (i = 0; i < 32; i++) {

    	// 8 prdt entries per command table
    	// 256 bytes per command table, 64+16+48+16*8
        cmdheader[i].prdtl = 8;

        // Command table offset: 40K + 8K*portno + cmdheader_index*256
        cmdheader[i].ctba = AHCI_BASE + (40 << 10) + (portno << 13) + (i << 8);
        memset((void*)cmdheader[i].ctba, 0, 256);
    }

    // Start command engine
    start_cmd(port);
}

// Check device type
int check_type(hba_port_t *port) {
    switch (port->sig) {
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        case SATA_SIG_ATA:
            return AHCI_DEV_SATA;
        default:
            return -1;
    }
}

// Verify read and write to disks
void verify_read_write(uint8_t port) {
    uint32_t i, j;
    int flag = 0;
    uint8_t *write_buffer = (uint8_t *)0x400000, *read_buffer = (uint8_t *)0x500000;

    kprintf("Writing to disk ...\n");

    // Write 0 to the first 4KB Block
    // 512 bytes * 8 blocks = 4096 KB
    memset(write_buffer, 0, BLOCK_SIZE);
    write1(&abar->ports[port], 0, 0, 8, write_buffer);

    for (i = 0; i < NUM_BLOCKS; i++) {
        memset(write_buffer, i, BLOCK_SIZE);
        write1(&abar->ports[port], i * 8, 0, 8, write_buffer);
    }

    kprintf("Reading from disk ...\n");

    for (i = 0; i < NUM_BLOCKS; i++) {
        read(&abar->ports[port], i * 8, 0, 8, read_buffer);
        for (j = 0; j < BLOCK_SIZE; j++) {
            if (j == 0) {
                kprintf("%d ", read_buffer[j]);
            }
            // If read-write match is found
            if (read_buffer[j] == i) {
                flag = 1;
            } else {
                // If read-write mismatch
                flag = 0;
                break;
            }
        }

        // Break even if a single mismatch is found
        if (flag == 0) {
            break;
        }
    }

    if (flag == 1)
        kprintf("\nRead and write verified successfully at port %d\n \n", port);
}

// Search disk in ports impelemented
void probe_port() {
    uint32_t pi = abar->pi, dt, i = 0;
    uint8_t disk_read_write = 0;

    while (i < 32) {
        if (pi & 1) {
            dt = check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA) {
                kprintf("SATA drive found at port %d\n", i);
                if (disk_read_write == 0) {
                    port_rebase(&abar->ports[i], i);
                    verify_read_write(i);
                    disk_read_write = 1;
                }
            } else if (dt == AHCI_DEV_SATAPI) {
                kprintf("SATAPI drive found at port %d\n", i);
            } else if (dt == AHCI_DEV_SEMB) {
                kprintf("SEMB drive found at port %d\n", i);
            } else if (dt == AHCI_DEV_PM) {
                kprintf("PM drive found at port %d\n", i);
            } else {
                kprintf("No drive found at port %d\n", i);
            }
        }
        pi >>= 1;
        i++;
    }
}

void init_ahci(uint32_t bar5) {
    // Convert Physical address to virtual address
    abar = (hba_mem_t *)((uint64_t)bar5);

    // Set bit31 to enable AHCI
    abar->ghc |= 0x80000000;

    probe_port();
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

uint64_t pci_remap_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t remap_address) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;

    address = (uint32_t)((lbus << 16) | (lslot << 11) | (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
    outl(0xCF8, address);
    outl(0xCFC, remap_address);

    return (uint32_t)(inl(0xCFC));
}

void device_info(uint8_t bus, uint8_t device) {
    uint16_t vendor_id, device_id, class_subclass;
    uint32_t bar5;
    int func = 0;

    for (func = 0; func < 8; func++) {
        if ((vendor_id = pci_read_word(bus, device, func ,0)) != 0xFFFF) {
            device_id = pci_read_word(bus, device, func , 2);
            class_subclass = pci_read_word(bus, device, func , 10);

            if (((class_subclass & 0xFF00) >> 8) == AHCI_CLASS && (class_subclass & 0x00FF) == AHCI_SUBCLASS) {
                kprintf("*** AHCI controller found ***\n");
                kprintf("Vendor ID: %x, Device ID: %x, Class code: %x, Subclass: %x\n \n", vendor_id, device_id, AHCI_CLASS, AHCI_SUBCLASS);

                // Move the bar5 (beyond physical memory space) to a place you can read (within physical memory space)
                bar5 = pci_remap_bar(bus, device, func , 0x24, BAR_MEM);
                init_ahci(bar5);
            }
        }
    }
}

void init_pci() {
    uint8_t bus = 0, slot;

    kprintf("Walking PCI configuration space ...\n");
    // 256 buses, each with up to 32 devices
    do {
        for (slot = 0; slot < 32; slot++)
            device_info(bus, slot);
        bus++;
    } while (bus != 0);
 }
