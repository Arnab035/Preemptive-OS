/* function definitions of utility functions */
#ifndef __UTIL_H
#define __UTIL_H

#include <sys/defs.h>
#include <sys/phys_mmgr.h>
#include <sys/mmgr.h>
#include <sys/process.h>

/* stack frame layout helper functions - layout of struct reg_list on kstack top */

/* this idea (in part) has been inherited from linux kernel source code */

/* callee preserved registers - usually not needed on kernel entry */

#define R15  0*8
#define R14  1*8
#define R13  2*8
#define R12  3*8
#define RBP  4*8
#define RBX  5*8

/* callee-clobbered registers - always needs to be saved on kernel entry */

#define R11  6*8
#define R10  7*8
#define R9   8*8
#define R8   9*8
#define RAX  10*8
#define RCX  11*8
#define RDX  12*8
#define RSI  13*8
#define RDI  14*8

/* error code/syscall num/irq num */
#define ORIG_RAX 15*8
/* Return frame for iretq */
#define RIP		16*8
#define CS		17*8
#define EFLAGS		18*8
#define RSP		19*8
#define SS		20*8

uint32_t last_pid;

int my_memcmp(void *, void *, int);

void my_memset(void *, int , size_t);

uint16_t get_pid();

uint64_t page_round_up(uint64_t );

uint64_t page_round_down(uint64_t );

uint64_t read_cr3();

uint64_t read_cr2();

uint64_t oct2dec(unsigned char *, uint8_t );

uint64_t page_mask(uint64_t );

void my_memcpy(void *, void *, int);

int my_strlen(const char *);

char *my_strchr(char *, int );

void do_wrmsr(uint32_t, uint32_t, uint32_t);

char *my_strtok(char *, char *);

char *my_strcat(char *, char *);

void do_rdmsr(uint32_t, uint32_t *, uint32_t *);

#endif
