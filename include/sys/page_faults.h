#ifndef __PAGE_FAULTS_H
#define __PAGE_FAULTS_H

#include <sys/defs.h>
#include <sys/process.h>
#include <sys/signal.h>
#include <sys/common.h>
#include <sys/mmgr.h>

#define PG_FAULT_PRESENT  0x1
#define PG_FAULT_WRITE    0x2
#define PG_FAULT_USER     0x4
#define PG_FAULT_RESERVED 0x8


struct reg_list *regs;
struct vm_map_entry *vma;
struct mm_struct *mm;

void i86_initialize_page_fault_handler();

void i86_page_fault_handler();

void page_fault_handler(struct reg_list *, int );

void page_fault_handler_wrapper();

struct reg_list *copy_regs(struct reg_list *);

void do_page_fault(struct reg_list *, int, uint64_t);

int do_copy_on_write_fault(struct mm_struct *, struct vm_map_entry *, uint64_t );

int do_anon_fault(struct mm_struct *, struct vm_map_entry *, uint64_t );

int do_file_fault(struct mm_struct *, struct vm_map_entry *, uint64_t );



#endif
