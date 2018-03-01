#include "sys/mmgr.h"
#include "sys/kprintf.h"
#include "sys/idt.h"

/*************  Modified virtual memory manager (64 bit) ***************/

/*
  *******  The structure of a linear address looks like this now *******
  
  [PML4] -----  [DIRECTORY PTR] -----  [DIRECTORY] -----  [TABLE] ---- [OFFSET]
   47-39            38-30                 29-21            20-12         11-0

  // PML4E will have 2^9 = 512 entries now
  // PDPTE will also have 2^9 = 512 entries now
  // PDIRECTORY will also have 512 entries
  // PAGE TABLE will also have 512 entries
  // PAGE OFFSET IS FOR A 4096 BYTE PAGE
*/

int is_paging = 0;

pml_table *pml4 = 0;

void *get_pml4()
{
  // returns cr3 value
  return (void *)pml4;
}

// this means setting your physical address of cr3
void set_pml4(uint64_t phys_addr)
{
  __asm__ __volatile__(
   "movq %0, %%cr3\n\t"
   "movq %%cr0, %%rax\n\t"
   "or  $0x80000001, %%eax\n\t"
   "movq %%rax, %%cr0\n\t"
   : : "r"(phys_addr) : "rax" 
  );
}

// assume MAXPHYSADDR(M) to be 52 for now

uint16_t get_index_of_pml4(uint64_t virt_addr)
{
  return (virt_addr >> 39) & 0x1FF; 
}

uint16_t get_index_of_pdpt(uint64_t virt_addr)
{
  return (virt_addr >> 30) & 0x1FF;
}

uint16_t get_index_of_pdt(uint64_t virt_addr)
{
  return (virt_addr >> 21) & 0x1FF;
}

uint16_t get_index_of_pt(uint64_t virt_addr)
{
  return (virt_addr >> 12) & 0x1FF;
}

/*******************************************  helper functions end  **********************************************/

/*call this function for every process - realized it just now :( - what a fool ! */
/* if first = 1, do not add KERNBASE to address because it is the first task */

