#include <sys/defs.h>

// header file for pic.h

#define I86_PIC_MASTER_COMMAND_REGISTER    0x20
#define I86_PIC_SLAVE_COMMAND_REGISTER     0xA0
#define I86_PIC_MASTER_DATA_REGISTER       0x21
#define I86_PIC_SLAVE_DATA_REGISTER        0xA1
// Normally, reading from the data port returns the IMR register (see above), and writing to it sets the register.
#define I86_PIC_MASTER_MASK_REGISTER       0x21
#define I86_PIC_SLAVE_MASK_REGISTER        0xA1

// ALL INTERRUPTS

#define TIMER_INTERRUPT                    0
#define KEYBOARD_INTERRUPT                 1

// COMMAND WORD 2

#define I86_PIC_END_OF_INTERRUPT           0x20
#define I86_PIC_SL                         0x40    // 01000000 -bit 6- select
#define I86_PIC_INT_L1                     0x1
#define I86_PIC_INT_L2                     0x2
#define I86_PIC_INT_L3                     0x4
#define I86_PIC_ROTATE                     0x80

/*
When you enter protected mode (or even before hand, if you're not using GRUB) the first command you will need to give the two PICs is the initialise command (code 0x11). This command makes the PIC wait for 3 extra "initialisation words" on the data port. These bytes give the PIC:

    Its vector offset. (ICW2)
    Tell it how it is wired to master/slaves. (ICW3)
    Gives additional information about the environment. (ICW4) 
*/

#define INIT_PIC_CMD_WORD                 0x11
#define MODE_8086                         0x01


// todo: write function definitions

void i86_initialize_pic(uint8_t , uint8_t );

void i86_set_mask(unsigned int );

void i86_clear_mask(unsigned int);

uint16_t i86_read_status_regs(uint8_t );

void i86_pic_send_data(uint8_t , uint8_t );

uint8_t i86_pic_read_data(uint8_t );

// alert: please use to send end of interrupt
void i86_pic_send_eoi_command(unsigned int );

// definition of outb and inb use inline asm

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ __volatile__ ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ __volatile__ ( "inb %1, %0": "=a"(ret) : "Nd"(port) );
    return ret;
}


