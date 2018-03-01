#ifndef _PAGEFAULTHANDLER_H
#define _PAGEFAULTHANDLER_H

#include <sys/mmgr.h>
#include <sys/defs.h>
#include <sys/common.h>    /* for set_int_vect()  */

void page_fault_handler(uint64_t , uint64_t );

void i86_page_fault_handler();

void i86_initialize_page_fault_handler();

#endif