void init_setup_virtual_memory()
{ 
  page_table *pt = (page_table *)phys_mem_alloc();
  if(!pt) return;

  my_memset((void *)pt, 0, sizeof(page_table));

  page_table *pt2 = (page_table *)phys_mem_alloc();
  if(!pt2) return;
  
  my_memset((void *)pt2, 0, sizeof(page_table));

  page_table *pt3 = (page_table *)phys_mem_alloc();
  if(!pt3) return;
 
  my_memset((void *)pt3, 0, sizeof(page_table));
  page_directory *pdt = (page_directory *)phys_mem_alloc();
  if(!pdt) return;

  my_memset((void *)pdt, 0, sizeof(page_directory));

  page_directory_pointer_table *pdpt = (page_directory_pointer_table *)phys_mem_alloc();
  if(!pdpt) return;

  my_memset((void *)pdpt, 0, sizeof(page_directory_pointer_table));
  
  pml4 = (pml_table *)phys_mem_alloc();
  if(!pml4) return;

  my_memset((void *)pml4, 0, sizeof(pml_table));

  uint64_t virt, phys;
  int i=0;

  page_table *pt2_virt = (page_table *)(KERNBASE | (uint64_t)pt2);
  page_table *pt3_virt = (page_table *)(KERNBASE | (uint64_t)pt3);
  
  for(virt = 0xFFFFFFFF80200000, phys = 0x200000; i < 512; virt += 4096, phys += 4096, i++)
  {
    uint64_t page = 0;
    page |= PTE_PRESENT;
    page |= PTE_READWRITE;
    page = (page & ~PTE_PHYS_ADDR) | (uint64_t)phys;
    pt->pte[get_index_of_pt((uint64_t)virt)] = page;  
  }
  uint64_t *pde_entry = &pdt->pde[get_index_of_pdt((uint64_t)0xFFFFFFFF80200000)];
  *pde_entry = (*pde_entry & ~PDE_PT_ADDRESS) | (uint64_t)pt ;
  *pde_entry |= PDE_PRESENT;
  *pde_entry |= PDE_READWRITE;
  
  uint64_t *pdpte_entry = &pdpt->pdpte[get_index_of_pdpt((uint64_t)0xFFFFFFFF80200000)];
  *pdpte_entry = (*pdpte_entry & ~PDPTE_PDT_ADDRESS) | (uint64_t)pdt ;
  *pdpte_entry |= PDPTE_PRESENT;
  *pdpte_entry |= PDPTE_READWRITE;
 
  uint64_t *pml4e_entry = &pml4->pml4e[get_index_of_pml4((uint64_t)0xFFFFFFFF80200000)];

  *pml4e_entry = (*pml4e_entry & ~PMLE_PDPTE_ADDRESS) | (uint64_t)pdpt;
  *pml4e_entry |= PMLE_READWRITE;
  *pml4e_entry |= PMLE_PRESENT;
  
  set_pml4((uint64_t)pml4);

  is_paging = 1;
  i = 0;

  page_directory *pdt_virt = (page_directory *)(KERNBASE | (uint64_t)pdt);

  for(virt = 0xFFFFFFFF80000000, phys = 0x0; i < 512; i++, phys += 4096, virt += 4096)
  {
    uint64_t page = 0;
    page |= PTE_PRESENT;
    page |= PTE_READWRITE;
    page = (page & ~PTE_PHYS_ADDR) | (uint64_t)phys;
    pt2_virt->pte[get_index_of_pt((uint64_t)virt)] = page;
  }
  uint64_t *pde_entry2 = &pdt_virt->pde[get_index_of_pdt((uint64_t)0xFFFFFFFF80000000)];
  *pde_entry2 = (*pde_entry2 & ~PDE_PT_ADDRESS) | (uint64_t)pt2 ;
  *pde_entry2 |= PDE_PRESENT;
  *pde_entry2 |= PDE_READWRITE;

  i = 0;

  for(virt = 0xFFFFFFFF80400000, phys = 0x400000; i < 512; i++, phys += 4096, virt += 4096)
  {
    uint64_t page = 0;
    page |= PTE_PRESENT;
    page |= PTE_READWRITE;
    page = (page & ~PTE_PHYS_ADDR) | (uint64_t)phys;
    pt3_virt->pte[get_index_of_pt((uint64_t)virt)] = page;
  }
  uint64_t *pde_entry3 = &pdt_virt->pde[get_index_of_pdt((uint64_t)0xFFFFFFFF80400000)];
  *pde_entry3 = (*pde_entry3 & ~PDE_PT_ADDRESS) | (uint64_t)pt3;
  *pde_entry3 |= PDE_PRESENT;
  *pde_entry3 |= PDE_READWRITE;

  return;
}

void *get_physical_address_of_pdpt_from_pml(uint64_t *pml_entry)
{
  return (void *)(*pml_entry & PMLE_PDPTE_ADDRESS);
}

void *get_physical_address_of_pdt_from_pdpt(uint64_t *pdpt_entry)
{
  return (void *)(*pdpt_entry & PDPTE_PDT_ADDRESS);
}

void *get_physical_address_of_pt_from_pdt(uint64_t *pdt_entry)
{
  return (void *)(*pdt_entry & PDE_PT_ADDRESS);
}

void *get_physical_address_of_page_from_pt(uint64_t *pt_entry)
{
  return (void *)(*pt_entry & PTE_PHYS_ADDR);
}

