#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include <sys/defs.h>
#include <sys/terminals.h>
#include <sys/kprintf.h>

#define KEYBOARD_CONTROLLER_COMMAND_REGISTER     0x64
#define KEYBOARD_CONTROLLER_STATUS_REGISTER      0x64
#define KEYBOARD_INPUT_BUFFER_STATUS             2
#define KEYBOARD_ENCODER_INPUT_REGISTER          0x60
#define KEYBOARD_ENCODER_COMMAND_REGISTER        0x60
#define KEYBOARD_OUTPUT_BUFFER_STATUS            1

unsigned int _shift, _ctrl;  // globals

unsigned int stop_buffer_output;

void i86_initialize_keyboard_interrupt();

void send_command_to_controller(uint8_t );  // backlight LEDs

void send_command_to_encoder(uint8_t );

void i86_keyboard_wrapper();

void i86_keyboard_irq();

#endif
