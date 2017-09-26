#ifndef __PCI_H
#define __PCI_H

#include <sys/defs.h>

typedef struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t bus;
    uint8_t slot;
}__attribute__((__packed__)) device_info;

uint16_t pci_read_word (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint16_t get_vendor_id(uint8_t bus, uint8_t device);
void init_pci();
uint64_t pci_read_bar (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
device_info get_device_info(uint8_t bus, uint8_t slot);
uint32_t pci_remap_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t remap_address);

#endif