/* recursively copy the parent process address space- including the kernel memory */
uint64_t copy_page_tables(uint64_t page_table_address, int level)
{
  int i;
  int p_level = level;
  pml_table *copy_pml4 = 0;
  uint64_t kernel_pml4, kernel_pdpt, kernel_pdt, kernel_pt;
  switch(p_level)
  {
    case 4:
      kernel_pml4 = (uint64_t)page_table_address;
      pml_table *kernel_pml4_virt = (pml_table *)((uint64_t)kernel_pml4 | KERNBASE);
      i = 0;

      copy_pml4 = (pml_table *)phys_mem_alloc();
      copy_pml4 = (pml_table *)((uint64_t)copy_pml4 | KERNBASE);

      for(; i < 512; i++)
      {
        copy_pml4->pml4e[i] = kernel_pml4_virt->pml4e[i];
        if(kernel_pml4_virt->pml4e[i] != 0)
        {
          uint64_t *pml4_entry = &kernel_pml4_virt->pml4e[i];
          copy_page_tables((uint64_t)get_physical_address_of_pdpt_from_pml(pml4_entry), 3);
        }
      }
      break;
    case 3:
      kernel_pdpt = (uint64_t)page_table_address;
      page_directory_pointer_table *kernel_pdpt_virt = (page_directory_pointer_table *)((uint64_t)kernel_pdpt | KERNBASE);
      i = 0;
      page_directory_pointer_table *copy_pdpt = (page_directory_pointer_table *)phys_mem_alloc();
      copy_pdpt = (page_directory_pointer_table *)((uint64_t)copy_pdpt | KERNBASE);

      for(; i < 512; i++)
      {
        copy_pdpt->pdpte[i] = kernel_pdpt_virt->pdpte[i];
        if(kernel_pdpt_virt->pdpte[i] != 0)
        {
          uint64_t *pdpte_entry = &kernel_pdpt_virt->pdpte[i];
          copy_page_tables((uint64_t)get_physical_address_of_pdt_from_pdpt(pdpte_entry), 2);
        }
      }
      break;
    case 2:
      kernel_pdt = (uint64_t)page_table_address;
      page_directory *kernel_pdt_virt = (page_directory *)((uint64_t)kernel_pdt | KERNBASE);
      i = 0;
      page_directory *copy_pdt = (page_directory *)phys_mem_alloc();
      copy_pdt = (page_directory *)((uint64_t)copy_pdt | KERNBASE);

      for(; i < 512; i++)
      {
        copy_pdt->pde[i] = kernel_pdt_virt->pde[i];
        if(kernel_pdt_virt->pde[i] != 0)
        {
          uint64_t *pdt_entry = &kernel_pdt_virt->pde[i];
          copy_page_tables((uint64_t)get_physical_address_of_pt_from_pdt(pdt_entry), 1);
        }
      }
      break;
    case 1:
      kernel_pt = (uint64_t)page_table_address;
      page_table *kernel_pt_virt = (page_table *)((uint64_t)kernel_pt | KERNBASE);
      i = 0;
      page_table *copy_pt = (page_table *)phys_mem_alloc();
      copy_pt = (page_table *)((uint64_t)copy_pt | KERNBASE);

      for(; i < 512; i++)
      {
        copy_pt->pte[i] = kernel_pt_virt->pte[i];
      }
      break;
    default:
      kprintf("wrong level\n");
      return -1;
  }
  // no need setting pml4 here 
  return (~(KERNBASE) & (uint64_t)copy_pml4);
}  

struct page find_page_from_virt(uint64_t addr, uint64_t cr3)
{
  uint64_t phys_addr;
  struct page page;

  phys_addr = walk_pml4(cr3, addr);
 
  page = get_page_from_phys_addr(phys_addr);
  return page;   
}

