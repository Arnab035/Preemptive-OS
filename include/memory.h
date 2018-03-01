// include memory.h //
#ifndef __MEMORY_H
#define __MEMORY_H

#include <sys/defs.h>

int do_stack_vma(struct mm_struct *, char **, char **);

int do_heap_vma(struct mm_struct *);

int expand_stack(struct mm_struct *, uint64_t );

struct vm_map_entry *find_vma(struct mm_struct *, uint64_t);

void insert_vma(struct mm_struct *, struct vm_map_entry *, struct vm_map_entry *);

int is_overlapping_vma(struct vm_map_entry *, struct vm_map_entry *);

struct vm_map_entry *find_vma(struct mm_struct *, uint64_t );

int find_vma_prev(struct vm_map_entry *, struct vm_map_entry *, struct vm_map_entry *);

int add_vma(struct mm_struct *, struct vm_map_entry *);

uint64_t tarfs_mmap(struct file *, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

#endif
