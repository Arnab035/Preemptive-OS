#ifndef _PCI_H
#define _PCI_H

#include <sys/defs.h>

#define ENABLE 0x80000000
#define PCI_CONFIG_ADDRESS 0x0CF8
#define PCI_CONFIG_DATA    0x0CFC

void check_all_buses();  // can be improved

void check_device(uint8_t, uint8_t);

uint8_t check_header(uint8_t, uint8_t, uint8_t, uint8_t);

void check_function(uint8_t, uint8_t, uint8_t);


uint8_t get_base_class(uint8_t ,uint8_t ,uint8_t, uint8_t);

uint8_t get_sub_class(uint8_t, uint8_t, uint8_t, uint8_t);

uint8_t get_prog_if(uint8_t ,uint8_t ,uint8_t, uint8_t);

uint16_t check_vendor_id(uint8_t, uint8_t, uint8_t);

uint16_t read_from_config_address_register(uint8_t, uint8_t, uint8_t, uint8_t);

uint32_t read_bar(uint8_t, uint8_t, uint8_t, uint8_t);

void write_bar(uint8_t, uint8_t, uint8_t, uint8_t, uint32_t);

static inline void outbig(uint16_t reg, uint32_t val)
{
  __asm__ __volatile__ ( "outl %0, %1" : : "a"(val), "Nd"(reg) );
}


static inline uint32_t inbig(uint16_t reg)
{
  uint32_t ret;
  __asm__ __volatile__ ("inl %1, %0" : "=a"(ret) :  "Nd"(reg));
  return ret;
}

#endif
