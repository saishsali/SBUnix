#include <sys/kprintf.h>
#include <sys/defs.h>
#include <sys/ahci.h>
#include <sys/io.h>
#include <sys/string.h>

#define AHCI_CLASS              0x01
#define AHCI_SUBCLASS           0x06
#define SATA_SIG_ATA            0x00000101  // SATA drive
#define SATA_SIG_ATAPI          0xEB140101  // SATAPI drive
#define SATA_SIG_SEMB           0xC33C0101  // Enclosure management bridge
#define SATA_SIG_PM             0x96690101  // Port multiplier
#define HBA_PORT_DET_PRESENT    3
#define HBA_PORT_IPM_ACTIVE     1
#define BAR_MEM                 0x20000000
#define AHCI_BASE               0x30000000    // 4M
#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08
#define ATA_CMD_READ_DMA_EX     0x25
#define ATA_CMD_WRITE_DMA_EX     0x35
#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

// Find a free command list slot
int find_cmdslot(hba_port_t *port)
{
    // If not set in SACT and CI, the slot is free
    uint32_t slots = (port->sact | port->ci);
    for (int i = 0; i < 32; i++)
    {
        if ((slots & 1) == 0)
            return i;
        slots >>= 1;
    }
    kprintf("Cannot find free command list entry\n");

    return -1;
}


int read(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf)
{
    port->is_rwc = (uint32_t)-1;   // Clear pending interrupt bits
    int spin = 0; // Spin lock timeout counter
    int i;
    int slot = find_cmdslot(port);
    if (slot == -1)
        return 0;
 
    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)port->clb;
    cmdheader += slot;
    cmdheader->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t); // Command FIS size
    cmdheader->w = 0;       // Read from device
    cmdheader->prdtl = (uint16_t)((count - 1) >> 1) + 1;    // PRDT entries count
 
    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t*)(cmdheader->ctba);
    memset(cmdtbl, 0, sizeof(hba_cmd_tbl_t) + (cmdheader->prdtl-1) * sizeof(hba_prdt_entry_t));
 
    // 8K bytes (16 sectors) per PRDT
    for (i = 0; i < cmdheader->prdtl - 1; i++)
    {
        cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
        cmdtbl->prdt_entry[i].dbc = 8 * 1024; // 8K bytes
        cmdtbl->prdt_entry[i].i = 1;
        buf += 4 * 1024;  // 4K words
        count -= 2;    // 16 sectors
    }

    // Last entry
    cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
    cmdtbl->prdt_entry[i].dbc = count << 12;   // 512 bytes per sector
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
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000)
    {
        kprintf("Port is hung\n");
        return 0;
    }
 
    port->ci = 1<<slot; // Issue command
 
    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit 
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1 << slot)) == 0) 
            break;
        if (port->is_rwc & HBA_PxIS_TFES)   // Task file error
        {
            kprintf("Read disk error\n");
            return 0;
        }
    }
 
    // Check again
    if (port->is_rwc & HBA_PxIS_TFES)
    {
        kprintf("Read disk error\n");
        return 0;
    }
 
    return 1;
}



int write(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint16_t *buf)
{
    port->is_rwc = (uint32_t)-1;       // Clear pending interrupt bits
    int spin = 0; // Spin lock timeout counter
    int i;
    int slot = find_cmdslot(port);
    if (slot == -1)
        return 0;
 
    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)port->clb;
    cmdheader += slot;
    cmdheader->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t); // Command FIS size
    cmdheader->w = 1;       // Write to device
    cmdheader->prdtl = (uint16_t)((count - 1) >> 1) + 1;    // PRDT entries count
 
    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t*)(cmdheader->ctba);
    // memset(cmdtbl, 0, sizeof(hba_cmd_tbl_t) + (cmdheader->prdtl-1) * sizeof(hba_prdt_entry_t));
 
    // 8K bytes (16 sectors) per PRDT
    for (i = 0; i < cmdheader->prdtl - 1; i++)
    {
        cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
        cmdtbl->prdt_entry[i].dbc = 8 * 1024; // 8K bytes
        cmdtbl->prdt_entry[i].i = 1;
        buf += 4 * 1024;  // 4K words
        count -= 2;    // 16 sectors
    }

    // Last entry
    cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
    cmdtbl->prdt_entry[i].dbc = count << 12;   // 512 bytes per sector
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
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000)
    {
        kprintf("Port is hung\n");
        return 0;
    }
 
    port->ci = 1<<slot; // Issue command
 
    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit 
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1 << slot)) == 0) 
            break;
        if (port->is_rwc & HBA_PxIS_TFES)   // Task file error
        {
            kprintf("Read disk error\n");
            return 0;
        }
    }
 
    // Check again
    if (port->is_rwc & HBA_PxIS_TFES)
    {
        kprintf("Read disk error\n");
        return 0;
    }
 
    return 1;
}



// Start command engine
void start_cmd(hba_port_t *port)
{
    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR);
 
    // Set FRE (bit4) and ST (bit0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST; 
}
 
// Stop command engine
void stop_cmd(hba_port_t *port)
{
    // Clear ST (bit0)
    port->cmd &= ~HBA_PxCMD_ST;

    // // Clear FRE (bit4)
    // port->cmd &= ~HBA_PxCMD_FRE;


    // Wait until FR (bit14), CR (bit15) are cleared
    while(1) {
        if (port->cmd & HBA_PxCMD_FR)
        {
            continue;
        }
        if (port->cmd & HBA_PxCMD_CR) {
            continue;
        }
        break;
    }

    // Clear FRE (bit4)
    port->cmd &= ~HBA_PxCMD_FRE;
}
 
void port_rebase(hba_port_t *port, int portno)
{
    // stop_cmd(port); // Stop command engine
 
    // Command list offset: 1K*portno
    // Command list entry size = 32
    // Command list entry maxim count = 32
    // Command list maxim size = 32*32 = 1K per port
    port->clb = AHCI_BASE + (portno<<10);
    memset((void*)(port->clb), 0, 1024);
 
    // FIS offset: 32K+256*portno
    // FIS entry size = 256 bytes per port
    port->fb = AHCI_BASE + (32<<10) + (portno<<8);
    memset((void*)(port->fb), 0, 256);
 
    // Command table offset: 40K + 8K*portno
    // Command table size = 256*32 = 8K per port
    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)(port->clb);
    for (int i=0; i<32; i++)
    {
        cmdheader[i].prdtl = 8; // 8 prdt entries per command table
                    // 256 bytes per command table, 64+16+48+16*8
        // Command table offset: 40K + 8K*portno + cmdheader_index*256
        cmdheader[i].ctba = AHCI_BASE + (40<<10) + (portno<<13) + (i<<8);
        memset((void*)cmdheader[i].ctba, 0, 256);
    }
 
    // start_cmd(port);    // Start command engine
}
 

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
    uint16_t buf[4096] = {0};

    int i = 0, j;
    while (i < 32) {
        if (pi & 1) {
            int dt = check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA) {
                kprintf("SATA drive found at port %d\n", i);
                port_rebase(&abar->ports[i], i);
                read(&abar->ports[i], 0, 0, 1, buf);

                kprintf("Read from disk: ");
                for (j = 0; j < 4096; j++)
                    kprintf("%d ", buf[j]);
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
