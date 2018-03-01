/* implementation of mmap and munmap system calls and other helper functions */

#include <sys/memory.h>

void *sys_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
  return do_mmap(addr, length, prot, flags, fd, offset);
}

int sys_munmap(void *addr, size_t size)
{
  struct pcb *curr = get_current_process();
  struct mm_struct *mm = curr->mm;

  return do_munmap(mm, addr, size);
}

int do_munmap(struct mm_struct *mm, void *addr, size_t size)
{
  /* addr and len should be aligned to page */
  uint64_t unmap_addr;
  unmap_addr = page_round_up((uint64_t)addr);

  uint64_t unmap_len;
  unmap_len = page_round_down((uint64_t)size);

  struct vm_map_entry *vma;
  vma = find_vma_region(mm, unmap_addr, unmap_len);
  if(vma == NULL)
  {
    kprintf("Appropriate vma not found\n");
    return -1;
  }
  remove_vma(mm, vma);   // remove vma from the list of vmas for the mm

  deallocate_vma(vma);   // this actually deallocates the vma
  return 1; 
}

/* physically delete a vma */

void deallocate_vma(struct vm_map_entry *vma)
{
  /* vmas could be more than 1 page size */
  int err;
  /* close the file */
  if(vma->file)
  {
    err = vfs_close(vma->file);
    if(err < 0) return;
  }
  /* TODO: free pages */
  phys_mem_free((uint64_t)vma);
}

/* physically delete all vmas */
void free_vmas(struct mm_struct *mm, struct vm_map_entry *vmas)
{
  struct vm_map_entry *all_vma = vmas;
  while(all_vma != NULL)
  {
    deallocate_vma(all_vma);
    all_vma = all_vma->next;
  }
}

/* logically delete a vma from the list of vmas */
void remove_vma(struct mm_struct *mm, struct vm_map_entry *vma)
{
  /* first vma */
  if(mm->vma == vma)
  {
    mm->vma = mm->vma->next;
    mm->vma->next->prev = NULL;
    return;
  }
  else
  {
    struct vm_map_entry *curr_vma;
    curr_vma = mm->vma;
    while(curr_vma->next != NULL)
    {
      if(curr_vma == vma)
      {
        curr_vma->prev->next = curr_vma->next;
        curr_vma->next->prev = curr_vma->prev;
        return;
      }
      curr_vma = curr_vma->next;
    }
    if(curr_vma->next == NULL)
    {
      if(curr_vma == vma)
      {
        curr_vma->prev->next = NULL;
        curr_vma->prev = NULL;
        return;
      }
    }
  }
  return;     // no vmas deleted 
}

/* get unmapped area returns the address where a vma could fit */ 
uint64_t get_unmapped_area(struct mm_struct *mm, uint64_t addr, size_t len)
{
  struct vm_map_entry *vmas;
  /* finds the vma in which the address is completely present */
  if(!addr)
  {
    addr = (KERNBASE) / 3;
  }
  vmas = mm->vma;
    /* first fit search */
  return do_first_fit_search(vmas, addr, len);
}

uint64_t do_first_fit_search(struct vm_map_entry *vmas, uint64_t addr, size_t len)
{
  /* first vma */
  int is_first = 1;
  if(is_first)
  {
    if((addr) < vmas->start_addr && (addr + len) < vmas->end_addr)
      return addr;
    
    is_first = 0;
  }
  while(vmas->next != NULL)
  {
    // keep a page buffer for yourself
    if(addr >= vmas->end_addr && (addr + PAGE_BLOCK_SIZE + len) <= vmas->next->start_addr)
    {
      break;
    }
    vmas = vmas->next;
    addr = vmas->end_addr + 1;
  }
  /* last vma */
  if(vmas->next == NULL)
  {
    return ((vmas->end_addr + len) > (KERNBASE) ? -1 : (vmas->end_addr + 1));
  }
  else return addr;
}

