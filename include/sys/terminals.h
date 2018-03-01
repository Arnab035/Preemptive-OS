/* terminals.h implementation */

#ifndef __TERMINALS_H
#define __TERMINALS_H

#include <sys/defs.h>
#include <sys/mmgr.h>
#include <sys/phys_mmgr.h>
#include <sys/vfs.h>
#include <sys/util.h>
#include <sys/scheduler.h>
#include <sys/fs.h>

/* everything comes from the vfs */

/* global input and output queues in the kernel */

struct file *file;

char *input_buffer;

int line_cursor;

void ttyinit();

char *head;

/* should be called when the init_task is being killed */
int terminal_close(struct file *);

ssize_t terminal_read(struct file *, char *, size_t , off_t);

ssize_t terminal_write(struct file *, char *, size_t, off_t);

int is_input_buffer_empty();

int check_input_buffer();

int terminal_open(int, mode_t);

void insert_char_into_input_buffer(char );

void erase_char_from_input_buffer();

void clear_input_buffer();

#endif
