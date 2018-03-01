/*********** C source file to handle page faults in my OS ************/

#include <sys/page_faults.h>

void i86_initialize_page_fault_handler()
{
  set_int_vect(14, i86_page_fault_handler);
}

/* entry point of page fault */

/* this is the actual page fault handler called by the page fault handler wrapper */

void page_fault_handler(struct reg_list *regs, int error_code)
{
  uint64_t address = read_cr2();
  //kprintf("page fault address is 0x%x\n", address);
  do_page_fault(regs, error_code, address);
}

struct reg_list *copy_regs(struct reg_list *regs)
{
  struct pcb *current = get_current_process();
  /* top of the stack contains the register_list struct (in order) */
  struct reg_list *k_regs = task_reg_list(current);
  *k_regs = *regs;
  return k_regs;
}

void do_page_fault(struct reg_list *regs, int error_code, uint64_t address)
{
  struct vm_map_entry *vma;
  struct vm_map_entry *closest_vma;
  int err;
  //kprintf("ALERT: Page Fault at address 0x%p\n", address);
  // check if the page fault is in kernel space
  if(address > KERNBASE && error_code < 4)  /*unsigned int address */
  {
    //kprintf("Fault in the kernel space that is not expected\n");
    return;
  }
  struct pcb *current_process = get_current_process();
  struct mm_struct *mm = current_process->mm;

  vma = find_vma(mm, address);
  if(vma == NULL)
  {
    //kprintf("Address is not in any available vma range-- wrong\n");
    closest_vma = find_closest_vma(mm, address);
    if(!closest_vma)
    {
      if(closest_vma->start_addr == STACK_START)
      {
        /* it is a stack */
        err = expand_stack(mm, address);
        if(err < 0) sys_kill((pid_t)current_process->pid, 11);   // TODO: SEND SIGSEGV signals
      }
    }
    else       
      sys_kill((pid_t)current_process->pid, 11);
  }
  else
  {
    /* align address here - you have lots of virt_to_phys mappings inside the below fns */
    address = page_round_up(address);
    /* PG_FAULT_PRESENT --> 0x1 */
    if((error_code & PG_FAULT_PRESENT) && (error_code & PG_FAULT_WRITE))
    {
      /* write access on a present page -- clearly a cow fault */
      if((vma->protection) & (PROT_WRITE))  
      {
        err = do_copy_on_write_fault(mm, vma, address);      
      }
    }
    else
      err = vma->fault(mm, vma, address);
  }
}

int do_copy_on_write_fault(struct mm_struct *mm, struct vm_map_entry *vma, uint64_t addr)
{
  int err;
  struct page page;

  uint64_t k_virt_addr;
  
  uint64_t phys_addr;
  int flags = PTE_PRESENT | PTE_READWRITE | PTE_USERSUPERVISOR;
  /*q
 first step - modify the page table entries for the faulting process' page-tables */

  /* second find the refcount of the page -- if it is 1, do not copy else copy */
  //kprintf("copy-on-write fault at address : %p\n", addr);
  page = find_page_from_virt(addr, mm->cr3);
  
  if(page.reference_count == 1)
  {
    // just modify the page table flags and then invalidate tlbs
    err = change_flags_in_parent_page_tables(addr, mm->cr3, flags);
    if(err < 0)
    {
      return -1;
    }
    set_pml4(mm->cr3);   // invalidate tlbs
  }
  else if(page.reference_count > 1)
  {
    /* parent and child both share the same page */
    phys_addr = (uint64_t)phys_mem_alloc();
    /* sneaky trick - create a page map it to kernel and then copy the old mapped page to this page */
    /* once you are done - then map the new page to the correct virtual address -- addr */
    k_virt_addr = (phys_addr | KERNBASE);
    my_memcpy((void *)k_virt_addr, (void *)addr, PAGE_BLOCK_SIZE - 1);
    
    map_virtual_address_to_physical_address((void *)addr, (void *)phys_addr, flags, mm->cr3, 0);
    decrement_reference_count_of_page(page);
    set_pml4(mm->cr3);   // invalidate tlbs
  }
  else
  {
    kprintf("Page ref count should not be 0\n");
    return -1;
  }
  return 1;  
}

