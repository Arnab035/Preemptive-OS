/**********  header file for terminals ************/

#include <unistd.h>

char termbuf[ 2 * PAGE_BLOCK_SIZE ];            // terminal buffer has a size of 8192 bytes

char *term_head = termbuf;            // pointers to terminal buffer entries -- these will be updated by the keyboard code
char *term_tail = termbuf;

ssize_t terminal_read(int fd, void *buf, ssize_t count);


ssize_t terminal_write(int fd, void *buf, ssize_t count);
