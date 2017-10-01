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

#define NUM_BLOCKS              1
#define BLOCK_SIZE              100      // 4KB

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

int read(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint8_t *buf)
{
    port->is_rwc = (uint32_t)-1;   // Clear pending interrupt bits
    // int spin = 0; // Spin lock timeout counter
    int i;
    int slot = find_cmdslot(port);
    if (slot == -1)
        return 0;

    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)port->clb;
    cmdheader += slot;
    cmdheader->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t); // Command FIS size
    cmdheader->w = 0;       // Read from device
    cmdheader->prdtl = (uint16_t)((count - 1) >> 4) + 1;    // PRDT entries count

    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t*)(cmdheader->ctba);
    memset(cmdtbl, 0, sizeof(hba_cmd_tbl_t) + (cmdheader->prdtl - 1) * sizeof(hba_prdt_entry_t));

    // 8K bytes (16 sectors) per PRDT
    for (i = 0; i < cmdheader->prdtl - 1; i++)
    {
        cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
        cmdtbl->prdt_entry[i].dbc = 8 * 1024; // 8K bytes
        cmdtbl->prdt_entry[i].i = 1;
        buf += 4 * 1024;  // 4K words
        count -= 16;    // 16 sectors
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
    // cmdfis->control = 1 << 7;

    cmdfis->lba3 = (uint8_t)(startl >> 24);
    cmdfis->lba4 = (uint8_t)starth;
    cmdfis->lba5 = (uint8_t)(starth >> 8);

    cmdfis->count = count;

    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) /*&& spin < 1000000*/)
    {
        //spin++;
    }
    /*
    if (spin == 1000000)
    {
        kprintf("Port is hung\n");
        return 0;
    }
    */
    port->ci = 1 << slot; // Issue command

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

    kprintf("%d %d \n\r",port->ci, (1<<slot));

    // Check again
    if (port->is_rwc & HBA_PxIS_TFES)
    {
        kprintf("Read disk error\n");
        return 0;
    }

    kprintf("Read done\n");
    kprintf("Port CI: %x\n", port->ci);
    kprintf("Slot CI: %x\n", 1 << slot);

    return 1;
}


// int write(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint8_t *buf)
// {
//     port->is_rwc = (uint32_t)-1;       // Clear pending interrupt bits
//     int spin = 0; // Spin lock timeout counter
//     int i;
//     int slot = find_cmdslot(port);
//     if (slot == -1)
//         return 0;

//     hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)port->clb;
//     cmdheader += slot;
//     cmdheader->cfl = sizeof(fis_reg_h2d_t) / sizeof(uint32_t); // Command FIS size
//     cmdheader->w = 1;       // Write to device
//     cmdheader->prdtl = (uint16_t)((count - 1) >> 4) + 1;    // PRDT entries count

//     hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t*)(cmdheader->ctba);
//     memset(cmdtbl, 0, sizeof(hba_cmd_tbl_t) + (cmdheader->prdtl-1) * sizeof(hba_prdt_entry_t));

//     // 8K bytes (16 sectors) per PRDT
//     for (i = 0; i < cmdheader->prdtl - 1; i++)
//     {
//         cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
//         cmdtbl->prdt_entry[i].dbc = 8 * 1024; // 8K bytes
//         cmdtbl->prdt_entry[i].i = 1;
//         buf += 4 * 1024;  // 4K words
//         count -= 16;    // 16 sectors
//     }

//     // Last entry
//     cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
//     cmdtbl->prdt_entry[i].dbc = count << 9;   // 512 bytes per sector
//     cmdtbl->prdt_entry[i].i = 1;

//     // Setup command
//     fis_reg_h2d_t *cmdfis = (fis_reg_h2d_t*)(&cmdtbl->cfis);

//     cmdfis->fis_type = FIS_TYPE_REG_H2D;
//     cmdfis->c = 1;  // Command
//     cmdfis->command = ATA_CMD_WRITE_DMA_EX;

//     cmdfis->lba0 = (uint8_t)startl;
//     cmdfis->lba1 = (uint8_t)(startl >> 8);
//     cmdfis->lba2 = (uint8_t)(startl >> 16);
//     cmdfis->device = 1 << 6;  // LBA mode

//     cmdfis->lba3 = (uint8_t)(startl >> 24);
//     cmdfis->lba4 = (uint8_t)starth;
//     cmdfis->lba5 = (uint8_t)(starth >> 8);

//     cmdfis->count = count;

