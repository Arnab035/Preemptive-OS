#ifndef __COMMON_H
#define __COMMON_H

/* define msr definitions */

#define MSR_STAR    0xC0000081
#define MSR_LSTAR   0xC0000082
#define MSR_CSTAR   0xC0000083
#define MSR_SFMASK  0xC0000084
#define IA32_EFER   0xC0000080

void initialize();

void set_int_vect(unsigned char, void (* )());

void enable_all_interrupts();

void disable_all_interrupts();

void set_default_interrupts();

void default_handler();

#endif