void *do_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off)
{
  struct pcb *current = get_current_process();
  struct mm_struct *mm = current->mm;
  struct file *fp;
  struct dentry *fp_dentry;
  struct inode fp_inode;
  uint64_t filesize;
  uint64_t ulen;
  uint64_t ret_addr;
  ulen = page_round_up(len);
  if(!ulen)  // if length is 0
  {
    return (void *)NULL;
  }
  if((off + (ulen >> 12)) < off)  // overflow
  {
    return (void *)NULL;
  }
  if(ulen > (KERNBASE)) // invalid length - from linux dunno why I did this 
  {
    return (void *)NULL;
  }
  /* sanity checks */
  if((flags & MAP_PRIVATE) && (flags & MAP_SHARED))
  {
    /* cannot be private and shared at the same time */
    return (void *)-1;
  }
  if(flags & MAP_ANONYMOUS)
  {
    /* fd should be -1 , portable appls should ensure this */
    fp = NULL;
    filesize = 0;
  }
  else
  {
    fp = get_file_from_fd(fd);
    /* TODO: obtain filesize from dentry and then inode(check tarfs.c) */
    fp_dentry = fp->f_dentry;
    fp_inode = get_inode_from_dentry(fp_dentry);
    filesize = fp_inode.i_size;
  }
  /* if addr is provided, use it for mapping */
  ret_addr = get_unmapped_area(mm, (uint64_t)addr, len);
  //kprintf("ret addr is 0x%p  ", ret_addr); 
  if(ret_addr < 0)
  {
    return (void *)NULL;  // cannot do anything now 
  }
  return (void *)mmap_region(fp, ret_addr, filesize, len, off, prot);  
}

/* mmap_region is being called in 2 different ways - one by elf files and the other by a simple mmap call where the parameter t_memsz/t_filesz could be taken to be 0, hence ignore it*/

uint64_t mmap_region(struct file *file, uint64_t t_addr, uint64_t t_filesz , uint64_t t_memsz, uint64_t t_off, uint64_t prot)
{ 
  struct pcb *current_process = get_current_process();
  uint64_t map_addr = (uint64_t) t_addr;
  int err;
  struct mm_struct *mm = current_process->mm;
  if(!file)
  {
    prot &= ~(PROT_EXEC);   // anon vmas are not executable
  }
  if(map_addr > KERNBASE)
  {
    return -1;             // above kernbase
  }
  // get_unmapped_area
  struct vm_map_entry *vma = (struct vm_map_entry *)phys_mem_alloc();
  if(!vma) return -1;
  
  vma = (struct vm_map_entry *)((uint64_t)vma | KERNBASE);
  
  vma->filesize = t_filesz;
  vma->fileoffset = t_off;
  vma->protection = 0;
  vma->protection |= prot;
  vma->memsize = t_memsz;
  vma->start_addr = t_addr;
  if(t_filesz > t_memsz)
  {
    return -1;
  }
  if(!file)
  {
    vma->end_addr = t_addr + (4095 * ((t_memsz/4097) + 1));  // one page after the other ?
  }
  else
  {
    vma->end_addr = t_addr + t_filesz;  /* should be file size not memsize */
  }
  vma->file = file;

  if(file)
    vma->fault = do_file_fault;   // page fault handlers
  else
    vma->fault = do_anon_fault;

  err = add_vma(mm, vma);
  if(err < 0)
    goto free_vma;

  return t_addr; 
  free_vma:  // close on exec
    if(file)
      err = tarfs_close(file);
    phys_mem_free((uint64_t)vma);
    return -1;
}

struct vm_map_entry *find_vma_region(struct mm_struct *mm, uint64_t addr, size_t len)
{
  struct vm_map_entry *vma = mm->vma;
  while(vma != NULL)
  {
    if(vma->start_addr <= addr && page_round_down(vma->end_addr) >= (addr + len))
    {
      return vma;
    }
    vma = vma->next;
  }
  return NULL; 
}

/* add vma to list of mm->vmas */
int add_vma(struct mm_struct *mm, struct vm_map_entry *vma)
{
  struct vm_map_entry *vmas = mm->vma;
  struct vm_map_entry *prev;
  int err = find_vma_prev(vmas, vma, &prev);   // find vma in the list of vmas
  if(!err)
  {
    return -1;
  }
  insert_vma(mm, vma, prev);
  return 0;    
}

