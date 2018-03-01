// handle keyboard interrupts

#include <sys/keyboard.h>
#include <sys/kprintf.h>
#include <sys/pic.h>
#include <sys/common.h>
#include <sys/keyboard.h>

void i86_initialize_keyboard_interrupt(){
  set_int_vect(33, i86_keyboard_wrapper);
}

/* keyboard scancodes here */
// scancode layout copied from osdever uskeyboard

// random ascii  values for lshift and rshift 
// 2- left shift
// 3- right shift
// 4- ctrl

unsigned char scancodes[128] = 
{
  0, 27, '1', '2', '3', '4', '5',
  '6', '7', '8', '9', '0', '-', '=',
  '\b', '\t', 'q', 'w', 'e', 'r',
  't', 'y', 'u', 'i', 'o', 'p', '[',
  ']', '\n', 4,  /*control*/
  'a', 's', 'd', 'f', 'g', 'h', 'j',
  'k', 'l', ';','\'','`', 2, /* left shift*/
  '\\','z', 'x', 'c', 'v', 'b', 'n',
  'm', ',', '.', '/', 3,   /* right shift*/'*',
  0,   /* alt */
  ' ', /* space bar */
  0, /* Caps */
  0, /* 59 - F1 key... > */
  0, 0, 0, 0, 0, 0, 0, 0,
  0, /* <... F10 */
  0, /* 69- Num Lock */
  0, /* Home */
  0, /* Up arrow */
  0, /* Page up*/
 '-',
  0, /* left arrow */
  0,
  0, /* right arrow */
 '+',
  0, /* 79 - end key */
  0, /* Down arrow */
  0, /* Page down */
  0, /* Insert key */
  0, /* Delete key */
  0, 0, 0,
  0,  /* F11 key */
  0, /* F12 key */
  0, /* all other keys */
};

/*
Reading and writing: Input buffer
To send a command, we first wait to insure the keyboard controller is ready for it. This is done by seeing if the input buffer is full or not. We test this by reading the keyboard controllers status register and testing the bit. If its 0, the buffer is empty so we send the command byte to it. (Remember all of this information is inside of the status register bit layout shown above.)
*/

void send_command_to_controller(uint8_t cmd){
  while(1){
    if((inb(KEYBOARD_CONTROLLER_STATUS_REGISTER) & KEYBOARD_INPUT_BUFFER_STATUS) == 0)
      break;
  }
  outb(KEYBOARD_CONTROLLER_COMMAND_REGISTER, cmd);
}

/*
Remember that commands sent to the keyboard encoder are sent to the keyboard controller first. Because of this, you still need to ensure the keyboard controller itself is still ready for the command.*/

void send_command_to_encoder(uint8_t cmd){
  while(1){
    if((inb(KEYBOARD_ENCODER_INPUT_REGISTER) & KEYBOARD_INPUT_BUFFER_STATUS) == 0) break;
  }
  outb(KEYBOARD_ENCODER_COMMAND_REGISTER, cmd);
}


/* keyboard wrapper to be modified */

void i86_keyboard_wrapper(){
  /* nothing to do with registers here -> push in any order you want */
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
    "callq i86_keyboard_irq\n\t"
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
    : : : "memory"
  );
}


/*
The purpose of the interrupt handler is to update the current state of the driver and to decipher the scan code by converting it to a format that can be used by the driver and the system
*/

