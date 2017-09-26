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

#define AHCI_BASE               0x400000    // 4M

#define ATA_DEV_BUSY            0x80
#define ATA_DEV_DRQ             0x08
#define ATA_CMD_READ_DMA_EX     0x25
#define ATA_CMD_WRITE_DMA_EX    0x35
#define ATA_DEV_BUSY            0x80
#define ATA_DEV_DRQ             0x08

#define NUM_BLOCKS              100
#define BLOCK_SIZE              4096      // 4KB

// AHCI Base Memory Register
hba_mem_t *abar;

// Find a free command list slot
int find_cmdslot(hba_port_t *port)
{
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


int ahci_read_write(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint8_t *buf, int write) {
    // Clear pending interrupt bits
    port->is_rwc = (uint32_t)-1;

    // Spin lock timeout counter
    int spin = 0, i, slot = find_cmdslot(port);
    if (slot == -1)
        return 0;

    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)port->clb;
    cmdheader += slot;

    // Command FIS size
    cmdheader->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t);
    cmdheader->w = write;
    cmdheader->p = 1;
    cmdheader->c = 1;

    // PRDT entries count (8 sectors of 512 bytes each per 4 KB prdt)
    cmdheader->prdtl = (uint16_t)((count - 1) >> 3) + 1;

    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t*)(cmdheader->ctba);
    memset(cmdtbl, 0, sizeof(hba_cmd_tbl_t) + (cmdheader->prdtl - 1) * sizeof(hba_prdt_entry_t));

    // 8K bytes (16 sectors) per PRDT
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

    // Command
    cmdfis->c = 1;
    cmdfis->command = (write == 0) ? ATA_CMD_READ_DMA_EX: ATA_CMD_WRITE_DMA_EX;

    cmdfis->lba0 = (uint8_t)startl;
    cmdfis->lba1 = (uint8_t)(startl >> 8);
    cmdfis->lba2 = (uint8_t)(startl >> 16);

    // LBA mode
    cmdfis->device = 1 << 6;
    cmdfis->control = 1 << 7;

    cmdfis->lba3 = (uint8_t)(startl >> 24);
    cmdfis->lba4 = (uint8_t)starth;
    cmdfis->lba5 = (uint8_t)(starth >> 8);

    cmdfis->count = count;

    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
        spin++;
    if (spin == 1000000) {
        kprintf("Port is hung\n");

        return 0;
    }

    port->ci = 1 << slot; // Issue command

    // Wait for completion
    while (1) {
        // In some longer duration reads, it may be helpful to spin on the DPS bit
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1 << slot)) == 0)
            break;
        if (port->is_rwc & HBA_PxIS_TFES) {  // Task file error
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


// Start command engine
void start_cmd(hba_port_t *port) {
    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR);

    // Set FRE (bit4) and ST (bit0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}

// Stop command engine
void stop_cmd(hba_port_t *port) {
    // Clear ST (bit0)
    port->cmd &= ~HBA_PxCMD_ST;

    // Wait until FR (bit14), CR (bit15) are cleared
    while (1) {
        if (port->cmd & HBA_PxCMD_FR)
            continue;
        if (port->cmd & HBA_PxCMD_CR)
            continue;
        break;
    }

    // Clear FRE (bit4)
    port->cmd &= ~HBA_PxCMD_FRE;
}

void port_rebase(hba_port_t *port, int portno) {
    int i;

    // Set bit0 of Global Host Control to reset AHCI controller, then set bit31 to re-enable AHCI
    abar->ghc |= 0x01;
    abar->ghc |= 0x80000000;
    abar->ghc |= 0x02;

    while ((abar->ghc & 0x01) != 0);

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
    uint32_t ssts = port->ssts;
    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;

    // Check drive status
    if (det != HBA_PORT_DET_PRESENT)
        return AHCI_DEV_NULL;
    if (ipm != HBA_PORT_IPM_ACTIVE)
        return AHCI_DEV_NULL;

    switch (port->sig) {
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
void probe_port() {
    uint32_t pi = abar->pi, dt, i = 0, j, flag = 0;
    uint8_t *write_buffer = (uint8_t *)0x30000, *read_buffer = (uint8_t *)0x9FF000, k;

    while (i < 32) {
        if (pi & 1) {
            dt = check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA) {
            	kprintf("SATA drive found at port %d\n", i);
                port_rebase(&abar->ports[i], i);

                for (k = 0; k < NUM_BLOCKS; k++) {
                    for (j = 0; j < BLOCK_SIZE; j++) {
                        *write_buffer = k;
                        write_buffer++;
                    }
                    write_buffer -= j;

                    ahci_read_write(&abar->ports[i], k * 8, 0, 8, write_buffer, 1);
                }

                for (k = 0; k < NUM_BLOCKS; k++) {
                    ahci_read_write(&abar->ports[i], k * 8, 0, 8, read_buffer, 0);
                    for (j = 0; j < BLOCK_SIZE; j++) {
                        if (read_buffer[j] == k) {
                            flag = 1;
                        } else {
                            // Read and write does not match
                            flag = 0;
                            break;
                        }
                    }

                    if (flag == 0)
                        break;
                }

                if (flag == 1)
                    kprintf("Read and write verified successfully at port %d\n \n", i);
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
        flag = 0;
    }
}

void init_ahci(uint32_t bar5) {
    // Convert Physical address to virtual address
    abar = (hba_mem_t *)((uint64_t)bar5);
    probe_port();
}
