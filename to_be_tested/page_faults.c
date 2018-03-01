/*********** C source file to handle page faults in my OS ************/

#include <sys/page_fault_handler.h>

void i86_initialize_page_fault_handler()
{
  set_int_vect(14, i86_page_fault_handler);
}

/* this function calls the page fault handler wrapper in assembly */
void i86_page_fault_handler()
{
  page_fault_handler_wrapper();
}

/* this is the actual page fault handler called by the page fault handler wrapper */
/* rdi -error code, rsi - faulting address */

void page_fault_handler(uint64_t error_code, uint64_t address)
{
  kprintf("Page fault happened at address 0x%p\n", address);
  int type_of_page_fault = error_code & 0x7;  // only the last 3 bits
  uint64_t phys_addr, kern_phys_addr;
  struct pcb *curr = get_current_process();
  /*
US RW  P - Description
0  0  0 - Supervisory process tried to read a non-present page entry
0  0  1 - Supervisory process tried to read a page and caused a protection fault
0  1  0 - Supervisory process tried to write to a non-present page entry
0  1  1 - Supervisory process tried to write a page and caused a protection fault
1  0  0 - User process tried to read a non-present page entry
1  0  1 - User process tried to read a page and caused a protection fault
1  1  0 - User process tried to write to a non-present page entry
1  1  1 - User process tried to write a page and caused a protection fault
  */

  switch(type_of_page_fault)
  {
    case 0:
        kprintf("page fault : kernel process reads a non-existent page\n");
  	phys_addr = (uint64_t)phys_mem_alloc();
        // allocate a new page using cr3 as kernel cr3
        map_virtual_address_to_physical_address((void *)address, (void *)phys_addr, PTE_PRESENT | PTE_READWRITE, (uint64_t)get_pml4, 0);
	break;
    case 1:
	kprintf("page fault : kernel process reads a page and causes protection fault\n");
        //TODO: actions	
        break;
    case 2:
        kprintf("page fault : kernel process tries to write to a non-existent page\n");
        phys_addr = (uint64_t)phys_mem_alloc();
        map_virtual_address_to_physical_address((void *)address, (void *)phys_addr, PTE_PRESENT | PTE_READWRITE, (uint64_t)get_pml4, 0);
	break;
    case 3:
    	kprintf("page fault : kernel tried to write to a read-only page\n");
        // TODO: actions    
	break;
    case 4:
        kprintf("page fault : user process tried to read non-existent page\n");
        phys_addr = (uint64_t)phys_mem_alloc();
        map_virtual_address_to_physical_address((void *)address, (void *)phys_addr, PTE_PRESENT | PTE_READWRITE | PTE_USERSUPERVISOR, curr->cr3, 0);
	break;
 
    case 5:
        kprintf("page fault : user process tried to read and caused protection fault\n");
        // TODO: actions
	break;

    case 6:
        kprintf("page fault : user process tried to write to non-existent page\n");
        phys_addr = (uint64_t)phys_mem_alloc();
        map_virtual_address_to_physical_address((void *)address, (void *)phys_addr, PTE_PRESENT | PTE_READWRITE | PTE_USERSUPERVISOR, (uint64_t)curr->cr3, 0);
	break;

    case 7:
        /* should be relevant to COW fork */
        kprintf("page fault : user process tried to write a read-only page\n");
        phys_addr = (uint64_t)phys_mem_alloc();
        kern_phys_addr = phys_addr | 0xFFFFFFFF80000000;
        my_memcpy((void *)address, (void *)kern_phys_addr, 4096);   // copy page
        map_virtual_address_to_physical_address((void *)address, (void *)phys_addr, PTE_PRESENT | PTE_READWRITE | PTE_USERSUPERVISOR, curr->cr3, 0);  // no-reference
	break;
    default:
        kprintf("Invalid page fault error code\n");
  }
}
