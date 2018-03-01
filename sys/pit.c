/******************  PIT (Programmable interval timer)  ******************/

#include <sys/pit.h>
#include <sys/pic.h>
#include <sys/idt.h>
#include <sys/common.h>
#include <sys/kprintf.h>
#include <sys/scheduler.h>

uint32_t freq = 1000;
int ticks = 0;

void i86_initialize_pit(){
  // should write to the interrupt descriptor table
  set_int_vect(32, i86_pit_wrapper);
}


void i86_start_counter(){
  // start the counter with frequency freq, square wave and load lsb then msb mode
  if(freq == 0) return;
  outb(I86_PIT_COMMAND_REGISTER, INIT_PIT_CMD_WORD);

  uint16_t val = 1193180 / freq;

  outb(I86_PIT_DATA_REGISTER_0, (val & 0xff));     // load lsb
  outb(I86_PIT_DATA_REGISTER_0, ((val>>8) & 0xff));  // load msb
}

// interrupt wrapper

void i86_pit_wrapper(){

  __asm__ __volatile__(
    "pushq %%rax\n\t"
    "pushq %%rbx\n\t"
    "pushq %%rcx\n\t"
    "pushq %%rdx\n\t"
    "pushq %%rbp\n\t"
    "pushq %%rdi\n\t"
    "pushq %%rsi\n\t"
    "pushq %%r8\n\t"
    "pushq %%r9\n\t"
    "pushq %%r10\n\t"
    "pushq %%r11\n\t"
    "pushq %%r12\n\t"
    "pushq %%r13\n\t"
    "pushq %%r14\n\t"
    "pushq %%r15\n\t"
    "cld\n\t"
    "callq i86_pit_irq\n\t"
    "popq %%r15\n\t"
    "popq %%r14\n\t"
    "popq %%r13\n\t"
    "popq %%r12\n\t"
    "popq %%r11\n\t"
    "popq %%r10\n\t"
    "popq %%r9\n\t"
    "popq %%r8\n\t"
    "popq %%rsi\n\t"
    "popq %%rdi\n\t"
    "popq %%rbp\n\t"
    "popq %%rdx\n\t"
    "popq %%rcx\n\t"
    "popq %%rbx\n\t"
    "popq %%rax\n\t"
    "iretq\n\t"
    : : :"memory"
  );  
}

void i86_pit_irq(){
  i86_set_mask(1);

  // sleeping processes in queue
  i86_pic_send_eoi_command(0);
  struct pcb *process = sleeping_queue.processes;
  while(process != NULL)
  {
    if(process->sleep_time == 0)
    {
      try_to_wake_up(&sleeping_queue, process);
    }
    else
    {
      process->sleep_time = process->sleep_time - 1;
    }
    process = process->next_in_queue;
  }
  
  //kprintf("%d\n", sleep_shell_time);
  //}
  struct pcb *current = get_current_process();
  if(current->is_sigkill_pending) // current is current process
  {
    /* usually involves killing the process- signals are always handled by current process */
    current->is_sigkill_pending = 0;
    sys_exit(9);  // signal code KILL -9
  }
  else if(current->is_sigsegv_pending)
  {
    current->is_sigsegv_pending = 0;
    sys_exit(11);   // signal code : 11
  }
  current->time_slice = current->time_slice - 1;
  if(current->time_slice <= 0)
  {
    schedule();
  }                    
  i86_clear_mask(1);
  
  //enable_all_interrupts();
}

void i86_pit_send_data(uint16_t data, uint8_t counterNum){
  if(counterNum > 2) return;
  if(counterNum == 0) outb(I86_PIT_DATA_REGISTER_0, data);
  else if(counterNum == 1) outb(I86_PIT_DATA_REGISTER_1, data);
  else outb(I86_PIT_DATA_REGISTER_2, data);  
}

uint16_t i86_pit_read_data(uint8_t counterNum){
  if(counterNum > 2) return 0; 
  if(counterNum == 0) return inb(I86_PIT_DATA_REGISTER_0);
  else if(counterNum == 1) return inb(I86_PIT_DATA_REGISTER_1);
  else return inb(I86_PIT_DATA_REGISTER_2); 
}

uint64_t get_number_of_ticks_since_boot(){
  return ticks;
}