void i86_keyboard_irq(){
  i86_pic_send_eoi_command(1);
  unsigned char scancode;
  int i;
  //set_x(get_x() + 5);
  //disable_all_interrupts();
  i86_set_mask(0);
  /*
  if(!(is_input_buffer_empty()))  
  {
    // beep//
    goto out;
  } */
  if(KEYBOARD_OUTPUT_BUFFER_STATUS & inb(KEYBOARD_CONTROLLER_STATUS_REGISTER)){
    scancode = inb(KEYBOARD_ENCODER_INPUT_REGISTER);
  
    if(scancode & 0x80){
     scancode -= 0x80;   // break code to make code
     if(scancodes[scancode] == 2 || scancodes[scancode] == 3) // left shift
     {
       _shift = 0;
     }
     if(scancodes[scancode] == 4)
     {
       _ctrl = 0;
     }  
   }
   else 
   {
     if(scancodes[scancode] == 2 || scancodes[scancode] == 3)
     {
       _shift = 1;
     }
     else if(scancodes[scancode] == 4)
     {
       _ctrl = 1;
     }
     else {
      if(_shift) {
       switch(scancodes[scancode]){
         case 'a':
          insert_char_into_input_buffer('A');
          if(!stop_buffer_output)
            kprintf("A"); 
          //kputchar('A');
          break;
        case 'b':
          insert_char_into_input_buffer('B');
          if(!stop_buffer_output)
            kprintf("B"); 
          //kputchar('B');
          break;
        case 'c':
          insert_char_into_input_buffer('C');
          if(!stop_buffer_output) 
            kprintf("C");
          break;
        case 'd':
          insert_char_into_input_buffer('D');
          if(!stop_buffer_output)
            kprintf("D");
          break;
        case 'e':
          insert_char_into_input_buffer('E');
          if(!stop_buffer_output)
            kprintf("E");
          break;
        case 'f':
          insert_char_into_input_buffer('F');
          if(!stop_buffer_output)
            kprintf("F");
          break;
        case 'g':
          insert_char_into_input_buffer('G');
          if(!stop_buffer_output)
            kprintf("G");
          break;
        case 'h':
          insert_char_into_input_buffer('H');
          if(!stop_buffer_output)
            kprintf("H");
          break;
        case 'i':
          insert_char_into_input_buffer('I');
          if(!stop_buffer_output)
            kprintf("I");
          break;
        case 'j':
          insert_char_into_input_buffer('J');
          if(!stop_buffer_output)
            kprintf("J");
          break;
        case 'k':
          insert_char_into_input_buffer('K');
          if(!stop_buffer_output)
            kprintf("K");
          break;
        case 'l':
          insert_char_into_input_buffer('L');
          if(!stop_buffer_output)
            kprintf("L");
          break;
        case 'm':
          insert_char_into_input_buffer('M');
          if(!stop_buffer_output)
            kprintf("M");
          break;
        case 'n':
          insert_char_into_input_buffer('N');
          if(!stop_buffer_output)
            kprintf("N");
          break;
        case 'o':
          insert_char_into_input_buffer('O');
          if(!stop_buffer_output)
            kprintf("O");
          break;
        case 'p':
          insert_char_into_input_buffer('P');
          if(!stop_buffer_output)
            kprintf("P");
          break;
        case 'q':
          insert_char_into_input_buffer('Q');
          if(!stop_buffer_output)
            kprintf("Q");
          break;
        case 'r':
          insert_char_into_input_buffer('R');
          if(!stop_buffer_output)
            kprintf("R");
          break;
        case 's':
          insert_char_into_input_buffer('S');
          if(!stop_buffer_output)
            kprintf("S");
          break;
        case 't':
          insert_char_into_input_buffer('T');
          if(!stop_buffer_output)
            kprintf("T");
          break;
        case 'u':
          insert_char_into_input_buffer('U');
          if(!stop_buffer_output)
            kprintf("U");
          break;
        case 'v':
          insert_char_into_input_buffer('V');
          if(!stop_buffer_output)
            kprintf("V");
          break;
        case 'w':
          insert_char_into_input_buffer('W');
          if(!stop_buffer_output)
            kprintf("W");
          break;
        case 'x':
          insert_char_into_input_buffer('X');
          if(!stop_buffer_output)
            kprintf("X");
          break;
        case 'y':
          insert_char_into_input_buffer('Y');
          if(!stop_buffer_output)
            kprintf("Y");
          break;
        case 'z':
          insert_char_into_input_buffer('Z');
          if(!stop_buffer_output)
            kprintf("Z");
          break;
        case '1':
          insert_char_into_input_buffer('!');
          if(!stop_buffer_output)
            kprintf("!");
          break;
        case '2':
          insert_char_into_input_buffer('@');
          if(!stop_buffer_output)
            kprintf("@");
          break;
        case '3':
          insert_char_into_input_buffer('#');
          if(!stop_buffer_output)
            kprintf("#");
          break;
        case '4':
          insert_char_into_input_buffer('$');
          if(!stop_buffer_output)
            kprintf("$");
          break;
        case '5':
          insert_char_into_input_buffer('%');
          if(!stop_buffer_output)
            kprintf("%");
          break;
        case '6':
          insert_char_into_input_buffer('^');
          if(!stop_buffer_output) 
            kprintf("^");
          break;
        case '7':
          insert_char_into_input_buffer('&');
          if(!stop_buffer_output) 
            kprintf("&");
          break;
        case '8':
          insert_char_into_input_buffer('*');
          if(!stop_buffer_output) 
            kprintf("*");
          break;
        case '9':
          insert_char_into_input_buffer('(');
          if(!stop_buffer_output) 
            kprintf("(");
          break;
        case '0':
          insert_char_into_input_buffer(')');
          if(!stop_buffer_output) 
            kprintf(")");
          break;
        default:
          break;
      } }
      //ctrl
      else if(_ctrl){
       switch(scancodes[scancode]){
         case 'a':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('A');
           if(!stop_buffer_output) 
             kprintf("^A");
           break;
         case 'b':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('B');
           if(!stop_buffer_output) 
             kprintf("^B");
           break;
         case 'c':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('C');
           if(!stop_buffer_output) 
             kprintf("^C");
           break;
         case 'd':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('D');
           if(!stop_buffer_output) 
             kprintf("^D");
           break;
         case 'e':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('E');
           if(!stop_buffer_output) 
             kprintf("^E");
           break;
         case 'f':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('F');
           if(!stop_buffer_output) 
             kprintf("^F");
           break;
         case 'g':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('G');
           if(!stop_buffer_output) 
             kprintf("^G");
           break;
         case 'h':
           /* backspace - ctrl + H */
           erase_char_from_input_buffer();
           set_x(get_x() - 1);
           if(!stop_buffer_output) 
             kprintf(" ");
           set_x(get_x() - 1);
           break;
         case 'i':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('I');
           if(!stop_buffer_output) 
             kprintf("^I");
           break;
         case 'j':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('J');
           if(!stop_buffer_output) 
             kprintf("^J");
           break;
         case 'k':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('K');
           if(!stop_buffer_output) 
             kprintf("^K");
           break;
         case 'l':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('L');
           if(!stop_buffer_output) 
             kprintf("^L");
           break;
         case 'm':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('M');
           if(!stop_buffer_output) 
             kprintf("^M");
           break;
         case 'n':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('N');
           if(!stop_buffer_output) 
             kprintf("^N");
           break;
         case 'o':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('O');
           if(!stop_buffer_output) 
             kprintf("^O");
           break;
         case 'p':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('P');
           if(!stop_buffer_output) 
             kprintf("^P");
           break;
         case 'q':
           /* first clear the stopped buffered output */
           for(i = line_cursor; i >= 0; i--)
           {
             set_x(get_x()-1);
             kprintf(" ");
             set_x(get_x()-1);
           }
           /* start stopped buffer output */
           if(stop_buffer_output)
           {
             stop_buffer_output = 0;
             for(i = 0; i < line_cursor; i++)
             {
               kputchar(input_buffer[i]);
             } 
           }
           else
           {
             insert_char_into_input_buffer('^');
             insert_char_into_input_buffer('Q');
             kprintf("^Q");
           }
           break;
         case 'r':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('R');
           if(!stop_buffer_output) 
             kprintf("^R");
           break;
         case 's':
           clear_input_buffer();
           stop_buffer_output = 1;
           break;
         case 't':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('T');
           if(!stop_buffer_output) 
             kprintf("^T");
           break;
         case 'u':
           /* kill-clear buffer */
           for(i = line_cursor; i >= 0; i--)
           {
             /* do a backspace */
             set_x(get_x() - 1);
             kprintf(" ");
             set_x(get_x() - 1);
           }
           clear_input_buffer();
           //kprintf("^U");
           break;
         case 'v':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('V');
           if(!stop_buffer_output) 
             kprintf("^V");
           break;
         case 'w':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('W');
           if(!stop_buffer_output) 
             kprintf("^W");
           break;
         case 'x':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('X');
           if(!stop_buffer_output) 
             kprintf("^X");
           break;
         case 'y':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('Y');
           if(!stop_buffer_output) 
             kprintf("^Y");
           break;
         case 'z':
           insert_char_into_input_buffer('^');
           insert_char_into_input_buffer('Z');
           if(!stop_buffer_output) 
             kprintf("^Z");
           break;
         default:
           break;
      }   
     }
     else
     {
       insert_char_into_input_buffer(scancodes[scancode]);
       if(!stop_buffer_output) 
         kputchar(scancodes[scancode]);
     }
   }
  }
 }
  //out:
  //i86_pic_send_eoi_command(1);
  i86_clear_mask(0);
  //enable_all_interrupts();

}