int find_vma_prev(struct vm_map_entry *vmas, struct vm_map_entry *vma, struct vm_map_entry **prev)
{
  //kprintf("Calling find vma prev...\n");
  struct vm_map_entry *curr_vma;
  curr_vma = vmas;

  if(curr_vma == NULL)
  {
    *prev = NULL;
    return 1;
  }
  
  if(vma->end_addr <= curr_vma->start_addr)
  {
    /* first entry is to be inserted */
    *prev = NULL;
    return 1;
  }
  while(curr_vma != NULL)
  {
    /* overlapping vmas not acceptable */
    if(is_overlapping_vma(curr_vma, vma))
    {
      kprintf("mmap overlaps\n");
      *prev = NULL;
      return 0;
    }
    if(curr_vma->next != NULL)
    {
      if((vma->start_addr >= curr_vma->end_addr) && (vma->end_addr <= curr_vma->next->start_addr))
      {
        *prev = curr_vma;
         break;
      } 
    }
    else
    {
      if(vma->start_addr >= curr_vma->end_addr)
      {
        *prev = curr_vma;
        break;
      }
    }
    curr_vma = curr_vma->next;  
  }
  return 1;
}

int is_overlapping_vma(struct vm_map_entry *curr, struct vm_map_entry *vma)
{
  if((curr->start_addr == vma->start_addr) && (curr->end_addr == vma->start_addr))
  {
    return 1;
  }
  else if((curr->start_addr < vma->start_addr ) && (curr->end_addr > vma->end_addr))   // full overlap
  {
    return 1;
  }
  else if((vma->start_addr < curr->end_addr) && (vma->end_addr >= curr->end_addr))
  {
    return 1;
  }
  else if((vma->start_addr <= curr->start_addr) && (vma->end_addr > curr->start_addr))
  {
    return 1;
  }
  
  else return 0;
}

void insert_vma(struct mm_struct *mm, struct vm_map_entry *vma, struct vm_map_entry *prev)
{
  /* doubly linked list */
  struct vm_map_entry *next;
  if(prev)
  {
    next = prev->next;
    prev->next = vma;
  }
  else
  {
    next = mm->vma;
    mm->vma = vma;
  }
  vma->prev = prev;
  vma->next = next;
  if(next)
    next->prev = vma;
}

/* Look up the first VMA which satisfies  addr < vm_end,  NULL if none. */

struct vm_map_entry *find_vma(struct mm_struct *mm, uint64_t addr)
{
  struct vm_map_entry *temp_vma = mm->vma;
  uint64_t end_addr;
  while(temp_vma != NULL)
  {
    if(temp_vma->file != NULL)
    {
      end_addr = (temp_vma->filesize < temp_vma->memsize) ? (temp_vma->start_addr + temp_vma->memsize) : temp_vma->end_addr;
    }
    else
    {
      end_addr = temp_vma->end_addr;
    }
    if(temp_vma->start_addr > end_addr/*temp_vma->end_addr*/)
    {
      if(addr >= end_addr && addr <= temp_vma->start_addr)
        return temp_vma;
    }
    else
    {
      if(addr >= temp_vma->start_addr && addr <= end_addr)
      {
        return temp_vma;
      }
    }
    temp_vma = temp_vma->next;
  }
  return NULL;
}

// TODO: assign types to each of the VMAS

/* usually vmas will follow the order --> code->data->heap->stack */

struct vm_map_entry *find_closest_vma(struct mm_struct *mm, uint64_t address)
{
  struct vm_map_entry *all_vma = mm->vma;
  int diff1, diff2;
  
  while(all_vma != NULL)
  {
    if(address < all_vma->start_addr)
    {
      if(all_vma->prev)
      {
        diff1 = all_vma->start_addr - address;
        diff2 = address - all_vma->prev->end_addr;
        if(diff1 <= diff2)
        {
          return all_vma;
        }
        else return all_vma->prev;
      }
      else
      {
       /* should be the first vma present */
        return all_vma;
      } 
    }
    all_vma = all_vma->next;

  }
  return NULL;  /* address could be beyond stack */
}

