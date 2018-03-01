// file name : common.c 

#include <sys/common.h>
#include <sys/pic.h>
#include <sys/defs.h>
#include <sys/pit.h>
#include <sys/idt.h>
#include <sys/keyboard.h>

// initialize will initialize everything in the pic and pit
void initialize()
{
  // start initializing pic -- I will define the IDTs to start from 32
  // if master starts from 0x20 then slave will start from 0x28
  i86_initialize_pic(0x20, 0x28);
  
  // initialize pit now and also start the counter
  i86_initialize_pit();
  i86_start_counter();
  i86_initialize_keyboard_interrupt();
  //enable_all_interrupts();
}

void set_int_vect(unsigned char int_num, void ( *f)())
{
  set_idt_func(int_num, (uint64_t)f, 0x08 /*offset into GDT where you have kern code seg*/, 0x8E);
}


void set_int_vect_ring3(unsigned char int_num, void ( *f)())
{
  // check for the descriptor flags in the access flags please!
  set_idt_func(int_num, (uint64_t)f, 0x08 /*offset into GDT where you have user code seq*/, 0xEE);
} 

// enable and disable interrupts

void enable_all_interrupts()
{
  __asm__ __volatile__("sti":::"memory");
}

void disable_all_interrupts()
{
  __asm__ __volatile__("cli":::"memory");
}

// from Piazza post: install default interrupt handlers for interrupt numbers 0-31

void set_default_interrupts()
{
  /*
  set_int_vect(0, default_handler);
  set_int_vect(1, default_handler);
  set_int_vect(2, default_handler);
  set_int_vect(3, default_handler);
  set_int_vect(4, default_handler);
  set_int_vect(5, default_handler);
  set_int_vect(6, default_handler);
  set_int_vect(7, default_handler);
  set_int_vect(8, default_handler);
  set_int_vect(9, default_handler);
  set_int_vect(10, default_handler);
  set_int_vect(11, default_handler);
  set_int_vect(12, default_handler);
  set_int_vect(13, default_handler);
  set_int_vect(14, default_handler);
  set_int_vect(15, default_handler);
  set_int_vect(16, default_handler);
  set_int_vect(17, default_handler);
  set_int_vect(18, default_handler);
  set_int_vect(19, default_handler);
  
  set_int_vect(20, default_handler);
  set_int_vect(21, default_handler);
  set_int_vect(22, default_handler);
  set_int_vect(23, default_handler);
  set_int_vect(24, default_handler);
  set_int_vect(25, default_handler);
  set_int_vect(26, default_handler);
  set_int_vect(27, default_handler);
  set_int_vect(28, default_handler);
  set_int_vect(29, default_handler);
  set_int_vect(30, default_handler);
  set_int_vect(31, default_handler);*/
  int i;
  for(i = 20; i <= 255; i++){
    set_int_vect(i, default_handler);
  }

}

// default interrupt handler which just calls nop instruction

void default_handler(){
  __asm__ __volatile__(
    "push %%rax\n\t"
    "push %%rbx\n\t"
    "push %%rcx\n\t"
    "push %%rdx\n\t"
    "push %%rbp\n\t"
    "push %%rdi\n\t"
    "push %%rsi\n\t"
    "push %%r8\n\t"
    "push %%r9\n\t"
    "push %%r10\n\t"
    "push %%r11\n\t"
    "push %%r12\n\t"
    "push %%r13\n\t"
    "push %%r14\n\t"
    "push %%r15\n\t"
    "cld\n\t"
    "nop\n\t"
    "pop %%r15\n\t"
    "pop %%r14\n\t"
    "pop %%r13\n\t"
    "pop %%r12\n\t"
    "pop %%r11\n\t"
    "pop %%r10\n\t"
    "pop %%r9\n\t"
    "pop %%r8\n\t"
    "pop %%rsi\n\t"
    "pop %%rdi\n\t"
    "pop %%rbp\n\t"
    "pop %%rdx\n\t"
    "pop %%rcx\n\t"
    "pop %%rbx\n\t"
    "pop %%rax\n\t"
    "iretq\n\t"
    : : :"memory"
  );
}