void map_virtual_address_to_physical_address(void *virt_addr, void *phys_addr, unsigned int flags, uint64_t cr3, int is_reference)
{
   if(((uint64_t)virt_addr & 4095) != 0)
   {
     return;
   }
   if(((uint64_t)phys_addr & 4095) != 0)
   {
     return;
   }

   pml_table *pml4 = (pml_table *)cr3; // specific to process
   uint64_t virt;

   virt = (uint64_t)virt_addr;

   pml_table *pml4_virt = (pml_table *)((uint64_t)pml4 | (uint64_t)KERNBASE) ;
   uint64_t  *pml_entry = &pml4_virt->pml4e[get_index_of_pml4(virt)];
   if((*pml_entry & PMLE_PRESENT) != PMLE_PRESENT)    // pml entry not present
   {
     // create a new pdpt table
     page_directory_pointer_table *pdpt = (page_directory_pointer_table *)phys_mem_alloc();
     page_directory_pointer_table *pdpt_virt = (page_directory_pointer_table *)((uint64_t)pdpt | (uint64_t)KERNBASE);

     my_memset(pdpt_virt, 0, sizeof(page_directory_pointer_table));

     uint64_t *pml_entry = &pml4_virt->pml4e[get_index_of_pml4(virt)];

     *pml_entry = (*pml_entry & ~PMLE_PDPTE_ADDRESS) | (uint64_t)pdpt;
     *pml_entry |= flags;
     
   }

   page_directory_pointer_table *pdpt = (page_directory_pointer_table *)get_physical_address_of_pdpt_from_pml(pml_entry);
   page_directory_pointer_table *pdpt_virt = (page_directory_pointer_table *)((uint64_t)pdpt | (uint64_t)KERNBASE);

   uint64_t *pdpt_entry = &pdpt_virt->pdpte[get_index_of_pdpt(virt)];
   if((*pdpt_entry & PDPTE_PRESENT) != PDPTE_PRESENT)
   {
     page_directory *pdt = (page_directory *)phys_mem_alloc();
     page_directory *pdt_virt = (page_directory *)((uint64_t)pdt | (uint64_t)KERNBASE);

     my_memset(pdt_virt, 0, sizeof(page_directory));

     uint64_t *pdpt_entry = &pdpt_virt->pdpte[get_index_of_pdpt(virt)];
     *pdpt_entry = (*pdpt_entry & ~PDPTE_PDT_ADDRESS) | (uint64_t)pdt;
     *pdpt_entry |= flags;
    
   }
   page_directory *pdt = (page_directory *)get_physical_address_of_pdt_from_pdpt(pdpt_entry);
   page_directory *pdt_virt = (page_directory *)((uint64_t)pdt | (uint64_t)KERNBASE);

   uint64_t *pdt_entry = &pdt_virt->pde[get_index_of_pdt(virt)];
   if((*pdt_entry & PDE_PRESENT) != PDE_PRESENT)
   {
       // create a new page table
     page_table *pt = (page_table *)phys_mem_alloc();
     page_table *pt_virt = (page_table *)((uint64_t)pt | (uint64_t)KERNBASE);
     
     my_memset(pt_virt, 0, sizeof(page_table));

     uint64_t *pdt_entry = &pdt_virt->pde[get_index_of_pdt(virt)];
     *pdt_entry = (*pdt_entry & ~PDE_PT_ADDRESS) | (uint64_t)pt;
     
     *pdt_entry |= flags;
     
   }
   page_table *pt = (page_table *)get_physical_address_of_pt_from_pdt(pdt_entry);
   page_table *pt_virt = (page_table *)((uint64_t)pt | (uint64_t)KERNBASE);
   if(!is_reference)
   {
     uint64_t *pt_entry = &pt_virt->pte[get_index_of_pt(virt)];
     *pt_entry = (*pt_entry & ~PTE_PHYS_ADDR) | (uint64_t)phys_addr;
     *pt_entry |= flags;  // userpages, flags is a parameter passed to function
   }
   
   set_pml4((uint64_t)cr3);
}

/* similar to walk_pml4 -> here just change the flags of each entry */
/* not all pages will be mapped so far for the process - make sure you change the flags of only those that are already mapped */

int change_flags_in_parent_page_tables(uint64_t virt, uint64_t cr3, uint64_t flags)
{
  uint64_t pml4 = cr3;

  pml_table *pml4_virt = (pml_table *)((uint64_t)pml4 | KERNBASE); 
  uint64_t *pml_entry = &pml4_virt->pml4e[get_index_of_pml4(virt)];
  if((*pml_entry & PMLE_PRESENT) != PMLE_PRESENT) return 0;
  
  *pml_entry = (uint64_t)*pml_entry | flags;   // changed here

  page_directory_pointer_table *pdpt = (page_directory_pointer_table *)get_physical_address_of_pdpt_from_pml(pml_entry);
  
  page_directory_pointer_table *pdpt_virt = (page_directory_pointer_table *)((uint64_t)pdpt| KERNBASE);

  uint64_t *pdpt_entry = &pdpt_virt->pdpte[get_index_of_pdpt(virt)];
  if((*pdpt_entry & PDPTE_PRESENT) != PDPTE_PRESENT) return 0; 

  *pdpt_entry = (uint64_t)*pdpt_entry | flags;    // changed here

  page_directory *pdt = (page_directory *)get_physical_address_of_pdt_from_pdpt(pdpt_entry);
  page_directory *pdt_virt = (page_directory *)((uint64_t)pdt | (uint64_t)KERNBASE);

  uint64_t *pdt_entry = &pdt_virt->pde[get_index_of_pdt(virt)];
  if((*pdt_entry & PDE_PRESENT) != PDE_PRESENT) return 0;

  *pdt_entry = *pdt_entry | flags;             // changed here
  page_table *pt = (page_table *)get_physical_address_of_pt_from_pdt(pdt_entry);
  page_table *pt_virt = (page_table *)((uint64_t)pt | (uint64_t)KERNBASE);

  uint64_t *pt_entry = &pt_virt->pte[get_index_of_pt(virt)];
  *pt_entry = (uint64_t)*pt_entry | flags;

  set_pml4((uint64_t)cr3);
  return 0;
}

