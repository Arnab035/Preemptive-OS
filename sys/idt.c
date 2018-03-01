#include <sys/defs.h>
#include <sys/idt.h>

#define MAX_IDT 256   // hardware interrupts

//TODO:// define an array of idt entries

struct idtentry idt[MAX_IDT];
struct idtr_t idtr = { sizeof(idt)-1, (uint64_t)idt };

//TODO:// write a function that handles the entry of an interrupt routine
/* parameters to be passed---  
   IDT entry number, the base address of our Interrupt Service Routine, our Kernel Code Segment, and t   he access flags as outlined in the table introduced above, pass the ist offset as well
*/

void set_idt_func(uint8_t entry, uint64_t base_addr, uint16_t code_seg, uint8_t access){
  idt[entry].offset_low = base_addr & 0xffff;   // fetch low 16 bits
  idt[entry].selector = code_seg;
  idt[entry].ist = 0;                  // in long mode
  idt[entry].type_attr = access;
  idt[entry].offset_mid = (base_addr >> 16) & 0xffff;
  idt[entry].offset_high = (base_addr >> 32) & 0xffffffff;
  idt[entry].reserved = 0;
}

//TODO:// write a function that initializes the idt and loads idtr

void init_idt() {
  my_memset(idt, 0, MAX_IDT * sizeof(struct idtentry));
  
  __asm__ __volatile__("lidt %0":: "m"(idtr):"memory");
}


