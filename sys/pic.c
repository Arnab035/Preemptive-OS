/*****************  PIC (programmable interrupt controller) ********************
 
contains all functions that will manipulate the PIC to give us what we want 


******************************************************************************/


#include <sys/pic.h>

/* initialize pic */

// TODO: implement functions

// master irq 0:7 slave irq 8:15
// inb(port number) outb(port number, value)

// This specifies what interrupts are to be ignored, and not acknowledged
// This is an 8 bit register, where each bit determins if an interrupt is disabled or not. If the bit // is 0, it is enabled. If it is a 1, the interrupt device is disabled. 

// inb and outb names copied from osdev

void i86_initialize_pic(uint8_t pic_master_base, uint8_t pic_slave_base){
  uint8_t a1, a2;
  a1 = inb(I86_PIC_MASTER_DATA_REGISTER);
  a2 = inb(I86_PIC_SLAVE_DATA_REGISTER);

  // initialize command words
  outb(I86_PIC_MASTER_COMMAND_REGISTER, INIT_PIC_CMD_WORD);
  outb(I86_PIC_SLAVE_COMMAND_REGISTER, INIT_PIC_CMD_WORD);
  
  // initialize vector offsets
  outb(I86_PIC_MASTER_DATA_REGISTER, pic_master_base);
  outb(I86_PIC_SLAVE_DATA_REGISTER, pic_slave_base);

  // tell how it is wired to master and slaves
  outb(I86_PIC_MASTER_DATA_REGISTER, 4);
  outb(I86_PIC_SLAVE_DATA_REGISTER, 2);

  // give addnl info only mode 8086 info
  outb(I86_PIC_MASTER_DATA_REGISTER, MODE_8086);
  outb(I86_PIC_SLAVE_DATA_REGISTER, MODE_8086);

  // restore saved values
  outb(I86_PIC_MASTER_DATA_REGISTER, a1);
  outb(I86_PIC_SLAVE_DATA_REGISTER, a2);
}

void i86_set_mask(unsigned int irq){
  uint8_t val;
  if(irq > 7){
    val = inb(I86_PIC_SLAVE_DATA_REGISTER);  
    outb(I86_PIC_SLAVE_DATA_REGISTER, val | (1 << (irq-8))); // i do not care about other interrupts
  }
  else{
    val = inb(I86_PIC_MASTER_DATA_REGISTER);
    outb(I86_PIC_MASTER_DATA_REGISTER, val | (1 << irq));
  }
}

void i86_clear_mask(unsigned int irq){
  uint8_t val;
  if(irq > 7){
    val = inb(I86_PIC_SLAVE_DATA_REGISTER);
    outb(I86_PIC_SLAVE_DATA_REGISTER, val & ~(1 << (irq-8)));
  }
  else{
    val = inb(I86_PIC_MASTER_DATA_REGISTER);
    outb(I86_PIC_MASTER_DATA_REGISTER, val & ~(1 << irq)) ;
  }
}

/*
The PIC chip has two interrupt status registers: the In-Service Register (ISR) and the Interrupt Request Register (IRR). The ISR tells us which interrupts are being serviced, meaning IRQs sent to the CPU. The IRR tells us which interrupts have been raised. Based on the interrupt mask (IMR), the PIC will send interrupts from the IRR to the CPU, at which point they are marked in the ISR.

The ISR and IRR can be read via the OCW3 command word. This is a command sent to one of the command ports (0x20 or 0xa0) with bit 3 set. To read the ISR or IRR, write the appropriate command to the command port, and then read the command port (not the data port). To read the IRR, write 0x0a. To read the ISR, write 0x0b. 
*/
// read 16 bits of data from two command regs -- returns in a format slave | master

uint16_t i86_read_status_regs(uint8_t cmd){
  outb(I86_PIC_MASTER_COMMAND_REGISTER, cmd);
  outb(I86_PIC_SLAVE_COMMAND_REGISTER, cmd);
  return ((inb(I86_PIC_SLAVE_COMMAND_REGISTER) << 8) | inb(I86_PIC_MASTER_COMMAND_REGISTER));
}

/*
Perhaps the most common command issued to the PIC chips is the end of interrupt (EOI) command (code 0x20). This is issued to the PIC chips at the end of an IRQ-based interrupt routine. If the IRQ came from the Master PIC, it is sufficient to issue this command only to the Master PIC; however if the IRQ came from the Slave PIC, it is necessary to issue the command to both PIC chips. 
*/

void i86_pic_send_eoi_command(unsigned int irq){
  if(irq > 7)
    outb(I86_PIC_SLAVE_COMMAND_REGISTER, I86_PIC_END_OF_INTERRUPT);
  outb(I86_PIC_MASTER_COMMAND_REGISTER, I86_PIC_END_OF_INTERRUPT);
}


//i86_pic_send_data () and i86_pic_read_data (): Send and return a data byte to or from a PIC

void i86_pic_send_data(uint8_t data, uint8_t picNum){
  if(picNum > 1) return;
  if(picNum == 0) outb(I86_PIC_MASTER_DATA_REGISTER, data);
  else outb(I86_PIC_SLAVE_DATA_REGISTER, data);
}

uint8_t i86_pic_read_data(uint8_t picNum){
  if(picNum > 1) return 0;
  if(picNum == 0) return inb(I86_PIC_MASTER_DATA_REGISTER);
  else return inb(I86_PIC_SLAVE_DATA_REGISTER);
}