uint64_t walk_pml4(uint64_t cr3, uint64_t virt_addr)
{
  uint64_t pml4 = cr3;
  
  pml_table *pml4_virt = (pml_table *)((uint64_t)pml4 | KERNBASE) ;  
  uint64_t *pml_entry = &pml4_virt->pml4e[get_index_of_pml4(virt_addr)];
  if((*pml_entry & PMLE_PRESENT) != PMLE_PRESENT) return -1; // no point going further..

  page_directory_pointer_table *pdpt = (page_directory_pointer_table *)get_physical_address_of_pdpt_from_pml(pml_entry);
  page_directory_pointer_table *pdpt_virt = (page_directory_pointer_table *)((uint64_t)pdpt | (uint64_t)KERNBASE);

  uint64_t *pdpt_entry = &pdpt_virt->pdpte[get_index_of_pdpt(virt_addr)];
  if((*pdpt_entry & PDPTE_PRESENT) != PDPTE_PRESENT) return -1;
  
  page_directory *pdt = (page_directory *)get_physical_address_of_pdt_from_pdpt(pdpt_entry);
  page_directory *pdt_virt = (page_directory *)((uint64_t)pdt | (uint64_t)KERNBASE);
 
  uint64_t *pdt_entry = &pdt_virt->pde[get_index_of_pdt(virt_addr)];
  if((*pdt_entry & PDE_PRESENT) != PDE_PRESENT) return -1;

  page_table *pt = (page_table *)get_physical_address_of_pt_from_pdt(pdt_entry);
  page_table *pt_virt = (page_table *)((uint64_t)pt | (uint64_t)KERNBASE);

  uint64_t *pt_entry = &pt_virt->pte[get_index_of_pt(virt_addr)];
  uint64_t phys_addr = (uint64_t)get_physical_address_of_page_from_pt(pt_entry);

  return phys_addr;
}

uint64_t get_kpa(void *virt_addr)
{
  return (~(KERNBASE) & (uint64_t)virt_addr);
}

/* use recursion */
void free_page_range(int level, uint64_t pml4)
{
  uint64_t addr = 0;
  int i;
  uint64_t *table_entry;
  pml_table *virt_pml4;
  page_directory_pointer_table *pdpt_virt;
  page_directory *pdt_virt;
  page_table *pt_virt;
  /* top-down approach start from the top level -- move to the bottom */
  for(i = 0; i < 512; i++)
  {
    switch(level)
    {
      case 4:
        virt_pml4 = (pml_table *)((uint64_t)pml4 | KERNBASE);
        table_entry = &virt_pml4->pml4e[i];
        if(*table_entry)
        {
          if((*table_entry & PMLE_PRESENT) == PMLE_PRESENT)
          {
            addr = (uint64_t)get_physical_address_of_pdpt_from_pml(table_entry);
            free_page_range(3, addr);
          }
        }
        break;
      case 3:
        pdpt_virt = (page_directory_pointer_table *)((uint64_t)pml4 | KERNBASE);
        table_entry = &pdpt_virt->pdpte[i];
        if(*table_entry)
        {
          if((*table_entry & PDPTE_PRESENT) == PDPTE_PRESENT)
          {
            addr = (uint64_t)get_physical_address_of_pdt_from_pdpt(table_entry);
            free_page_range(2, addr);
          }
        }
        break;
      case 2:
        pdt_virt = (page_directory *)((uint64_t)pml4 | KERNBASE);
        table_entry = &pdt_virt->pde[i];
        if(*table_entry)
        {
          if((*table_entry & PDE_PRESENT) != PDE_PRESENT)
          {
            addr = (uint64_t)get_physical_address_of_pt_from_pdt(table_entry);
            free_page_range(1, addr);
          }
        }
        break;
      case 1:
        pt_virt = (page_table *)((uint64_t)pml4 | KERNBASE);
        table_entry = &pt_virt->pte[i];
        if(*table_entry)
        {
          if((*table_entry & PTE_PRESENT) != PTE_PRESENT)
          {
            addr = (uint64_t)get_physical_address_of_page_from_pt(table_entry);
            addr = addr | KERNBASE;
            phys_mem_free(addr);
          }
        }
        break;
      default:
        kprintf("wrong level value\n");
        return;
    }
    if(addr) 
      phys_mem_free(addr | KERNBASE); // free the main table once all of its internal entries are done with
  }
  return;
}

