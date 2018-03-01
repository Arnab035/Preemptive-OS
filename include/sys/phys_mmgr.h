// This is the header file for phys_mmgr.c

#ifndef  __PHYS_MMGR_H
#define  __PHYS_MMGR_H

#include <sys/defs.h>
#include <sys/kprintf.h>
#include <sys/mmgr.h>
#include <sys/idt.h>
#include <sys/tarfs.h>
#include <sys/vfs.h>

#define PAGE_BLOCK_SIZE  4096

typedef struct page
{
  uint64_t phys_addr_of_page;
  uint32_t reference_count; // how many processes point to the page
  uint16_t is_free_page;
}page;

page *free_page_list;

extern uint64_t number_of_pages;

void set_free_page_list(uint64_t bit_num);

void clear_free_page_list(uint64_t bit_num);

uint32_t test_free_page_bit(uint64_t bit_num);

uint64_t phys_addr_of_first_free_page();

/* instead of allocating physical addresses directly, we will allocate pages in the form of struct page now, from the struct page we can get the physical address of the page */

/*
struct page *alloc_page();

void free_page(struct page *free_page);   
*/

void *phys_mem_alloc_more(int );

int get_index_of_first_free_page();

struct page get_page_from_phys_addr(uint64_t );

/* phys_mem_alloc() will probably be deprecated later */
void *phys_mem_alloc();

void initialize_free_list(void *physbase);

void define_free_list(void *physbase, void *physfree);

void phys_mem_free(uint64_t );  // TODO: implement properly

void increment_reference_count_of_page(uint64_t phys_addr);

void initialize_dentry_cache();

void decrement_reference_count_of_page(struct page );

#endif