int expand_stack(struct mm_struct *mm, uint64_t address)
{
  struct vm_map_entry *mm_vma = mm->vma;
  
  uint64_t new_addr = address;
  new_addr = new_addr & ~(PAGE_BLOCK_SIZE-1);   // multiple of page size
  
  while(mm_vma != NULL)
  {
    if(mm_vma->end_addr == STACK_START)
    {
      if(new_addr < STACK_LIMIT)
      {
        kprintf("Cannot expand stack - you reached its limits\n");
        return -1;
      }
      mm_vma->start_addr = address;
    }
    mm_vma = mm_vma->next;
  }
  return 0;
}

int do_heap_vma(struct mm_struct *curr_mm)
{
  /* heap should start from the end of data vma */
  struct vm_map_entry *mm_vma = curr_mm->vma;
  int err;
  while(mm_vma->next != NULL)
    mm_vma = mm_vma->next;

  /* when you are in the last vma - the end_addr of vma is the start of heap */

  struct vm_map_entry *vma_heap = (struct vm_map_entry *)phys_mem_alloc();
  vma_heap = (struct vm_map_entry *)((uint64_t)vma_heap | KERNBASE);

  vma_heap->start_addr = page_round_down(mm_vma->end_addr);
  vma_heap->end_addr = vma_heap->start_addr + 4095;
  
  vma_heap->fault = do_anon_fault;  // anon 0 fill-on-demand
  vma_heap->file = NULL;

  vma_heap->protection = PROT_WRITE;
  vma_heap->filesize = 0;
  vma_heap->memsize = PAGE_BLOCK_SIZE;

  err = add_vma(curr_mm, vma_heap);
  if(err < 0)
  {
    goto free_vma_heap;
  }
  return 0;

  free_vma_heap:
    phys_mem_free((uint64_t)vma_heap);
    return -1;
}

int do_stack_vma(struct mm_struct *mm, const char **argv, const char **envp)
{
  /* create a new vma for the stack */
  struct vm_map_entry *vma_stack = (struct vm_map_entry *)phys_mem_alloc();
  int argc, envc, err, max_copy;
  
  int i;
  vma_stack = (struct vm_map_entry *)((uint64_t)vma | KERNBASE);
  vma_stack->start_addr = STACK_START;
  vma_stack->end_addr = STACK_START - (3 * PAGE_BLOCK_SIZE);   // 3 page sizes

  vma_stack->filesize = 0;
  vma_stack->fileoffset = 0;
  vma_stack->protection = PROT_WRITE;

  vma_stack->file = NULL;
  vma_stack->fault = do_anon_fault;
  vma_stack->memsize = (3 * PAGE_BLOCK_SIZE);
  /* make sure you put the argc, argv and envp pointers into the stack */
  if(argv)
  {
    for(i = 0; argv[i] != '\0'; i++);
    argc = i;
  }
  else argc = 0;
  if(envp)
  {
    for(i = 0; envp[i] != '\0'; i++);
    envc = i;
  }
  else envc = 0;

  /* one page keeps all the pointers(this page can be compared to am integer array), the other page keeps the strings */
  uint64_t *phys_pointer = (uint64_t *)phys_mem_alloc();
  uint64_t *virt_phys_pointer = (uint64_t *)((uint64_t)phys_pointer | KERNBASE);

  my_memset((void *)virt_phys_pointer, 0, PAGE_BLOCK_SIZE);

  char *phys_strings = (char *)phys_mem_alloc();
  char *virt_phys_strings = (char *)((uint64_t)phys_strings | KERNBASE);

  my_memset((void *)virt_phys_strings, 0, PAGE_BLOCK_SIZE);

  /* TODO : populate list of argv pointers to argv strings */
   
  max_copy = 512;
  //copy_params_to_userspace(envp, envc, argv, argc, max_copy, virt_phys_pointer, virt_phys_strings);
  copy_params_to_userspace(envp, envc, max_copy, virt_phys_pointer, &virt_phys_strings);
  copy_params_to_userspace(argv, argc, max_copy-envc-1, virt_phys_pointer, &virt_phys_strings);
  
  max_copy = max_copy - (envc + argc + 2);  // for two NULLs

  //kprintf("Number of parameters written : %d\n", 512 - max_copy);

  virt_phys_pointer[max_copy - 1] = argc;
  /* now map the pages */
  map_virtual_address_to_physical_address((void *)(STACK_START), (void *)phys_strings, PTE_PRESENT | PTE_READWRITE | PTE_USERSUPERVISOR, mm->cr3, 0);
  
  mm->exec  = STACK_START - PAGE_BLOCK_SIZE - (8 * (argc + envc + 3));  // point to stack top

  map_virtual_address_to_physical_address((void *)(STACK_START - (2 * PAGE_BLOCK_SIZE)), (void *)phys_pointer, PTE_PRESENT | PTE_READWRITE | PTE_USERSUPERVISOR, mm->cr3, 0);

  err = add_vma(mm, vma_stack);
  if(err < 0)
  {
    goto free_vma_stack;
  }
  return 0;
  free_vma_stack:
    phys_mem_free((uint64_t)vma_stack);
    return -1;
}