//     // The below loop waits until the port is no longer busy before issuing a new command
//     while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)))// && spin < 1000000)
//     {
//         spin++;
//     }
//     // if (spin == 1000000)
//     // {
//     //     kprintf("Port is hung\n");
//     //     return 0;
//     // }

//     port->ci = 1<<slot; // Issue command

//     // Wait for completion
//     while (1)
//     {
//         // In some longer duration reads, it may be helpful to spin on the DPS bit
//         // in the PxIS port field as well (1 << 5)
//         if ((port->ci & (1 << slot)) == 0) {
//             kprintf("Break ho gaya");
//             break;
//         }
//         if (port->is_rwc & HBA_PxIS_TFES)   // Task file error
//         {
//             kprintf("Read disk error\n");
//             return 0;
//         }
//     }

//     // Check again
//     if (port->is_rwc & HBA_PxIS_TFES)
//     {
//         kprintf("Read disk error\n");
//         return 0;
//     }

//     kprintf("Write done\n");

//     return 1;
// }

// Serial ATA AHCI 1.3.1 Specification (Section 10.4.2)
void port_reset(hba_port_t *port) {
    int spin = 0;
    uint32_t flag;

    // Invoke a COMRESET on the interface and start a re-establishment of Phy layer communications
    // flag = port->sctl;
    // flag &= 0xFFFFF0F0;
    // flag |= 0x00000301;
    // port->sctl = flag;
    port->sctl = 0x301;

    // Wait
    while (spin < 1000000) {
        spin++;
    }

    // Vlearing PxSCTL.DET to 0h; this ensures that at least one COMRESET signal is sent over the interface
    // port->sctl &= 0xFFFFFFF0;
    port->sctl = 0x300;

    // Set Interface Communication Control (ICC)
    flag = port->cmd;
    flag &= 0x0FFFFFFF;
    flag |= 0x10000000;
    port->cmd = flag;

    // Set FIS Receive Enable (FRE)
    port->cmd |= 0x00000008;

    // Check Cold Presence Detection (CPD) is set
    // if (((port->cmd >> 20) & 0x01) == 1) {
        // Set Power On Device (POD)
        port->cmd |= 0x00000004;
    // }

    // Set Spin-Up Device (SUD)
    port->cmd |= 0x00000002;

    // Write all 1s to the PxSERR register to clear any bits that were set as part of the port reset
    port->serr_rwc = 0xFFFFFFFF;
    port->is_rwc = 0xFFFFFFFF;
    // port->ie = 0xFFFFFFFF;

    // Wait for communication to be re-established
    while ((port->ssts & 0x0F) != 3);

    // Write all 1s to the PxSERR register to clear any bits that were set as part of the port reset
    // port->serr_rwc = 0xFFFFFFFF;

    // Transitions to both Partial and Slumber states disabled
    // flag = port->sctl;
    // flag &= 0xFFFFF0FF;
    // flag |= 0x00000300;
    // port->sctl = flag;
}

// Start command engine (Section 3.3.7 Serial ATA AHCI 1.3.1 Specification)
void start_cmd(hba_port_t *port) {
    // uint32_t flag;

    port_reset(port);

    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR);

    // // Set Interface Communication Control (ICC)
    // flag = port->cmd;
    // flag &= 0x0FFFFFFF;
    // flag |= 0x10000000;
    // port->cmd = flag;

    // // Set FIS Receive Enable (FRE)
    // port->cmd |= 0x00000008;

    // // Check Cold Presence Detection (CPD) is set
    // if (((port->cmd >> 20) & 0x01) == 1) {
    //     // Set Power On Device (POD)
    //     port->cmd |= 0x00000004;
    // }

    // // Set Spin-Up Device (SUD)
    // port->cmd |= 0x00000002;

    // Set FRE (bit4) and ST (bit0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}

