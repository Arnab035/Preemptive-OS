#ifndef _MMGR_H
#define _MMGR_H

#include <sys/defs.h>
#include <sys/phys_mmgr.h>

#define RECURSIVE_INDEX 510

#define KERNBASE 0xFFFFFFFF80000000

extern int is_paging;

typedef enum {
  PMLE_PRESENT = 0x1,
  PMLE_READWRITE = 0x2,
  PMLE_USERSUPERVISOR = 0x4,
  PMLE_WRITETHROUGH = 0x8,
  PMLE_CACHEDISABLE = 0x10,
  PMLE_ACCESSED = 0x20,
  PMLE_PDPTE_ADDRESS = 0x000FFFFFFFFFF000,  // 40 bits from 51-12
}pmle_flags;

typedef enum {
  PDPTE_PRESENT = 0x1,
  PDPTE_READWRITE = 0x2,
  PDPTE_USERSUPERVISOR = 0x4,
  PDPTE_WRITETHROUGH = 0x8,
  PDPTE_CACHEDISABLE = 0x10,
  PDPTE_ACCESSED = 0x20,
  PDPTE_PAGE_SIZE = 0x80,  // detect if page is 1gb page
  PDPTE_PDT_ADDRESS = 0x000FFFFFFFFFF000,
}pdpte_flags;

typedef enum {
  PDE_PRESENT = 0x1,
  PDE_READWRITE = 0x2,
  PDE_USERSUPERVISOR = 0x4,
  PDE_WRITETHROUGH = 0x8,
  PDE_CACHEDISABLE = 0x10,
  PDE_ACCESSED = 0x20,
  PDE_PAGE_SIZE = 0x80,  // detect if page is a 2mb page
  PDE_PT_ADDRESS = 0x000FFFFFFFFFF000,
}pde_flags;

typedef enum {
  PTE_PRESENT = 0x1,
  PTE_READWRITE = 0x2,
  PTE_USERSUPERVISOR = 0x4,
  PTE_WRITETHROUGH = 0x8,
  PTE_CACHEDISABLE = 0x10,
  PTE_ACCESSED = 0x20,
  PTE_DIRTY = 0x40,
  PTE_PAT = 0x80,
  PTE_PHYS_ADDR = 0x000FFFFFFFFFF000,
}pte_flags;

typedef struct{
  uint64_t pte[512];
}page_table;

typedef struct{
  uint64_t pde[512];
}page_directory;

typedef struct{
  uint64_t pdpte[512];
}page_directory_pointer_table;

typedef struct{
  uint64_t pml4e[512];
}pml_table;

pml_table *pml4;

void set_pml4(uint64_t phys_addr);

void *get_pml4();

struct page find_page_from_virt(uint64_t, uint64_t);

void init_setup_virtual_memory();

void set_physical_address_of_pdpt_to_pml(uint64_t *, page_directory_pointer_table *);

void set_physical_address_of_pdt_to_pdpt(uint64_t *, page_directory *);

void set_physical_address_of_pt_to_pdt(uint64_t *,page_table *);

void *get_physical_address_of_pdpt_from_pml(uint64_t *);

void *get_physical_address_of_pdt_from_pdpt(uint64_t *);

void *get_physical_address_of_pt_from_pdt(uint64_t *);

void *get_physical_address_of_page_from_pt(uint64_t *);

void map_virtual_address_to_physical_address(void *,void *, unsigned int, uint64_t, int);

int change_flags_in_parent_page_tables(uint64_t, uint64_t, uint64_t);

uint64_t walk_pml4(uint64_t, uint64_t);

void free_page_range(int, uint64_t);

uint64_t copy_page_tables(uint64_t, int);

uint64_t get_kpa(void *);

#endif