int do_anon_fault(struct mm_struct *mm, struct vm_map_entry *vma, uint64_t address)
{
  /* two cases here */
  /* either the anonymous region is a stack or a heap */
  /* if the anon region is a stack , it should grow downwards, otherwise upwards */
  int flags;
  //kprintf("Anonymous fault at address : %p\n", address);
  //kprintf("vma protection is %d\n", vma->protection);
  //kprintf("PROT_WRITE is %d\n", PROT_WRITE);
  if(((vma->protection) & (PROT_WRITE)) != 0)
  {
    flags = PTE_PRESENT | PTE_READWRITE | PTE_USERSUPERVISOR;
  }
  else
  {
    flags = PTE_PRESENT | PTE_USERSUPERVISOR;
  }
  if(!(vma->file))
  {
    /* heap is of 1 page size - could increase via mmap() */
    address = page_round_up(address);
    void *page = phys_mem_alloc();
    change_flags_in_parent_page_tables(address, mm->cr3, flags);
    map_virtual_address_to_physical_address((void *)address, page, flags, mm->cr3, 0);
    my_memset((void *)address, 0, PAGE_BLOCK_SIZE - 1);  // 0 filled anon memory
  }
  return 1;
}

int do_file_fault(struct mm_struct *mm, struct vm_map_entry *vma, uint64_t address)
{
  int diff, flags;
  void *phys_addr;
  /* three cases here - depending on where the alignment of the page is relative to the vma */
  //kprintf("File fault at address : %p\n", address);
  uint64_t align_address = page_round_up(address);
  int to_read, to_offset, num_read;
  if(((vma->protection) & (PROT_WRITE)) != 0)
  {
    flags = PTE_PRESENT | PTE_READWRITE | PTE_USERSUPERVISOR;
  }
  else
  {
    flags = PTE_PRESENT | PTE_USERSUPERVISOR;
  }
  phys_addr = phys_mem_alloc();
  map_virtual_address_to_physical_address((void *)align_address, phys_addr, flags, mm->cr3, 0); 
  
  my_memset((void *)align_address, 0, PAGE_BLOCK_SIZE - 1);
  /* case 1 : the align_address appears before the vma->start_address */
  if(align_address < vma->start_addr)
  {
    diff = vma->start_addr - align_address;

    if(vma->end_addr >= (align_address + PAGE_BLOCK_SIZE))
    {
      to_read = (align_address + PAGE_BLOCK_SIZE) - vma->start_addr;
    }
    else
    {
      to_read = vma->end_addr - vma->start_addr;
    }
    to_offset = vma->fileoffset;
    num_read = vfs_read(mm->file, (void *)(align_address + diff), to_read, to_offset);
    //kprintf("amount of bytes read during page fault : %d\n", num_read);
  }
  
  /* case 2 : the align address appears in the between of the vma */
  else if(align_address > vma->start_addr && align_address < vma->end_addr)
  {
    /* do not read more than one page */
    if(vma->end_addr >= (align_address + PAGE_BLOCK_SIZE))
    {
      to_read = PAGE_BLOCK_SIZE;
    }
    else
    {
      to_read = vma->end_addr - align_address;
    }
    to_offset = vma->fileoffset + (align_address - vma->start_addr);
 
    num_read = vfs_read(mm->file, (void *)align_address, to_read, to_offset);
    //kprintf("amount of bytes read during page fault : %d\n", num_read);    
  }
  /* case 3: when the align address is at the start of the vma->start_addr */
  else if(vma->start_addr == align_address)
  {
    /* do not read more than one page */
    /* vma is exactly of 1 page size */
    if(vma->end_addr >= (align_address + PAGE_BLOCK_SIZE))
    {
      to_read = PAGE_BLOCK_SIZE;
    }
    else
    {
      to_read = vma->end_addr - vma->start_addr;
    }
    to_offset = vma->fileoffset;
    num_read = vfs_read(mm->file, (void *)align_address, to_read, to_offset);
    if(num_read <= 0 || num_read < to_read)
    {
      goto release_phys;
    }
    //kprintf("amount of bytes read during page fault : %d\n", num_read);
  }
  return 1;
  release_phys:
    phys_mem_free((uint64_t)phys_addr);
    return 0; 
}