// Stop command engine
void stop_cmd(hba_port_t *port) {
    // Clear ST (bit0)
    port->cmd &= ~HBA_PxCMD_ST;
    while (port->cmd & HBA_PxCMD_CR);

    // Clear FRE (bit4)
    port->cmd &= ~HBA_PxCMD_FRE;
    while (port->cmd & HBA_PxCMD_FR);

    // // Clear ST (bit0)
    // port->cmd &= ~HBA_PxCMD_ST;

    // // Wait until FR (bit14), CR (bit15) are cleared
    // while(1)
    // {
    //     if (port->cmd & HBA_PxCMD_FR)
    //         continue;
    //     if (port->cmd & HBA_PxCMD_CR)
    //         continue;
    //     break;
    // }

    // // Clear FRE (bit4)
    // port->cmd &= ~HBA_PxCMD_FRE;
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
    // uint32_t ssts = port->ssts;
    // uint8_t ipm = (ssts >> 8) & 0x0F;
    // uint8_t det = ssts & 0x0F;

    // Check drive status
    // if (det != HBA_PORT_DET_PRESENT)
    //     return AHCI_DEV_NULL;
    // if (ipm != HBA_PORT_IPM_ACTIVE)
    //     return AHCI_DEV_NULL;

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

void verify_read_write(uint8_t port) {
    // uint8_t flag = 0, *write_buffer = (uint8_t *)0x30000, *read_buffer = (uint8_t *)0x9FF000;
    uint32_t k, j, i = port;
    uint8_t *buf1 = (uint8_t *)0x400000, *buf2 = (uint8_t *)0x500000;//, *tmp;
    buf1++;
    // int spin = 0;
    // port_rebase(&abar->ports[port], port);

    for (int j = 0; j < 4096; j++) {
        buf2[j] = 0;
    }

    // for (k = 0; k < NUM_BLOCKS; k++) {
    //     for (j = 0; j < BLOCK_SIZE; j++) {
    //         *write_buffer = k;
    //         write_buffer++;
    //     }
    //     write_buffer -= j;

    //     // ahci_read_write(&abar->ports[port], k * 8, 0, 8, write_buffer, 1);
    //     write(&abar->ports[port], k * 8, 0, 8, write_buffer);
    // }

    // for (k = 0; k < NUM_BLOCKS; k++) {
    //     // ahci_read_write(&abar->ports[port], k * 8, 0, 8, read_buffer, 0);
    //     read(&abar->ports[port], k * 8, 0, 8, read_buffer);
    //     for (j = 0; j < BLOCK_SIZE; j++) {
    //         // if (read_buffer[j] == k) {
    //         //     flag = 1;
    //         // } else {
    //         //     // Read and write does not match
    //         //     flag = 0;
    //         //     break;
    //         // }
    //         kprintf("%d ", read_buffer[j]);
    //     }

    //     if (flag == 0)
    //         break;
    // }

    // for (k = 0; k < NUM_BLOCKS; k++) {
    //     tmp = buf1;
    //     for (j = 0; j < BLOCK_SIZE; j++) {
    //         *tmp = k;
    //         tmp++;
    //     }

    //     write(&abar->ports[i], k, 0, 1, buf1);

    //     while (spin < 1000000)
    //         spin++;
    //     spin = 0;
    // }

    for (k = 0; k < NUM_BLOCKS; k++) {
        read(&abar->ports[i], k, 0, 1, buf2);
        kprintf("Read from disk: \n");
        for (j = 0; j < BLOCK_SIZE; j++)
            kprintf("%d ", buf2[j]);
        kprintf("\n");
    }

    // if (flag == 1)
    //     kprintf("Read and write verified successfully at port %d\n \n", port);
}


// Search disk in ports impelemented
void probe_port() {
    uint32_t pi = abar->pi, dt, i = 0;
    uint8_t device_found;

    while (i < 32) {
        device_found = 0;
        if (pi & 1) {
            dt = check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA) {
                kprintf("SATA drive found at port %d\n", i);
                device_found = 1;
            } else if (dt == AHCI_DEV_SATAPI) {
                kprintf("SATAPI drive found at port %d\n", i);
                device_found = 1;
            } else if (dt == AHCI_DEV_SEMB) {
                kprintf("SEMB drive found at port %d\n", i);
                device_found = 1;
            } else if (dt == AHCI_DEV_PM) {
                kprintf("PM drive found at port %d\n", i);
                device_found = 1;
            } else {
                kprintf("No drive found at port %d\n", i);
                device_found = 0;
            }

            if (device_found == 1) {
                port_rebase(&abar->ports[i], i);
                verify_read_write(i);
                // return;
            }
        }
        pi >>= 1;
        i++;
    }
}

void init_ahci(uint32_t bar5) {
    // Convert Physical address to virtual address
    abar = (hba_mem_t *)((uint64_t)bar5);

    // Set bit0 of Global Host Control to reset AHCI controller, then set bit31 to re-enable AHCI
    // abar->ghc |= 0x01;
    // while ((abar->ghc & 0x01) != 0);
    abar->ghc |= 0x80000000;
    // abar->ghc |= 0x02;

    // abar->ghc |= 0x01;
    // while ((abar->ghc & 0x01) != 0);

    // while (spin < 1000000) {
    //     spin++;
    // }

    // if (!(abar->ghc & 0x80000000))
    //     abar->ghc |= 0x80000000;


    probe_port();
}
