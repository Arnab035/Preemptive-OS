/** include file for all of mmap based functions here **/

#ifndef __MEMORY_H
#define __MEMORY_H

#include <sys/defs.h>
#include <sys/process.h>
#include <sys/elf64.h>
#include <sys/tarfs.h>
#include <sys/page_faults.h>

#define PROT_READ         0x1
#define PROT_WRITE        0x2
#define PROT_EXEC         0x4/*ELF_EXECUTE*/

#define VERIFY_WRITE   0x01

#define MAP_PRIVATE    0x02
#define MAP_ANONYMOUS  0x20
#define MAP_SHARED     0x01

struct vm_map_entry *vma;
struct file *file;
struct mm_struct *mm;

void *sys_mmap(void *, size_t, int, int, int, off_t);

int sys_munmap(void *, size_t);

int do_munmap(struct mm_struct *, void *, size_t );

void deallocate_vma(struct vm_map_entry *);

void free_vmas(struct mm_struct *, struct vm_map_entry *);

void remove_vma(struct mm_struct *, struct vm_map_entry *);

uint64_t get_unmapped_area(struct mm_struct *, uint64_t, size_t );

uint64_t do_first_fit_search(struct vm_map_entry *, uint64_t, size_t);

void *do_mmap(void *, size_t, int, int, int, off_t);

uint64_t mmap_region(struct file *, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

struct vm_map_entry *find_vma_region(struct mm_struct *, uint64_t, size_t);

int add_vma(struct mm_struct *, struct vm_map_entry *);

int find_vma_prev(struct vm_map_entry *, struct vm_map_entry *, struct vm_map_entry **);

int is_overlapping_vma(struct vm_map_entry *, struct vm_map_entry *);

void insert_vma(struct mm_struct *, struct vm_map_entry *, struct vm_map_entry *);

struct vm_map_entry *find_vma(struct mm_struct *, uint64_t);

struct vm_map_entry *find_closest_vma(struct mm_struct *, uint64_t );

int expand_stack(struct mm_struct *, uint64_t);

int do_heap_vma(struct mm_struct *);

int do_stack_vma(struct mm_struct *, const char **, const char **);

void copy_params_to_userspace(const char **, int, int, uint64_t *, char **);

int is_userspace_access_valid(struct mm_struct *, void *, size_t, int );

#endif