void copy_params_to_userspace(const char **strs, int argc, int max_copy, uint64_t *ptrs, char **deststr) {
    uint64_t user_ptr = page_round_up(STACK_START);
    char *dest = *deststr;
    const char *src;
    user_ptr += (uint64_t)dest - page_round_up((uint64_t)dest);

    ptrs[--max_copy] = 0;
    while(argc-- > 0) 
    {
        src = strs[argc];
        ptrs[--max_copy] = user_ptr; 
        while (user_ptr++ && (*dest++ = *src++) != '\0');
    }
    *deststr = dest;
}

/* UNUSED SORRY !
int copy_params_to_userspace(const char **envp, int envc, const char **argv, int argc, int max_copy, uint64_t *pointer, char *strings)
{
  * start from envp 
  uint64_t virt_addr;
  virt_addr = STACK_START;
  char *dest = strings;
  const char *src;
  int len;
  int i = max_copy;
  int offset = 0, bytes_to_copy;
  char *pos;
  pointer[--i] = 0;
  //virt_addr += page_mask((uint64_t)dest);
  while(envc-- > 0)
  {
    //pointer[--i] = (uint64_t)virt_addr; should contain the userspace address where you have the actual string 
    pos = dest;
    src = envp[envc];
    len = my_strlen(src);
    offset = (uint64_t)pos % (PAGE_BLOCK_SIZE);
    if(offset == 0)
    {
      offset = PAGE_BLOCK_SIZE ;
    }
    virt_addr = virt_addr - len;   should be less than a page 
    pointer[--i] = virt_addr;
    bytes_to_copy = len;   // for 506 expect len is smaller than  a page
    offset -= bytes_to_copy;
    my_memcpy((void *)(pos + offset), (char *)src, len);
    //j++;
  }
  pointer[--i] = 0;
  while(argc-- > 0)
  {
    pos = dest;
    src = argv[argc];
    len = my_strlen(src);
    //offset = (uint64_t)pos % (PAGE_BLOCK_SIZE);
    if(offset == 0)
    {
      offset = PAGE_BLOCK_SIZE;
    }
    virt_addr = virt_addr - len;
    pointer[--i] = virt_addr;
    bytes_to_copy = len;
    offset -= bytes_to_copy;
    my_memcpy((void *)(pos + offset), (char *)src, len);
    //j++;
  }
  return 1;   // NO NEED : update as to how many pointers were written
}
*/

/* The function returns non-zero if the region is likely accessible (though access may still result in -EFAULT) */
/* NOTE : vmas are always having the correct representation of protection of pages - the underlying pages might be having different protection flags which will raise faults anyway */

int is_userspace_access_valid(struct mm_struct *mm, void *addr, size_t n, int type)
{
  struct vm_map_entry *corr_vma;
  corr_vma = find_vma_region(mm, (uint64_t)addr, n-1);

  if(corr_vma == NULL)
  {
    return -1;
  }
  else
  {
    if(type == VERIFY_WRITE)
    {
    /* the vma should be read and write accessible */
      return ((corr_vma->protection) & (PROT_WRITE));
    }
    else
    {
      return ((corr_vma->protection) & (PROT_READ));
    }
  }
}

