// programmable interval timer --- include

#include <sys/defs.h>
#include <sys/process.h>
#include <sys/scheduler.h>

#define I86_PIT_DATA_REGISTER_0    0x40
#define I86_PIT_DATA_REGISTER_1    0x41
#define I86_PIT_DATA_REGISTER_2    0x42
#define I86_PIT_COMMAND_REGISTER   0x43
#define INIT_PIT_CMD_WORD          0x34  // makes it a rate generator

extern uint32_t freq;

extern unsigned long sleep_shell_time; 

void i86_initialize_pit();

void i86_start_counter();

void i86_pit_wrapper();

void i86_pit_irq();

void i86_pit_send_data(uint16_t, uint8_t);

uint16_t i86_pit_read_data(uint8_t );

uint64_t get_number_of_ticks_since_boot();

void sys_sleep(unsigned long );
