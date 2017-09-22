#ifndef __PCI_H
#define __PCI_H

#include <sys/defs.h>

uint16_t pci_read_word (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t get_vendor_id(uint8_t bus, uint8_t device);
void check_all_buses();
uint32_t pci_read_bar (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);


#endif