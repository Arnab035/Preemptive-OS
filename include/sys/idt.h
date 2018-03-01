
// include idt.h -- interrupt descriptor table header

#ifndef __IDT_H
#define __IDT_H

#include <sys/defs.h>
#include <sys/util.h>

struct idtentry{
  uint16_t offset_low;
  uint16_t selector;
  uint8_t ist;
  uint8_t type_attr;
  uint16_t offset_mid;
  uint32_t offset_high;
  uint32_t reserved;
}__attribute__((packed));


struct idtr_t{
  uint16_t limit;
  uint64_t base;
}__attribute__((packed));

void init_idt();

void set_idt_func(uint8_t, uint64_t, uint16_t, uint8_t);


#endif
