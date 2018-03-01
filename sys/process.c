/************** process implementation ****************/
/*******************************************************/
 
#include <sys/process.h>

void switch_to_user_cr3()
{
  struct pcb *curr = get_current_process();
  struct mm_struct *mm = curr->mm;

  set_pml4(mm->cr3);
}

uint32_t insert_allproc(struct pcb *process)
{
  if(!process) return -1;
  struct proc_list *new_proc_list = (struct proc_list *)phys_mem_alloc();

  if(!new_proc_list)
  {
    return -1; //ENOMEM
  }
  new_proc_list = (struct proc_list *)((uint64_t)new_proc_list | KERNBASE);

  new_proc_list->process = process;
  new_proc_list->next = NULL; 

  if(!allproc)
  {
    allproc = new_proc_list;
    return 0;
  }
  struct proc_list *temp = allproc;
  while(temp->next != NULL)
  {
    temp = temp->next;
  }
  temp->next = new_proc_list;
  return 0;
}


uint32_t delete_from_allproc(struct pcb *process)
{
  if(!process) return -1;
  if(!allproc) return 0;
  struct proc_list *temp = allproc;
  struct proc_list *prev = allproc;
  // several cases here
  // 1. if first element is to be deleted
  if(temp->process->pid == process->pid) 
  {
    allproc = allproc->next;
    phys_mem_free((uint64_t)temp);
    return 0;   // 0 indicates success
  }
  // 2, any element in the middle is to be deleted
  while(temp->next != NULL)
  {
    struct pcb *currp = temp->process;
    if(currp->pid == process->pid)
    {
      prev->next = temp->next;
      phys_mem_free((uint64_t)temp);
      return 0;
    }
    prev = temp;
    temp = temp->next;
  }
  // 3, any element in the end to be deleted
  if(temp->next == NULL)
  {
    prev->next = NULL;
    phys_mem_free((uint64_t)temp);
    return 0;
  }
  return 0;
}


uint32_t delete_from_zombproc(struct pcb *process)
{
  if(!process) return -1;
  if(!zombproc) return 0;
  struct proc_list *temp    = zombproc;
  struct proc_list *prev = zombproc;
  //kprintf("deleting from zombproc\n");
  if(temp->process->pid == process->pid)
  {
    zombproc = temp->next;
    phys_mem_free((uint64_t)temp);
    return 0;
  }
  while(temp->next != NULL)
  {
    struct pcb *currp = temp->process;

    if(currp->pid == process->pid)
    {
      prev->next = temp->next;
      phys_mem_free((uint64_t)temp);
      return 0;
    }
    prev = temp;
    temp = temp->next;
  }
  if(temp->next == NULL)
  {
    prev->next = NULL;
    // TODO: implement phys_mem_free
    phys_mem_free((uint64_t)temp);
    return 0;
  }
  return 0;
}

uint32_t insert_into_pchildren(struct proc_list *pchildren, struct pcb *process)
{
  if(!process) return -1;
  struct proc_list *new_proc_list = (struct proc_list *)phys_mem_alloc();

  if(!new_proc_list)
  {
    return -1; //ENOMEM
  }
  new_proc_list = (struct proc_list *)((uint64_t)new_proc_list | KERNBASE);

  new_proc_list->process = process;
  new_proc_list->next = NULL;

  if(!pchildren)
  {
    pchildren = new_proc_list;
    return 0;
  }
  struct proc_list *temp = pchildren;
  while(temp->next != NULL)
  {
    temp = temp->next;
  }
  temp->next = new_proc_list;

  return 0;
}

uint32_t insert_zombproc(struct pcb *process)
{
  //kprintf("Zombproc inserted\n");
  if(!process) return -1;
  struct proc_list *new_proc_list = (struct proc_list *)phys_mem_alloc();

  if(!new_proc_list)
  {
    return -1; //ENOMEM
  }
  new_proc_list = (struct proc_list *)((uint64_t)new_proc_list | KERNBASE);

  new_proc_list->process = process;
  new_proc_list->next = NULL;

  if(!zombproc)
  {
    zombproc = new_proc_list;
    return 0;
  }
  struct proc_list *temp = zombproc;
  while(temp->next != NULL)
  {
    temp = temp->next;
  }
  temp->next = new_proc_list;
  return 0;
}

struct pcb *get_process_descriptor_from_pid(pid_t pid)
{
  /* search both allproc and zombproc lists - get the process descriptor from any of the pid that matches the processes */
  struct proc_list *allproc_head = allproc;
  //struct proc_list *zombproc_head = zombproc;

  struct pcb *process;

  while(allproc_head && allproc_head->process != NULL)
  {
    process = allproc_head->process;
    //kprintf("%d ", process->pid);
    if(process->pid == pid)
      return process;

    allproc_head = allproc_head->next;
  }
  /*
  while(zombproc_head && zombproc_head->process != NULL)
  {
    process = zombproc_head->process;
    //kprintf("%d ", process->pid);
    if(process->pid == pid)
      return process;
    zombproc_head = zombproc_head->next;
  }*/
  //well now go through the list of zombie processes
  kprintf("process has already been killed\n");
  return NULL;
}

int sys_fork()
{
  int ret;
  ret = do_fork(SIGCHLD, 0, 0);
  return ret;
}

int do_fork(int clone_flags, unsigned long stack_start, unsigned long stack_size)
{
  struct pcb *child;
  int err = 0;
  child        = (struct pcb *)phys_mem_alloc();
  if(is_paging)
  {
    child        = (struct pcb *)((uint64_t)child | KERNBASE);
  }
  my_memset((void *)child, 0, 4096);

  struct pcb *parent  =  get_current_process();
  child->process_state =  NEW;
  child->on_rq = 0;
  int i;
  
  /* deep copy */
  for(i = 0; i < MAX_FD ; i++)
  {
    if(parent->fd[i] == NULL) break; 
    child->fd[i] = parent->fd[i];
  }
  i = 0;
  if(parent->cptr == NULL)
  {
    parent->cptr = (struct proc_list *)phys_mem_alloc();
    parent->cptr = (struct proc_list *)((uint64_t)parent->cptr | KERNBASE);
    parent->cptr->process = child;
    parent->cptr->next = NULL;
  }
  else
  {
    while(parent->cptr != NULL)
    {
      parent->cptr = parent->cptr->next;
    }
    parent->cptr = (struct proc_list *)phys_mem_alloc();
    parent->cptr = (struct proc_list *)((uint64_t)parent->cptr | KERNBASE);
    parent->cptr->process = child;
    parent->cptr->next = NULL;
  }

  child->pptr     = parent;
  child->time_slice = TIME_SLICE;
  if(child->command == NULL)
  {
    child->command = (char *)phys_mem_alloc();
    child->command = (char *)((uint64_t)child->command | KERNBASE);
    my_memset((void *)child->command, 0, PAGE_BLOCK_SIZE);
  }
  my_memcpy((void *)child->command, (void *)parent->command, my_strlen(parent->command) + 1);  

  // assuming the initial kernel thread will be CLONE_FS, we assign "/" to process->cwd for now
  // if CLONE_FS is not there --(just for ease) copy the whole parent->cwd to child->cwd
  if(!child->cwd)
  {
    child->cwd = (char *)phys_mem_alloc();
    child->cwd = (char *)((uint64_t)child->cwd | KERNBASE);
  
    my_memset((void *)child->cwd, 0, PAGE_BLOCK_SIZE);
  }
  if(clone_flags & CLONE_FS)
  {
    my_memcpy((void *)child->cwd, "/", 1);
  }
  else
  {
    my_memcpy((void *)child->cwd, (void *)parent->cwd, my_strlen(parent->cwd));
  } 
  
  copy_mm(clone_flags, child, parent);

  if(!(clone_flags & CLONE_VM))
    err = copy_vm(child, parent);   /* cow fork */
  if(err < 0) return -1;
  
  if(clone_flags & CLONE_VM)
  {
    child->flags = PF_KTHREAD;  // kernel threads do not duplicate vmas
  }
  else
    child->flags = PF_UTHREAD;  // user thread

  /* copy thread */
  copy_thread(child, parent, stack_start, stack_size);
  /* end of copy thread */

  child->process_state = NORMAL;

  child->thread_state  = RUNNING;

  child->pid = get_pid();
  err = insert_allproc(child);
  if(err < 0) {return -1;}

  add_process_to_queue(&run_queue, child);
  
  child->on_rq = 1;
  return child->pid;
}

void increment_refcount_of_file(struct file *filp)
{
  filp->f_refcount += 1;
}

void copy_mm(int clone_flags, struct pcb *child, struct pcb *parent)
{
  struct mm_struct *old_mm, *mm;
  old_mm = parent->mm;

  if(!old_mm) return;
  if(clone_flags & CLONE_VM)
  {
    mm = old_mm;
    goto end;
  }
  mm = (struct mm_struct *)phys_mem_alloc();
  mm = (struct mm_struct *)((uint64_t)mm | KERNBASE);

  my_memcpy((void *)mm, (void *)old_mm, sizeof(struct mm_struct));
  increment_refcount_of_file(mm->file);
  /* child should copy the vmas of the parent */  
  dup_mm(mm, old_mm); 
  end:
    child->mm = mm;
    return;
}

void dup_mm(struct mm_struct *mm, struct mm_struct *old_mm)
{
  struct vm_map_entry *old_vma = old_mm->vma;
  struct vm_map_entry *last_vma;
  int is_first = 1;
  while(old_vma != NULL)
  {
    struct vm_map_entry *tmp = (struct vm_map_entry *)phys_mem_alloc();
    tmp = (struct vm_map_entry *)((uint64_t)tmp | KERNBASE);

    my_memcpy((void *)tmp, (void *)old_vma, sizeof(struct vm_map_entry));
    tmp->next = NULL;
    
    if(is_first)
    {
      tmp->prev = NULL;
      mm->vma = tmp;
      is_first = 0; 
    }
    else
    {
      tmp->prev = last_vma;
      last_vma->next = tmp;
    }
    last_vma = tmp;
    old_vma = old_vma->next;
  }
  //mm->cr3 = copy_page_tables(old_mm->cr3, 4);    
}

struct reg_list *task_reg_list(struct pcb *task)
{ return (struct reg_list *)((uint64_t)(task->esp0) -sizeof(struct reg_list)); }

void copy_thread(struct pcb *child, struct pcb *parent, unsigned long stack_start, unsigned long stack_size)
{
  uint64_t *kstack;
  int i;
  struct fork_frame *fork_frame;

  struct inactive_process_frame *frame;

  kstack   = (uint64_t *)phys_mem_alloc();
  //kprintf("0x%x", kstack);
  if(is_paging)
    kstack  = (uint64_t *)((uint64_t)kstack | KERNBASE);

  child->kstack = kstack;

  my_memset((void *)kstack, 0, PAGE_BLOCK_SIZE);
   
  //*(child->kstack) = *(parent->kstack);  // shallow copy parent stack to child stack
  for(i = 0; i < 512; i++)
  {
    child->kstack[i] = parent->kstack[i];
  }
  child->esp0 = (uint64_t)((uint64_t)child->kstack + PAGE_BLOCK_SIZE) ;

  struct reg_list *childregs;
  childregs = task_reg_list(child);   // start of child registers

  uint64_t address = (uint64_t)childregs - 56;  // offset from start of fork frame
  fork_frame = (struct fork_frame *)address;

  fork_frame->regs = *childregs;

  frame = &fork_frame->old_frame;
  frame->ret_addr = (uint64_t)ret_from_fork;  /* assembly magic */

  child->rsp = (uint64_t)fork_frame;
  if(child->flags & PF_KTHREAD)
  {
    my_memset((void *)childregs, 0, sizeof(struct reg_list));
    frame->rbx = stack_start;
    frame->r12 = stack_size;
    return;
  }

  frame->rbx = 0;

 *childregs = *task_reg_list(parent);
  childregs->rax = 0;   // fork return to child
  if(stack_start)
    childregs->sp = stack_start;
  return;
}


int copy_vm(struct pcb *child, struct pcb *parent)
{
  /* modify page table flags in the parent */
  struct mm_struct *parent_mm = parent->mm;
  struct vm_map_entry *parent_vm = parent_mm->vma;
  uint64_t vm_addr_start, vm_addr_end, phys_addr;
  int err;
  uint64_t i;
  uint64_t parent_cr3 = parent->mm->cr3;
  struct mm_struct *child_mm = child->mm;

  child_mm->cr3 = copy_page_tables((uint64_t)get_pml4(), 4);  // temporarily copy kernel page tables

  while(parent_vm != NULL)
  {
    if((uint64_t)parent_vm->start_addr > (uint64_t)parent_vm->end_addr)
    {
      vm_addr_start = parent_vm->end_addr;
      vm_addr_end = parent_vm->start_addr;
    }
    else
    {
      vm_addr_start = parent_vm->start_addr;
      vm_addr_end = parent_vm->end_addr;
    }
    
    //kprintf("vm addr start is 0x%x and vm addr end is 0x%x\n", vm_addr_start, vm_addr_end);
    for(i = vm_addr_start; i < vm_addr_end; i+= 4096)
    {
      err = change_flags_in_parent_page_tables(i, parent_cr3, PTE_PRESENT | PTE_USERSUPERVISOR);
      if(err < 0)
      {
        kprintf("Problems where changing flags in parent page tables\n");
        return -1;
      }
    }
    parent_vm = parent_vm->next;
  }

  parent_vm = parent_mm->vma;
  while(parent_vm != NULL)
  {
    if((uint64_t)parent_vm->start_addr > (uint64_t)parent_vm->end_addr)
    {
      vm_addr_start = parent_vm->end_addr;
      vm_addr_end = parent_vm->start_addr;
    }
    else
    {
      vm_addr_start = parent_vm->start_addr;
      vm_addr_end = parent_vm->end_addr;
    }

    for(i = vm_addr_start; i < vm_addr_end; i+= 4096)
    {
      phys_addr = walk_pml4(parent->mm->cr3, i);
      if(phys_addr == -1 || phys_addr == 0)
      {
        continue;
      }
      map_virtual_address_to_physical_address((void *)i, (void *)phys_addr, PTE_PRESENT | PTE_USERSUPERVISOR, child_mm->cr3, 0);
      increment_reference_count_of_page(phys_addr); 
    }
    parent_vm = parent_vm->next;
  }
  return 0;
}

/* create a new mm struct for the process */

void mm_init(struct pcb *curr)
{
  struct mm_struct *mm = NULL;
  
  mm = (struct mm_struct *)phys_mem_alloc();
  mm = (struct mm_struct *)((uint64_t)mm | KERNBASE);

  my_memset((void *)mm, 0, PAGE_BLOCK_SIZE);
  mm->exec = 0;

  mm->cr3 = copy_page_tables((uint64_t)get_pml4(), 4);
  if(!mm)
  {
    goto err;   // ENOMEM
  }
  curr->mm = mm;
  return;

  err:
    phys_mem_free((uint64_t)mm);
}

int count(char **argv)
{
  int i = 0;
  while(argv[i] != NULL)
  {
    i++;
  }
  return i;
}

// returns the fully qualified pathname of the file in kernel memory

char *getname(const char *filename)
{
  char *path = (char *)phys_mem_alloc();

  path = (char *)((uint64_t)path | KERNBASE);
  my_memcpy((void *)path, (void *)filename, my_strlen(filename));

  return path;
}

int sys_execve(const char *filename, const char *argv[], const char *envp[])
{
  char *pathname;
  pathname = getname(filename);   // copy user-space data into kernel space

  return do_execve(pathname, argv, envp);
}

// execve is expected to contain the full pathname for the file
int do_execve(char *filename, const char *argv[], const char *envp[])
{
  struct pcb *current_process = get_current_process();
  int err;
  struct file *fp;

  char *pathname = getname(filename); // TODO: implement getname
  
  fp = do_open_exec(pathname, O_RDONLY, 0);	

  char *buf;

  buf = (char *)phys_mem_alloc();
  buf = (char *)((uint64_t)buf | KERNBASE);
  
  //uint64_t arch_stack_start = STACK_START;/* insert sane value here */
  if(current_process->command != NULL)
  {
    my_memset((void *)current_process->command, 0, PAGE_BLOCK_SIZE);
    my_memcpy((void *)current_process->command, (void *)filename, my_strlen(filename) + 1);
  }

  mm_init(current_process);
  
  current_process->mm->file = fp;
  struct mm_struct *curr_mm = current_process->mm;
  
  /* copy filename to the top of stack */

  err = load_elf_binary(curr_mm);
  if(err < 0)
    goto read_out;

  err = do_heap_vma(curr_mm);
  if(err < 0)
    goto read_out;

  err = do_stack_vma(curr_mm, argv, envp);
  if(err < 0)
    goto read_out;

  struct reg_list *registers = task_reg_list(current_process);
  
  start_thread(registers, curr_mm->exec, curr_mm->rip);   
  return 0;

  read_out:
    phys_mem_free((uint64_t)buf);
    return -1;
}

struct mm_struct *init_mm(uint64_t pml4)
{
  struct mm_struct *init_mm = (struct mm_struct *)phys_mem_alloc();
  init_mm = (struct mm_struct *)((uint64_t)init_mm | KERNBASE);

  init_mm->vma = NULL;
  init_mm->exec = 0;
  init_mm->file = NULL;
  init_mm->rip = 0;
  init_mm->cr3 = (uint64_t)get_pml4();
  return init_mm;
}

void start_thread(struct reg_list *regs, uint64_t user_rsp, uint64_t user_rip)
{
  regs->rip = user_rip;
  regs->sp = user_rsp;
  regs->cs  = 0x2B;
  regs->ss  = 0x23;
  regs->rflags = 1 << 9;   // interrupt flag
}


void sys_exit(int status)
{
  // free user portions of address space
  struct pcb *current_process = get_current_process();
  //kprintf("process pid %d is being exited\n", current_process->pid);
  if(!(current_process->pid))
  {
    kprintf("Attempt to kill idle task\n");
    return;
  }
  /* all child processes should be reparented to init */
  struct proc_list *childlist = current_process->cptr;
  while(childlist != NULL)
  {
    reparent_process_to_init(childlist->process);   // TODO: implement
    childlist = childlist->next;
  }
  // close open file descriptors in the process

  int err;
  err = exit_files(current_process->fd);

  // release virtual memory resources for the process
  struct mm_struct *mm = current_process->mm;
  err = exit_vmmaps(current_process, mm);

  if (err < 0)
    return;
  // process is removed from all proc and added to zombproc
  struct proc_list *temp = allproc;
  while(temp != NULL)
  {
    if(temp->process->pid == current_process->pid)
    {
      delete_from_allproc(current_process);
      insert_zombproc(current_process);
    }
    temp = temp->next;
  }
  
  /* child process exiting- wakeup parent */
  current_process->process_exit_status = status;
  current_process->process_state = ZOMBIE;
  current_process->thread_state = STOPPED;

  exit_notify(current_process);   /* if parent is waiting, wake it up */
  schedule();
}

void reparent_process_to_init(struct pcb *process)
{
  // first task in allproc has to be init task
  if(!allproc)
  {
    kprintf("init task undefined\n");
  }
  int err;
  struct proc_list *init_list = allproc;
  struct pcb *init_task = init_list->process;
 
  process->pptr = init_task; 
  err = insert_into_pchildren(init_task->cptr, process); // process becomes a child of init now
  if(err < 0)
  {
    kprintf("undefined error while reparenting process to init\n");
    return;
  }
  return;
}

void exit_notify(struct pcb *process)
{
  if(process->pptr != NULL)
  {
    try_to_wake_up(&waiting_queue, process->pptr);
  }
}

int exit_files(struct file *fd[])
{
  int i = 0;
  while(fd[i] != NULL)
  {
    if(fd[i]->f_refcount <= 0)
    { // get the open file entry struct address and free it
      sys_close(i);   // deallocates memory 
      fd[i] = NULL;
    }
    i++;
  }
  return 1;
}

int exit_vmmaps(struct pcb *process, struct mm_struct *mm)
{
  /* TODO: implement properly */
  struct vm_map_entry *vmas = mm->vma;
 
  free_vmas(mm, vmas);
  mm->vma = NULL;

  free_page_range(4, (uint64_t)mm->cr3);  // pass the level also helps identify which table to use
  return 1;
}

// wait system call  -- it returns the pid of the completed process
int sys_wait(int *status)
{
  struct pcb *child;
  int retval;
  struct pcb *proc  = get_current_process();
  struct proc_list *child_list;
  if(proc->cptr == NULL) return -1;   // no child processes
  proc->process_state = NORMAL;
  proc->thread_state = WAITING;   // do not use SLEEPING
  // scheduler will add parent to wait queue during next schedule
  repeat:
    child_list = proc->cptr;
    do{
      //child_list = proc->cptr;
      child = child_list->process;
      retval = do_wait_thread(child);
      
      if(retval)
      {
        proc->process_state = NORMAL;
        proc->thread_state = RUNNING;
        *status = child->process_exit_status;
        kprintf("successful exit with retval: %d\n", retval);
        return retval;
      }
      else child_list = child_list->next;    
    }while(child_list != NULL);
    
  schedule();  // when schedule is called -> it will remove sleeping threads
  goto repeat;
}

int do_wait_thread(struct pcb *child)
{
  int retval;
  if(child->process_state == DEAD && child->thread_state == STOPPED)
  {
    retval = 0;
    return retval;  // unlikely
  }
  else if(child->process_state == ZOMBIE && child->thread_state == STOPPED)
  {
    child->process_state = DEAD;
    retval = child->pid;
    //kprintf("about to kill finally\n");
    delete_from_zombproc(child);
    release_task(child);
    return retval;
  }
  else return 0;
}

void release_task(struct pcb *dead_task)
{
  // release the kstack and the whole pcb
  uint64_t *kstack = dead_task->kstack;
  
  phys_mem_free((uint64_t)kstack);
  struct pcb *free_task = dead_task;

  phys_mem_free((uint64_t)free_task);
  return;
}

/* getprocs system call - ** EMERGENCY CHANGE IN DESIGN ** - write the whole process data into a large kernel buffer and return after copying to the user space buffer buf **/

/* do not make it fancy now ** make sure it works first  */

int sys_getprocs64(char *buf, size_t size)
{
  char *temp = buf;
  my_memset((void *)temp, 0, size);
  struct pcb *curr = get_current_process();
  char *p_info;
  if(is_userspace_access_valid(curr->mm, (void *)buf, size, VERIFY_WRITE) == 0)
  {
    return -1;
  }
  struct proc_list *allproc_head = allproc;
  
  struct pcb *process;
  int n_written = 0;
  struct proc_list *zombproc_head = zombproc;

  //char *kbuf = (char *)phys_mem_alloc();
  //char *kbuf_virt = (char *)((uint64_t)kbuf | KERNBASE);

  while(allproc_head && allproc_head->process != NULL)
  {
     if(n_written > size)
     {
       kprintf("size is full\n");
       return size;
     }
     process = allproc_head->process;
     p_info = itoa(process->pid);
     my_memcpy((void *)(temp + n_written), (void *)p_info, my_strlen(p_info) + 1);
     //*(buf + n_written) = process->pid;
     n_written += 6;
     p_info = itoa(process->pptr->pid);
     my_memcpy((void *)(temp + n_written), (void *)p_info, my_strlen(p_info) + 1);
     n_written += 6;

     my_memcpy((void *)(temp + n_written), (void *)process->command, my_strlen(process->command) + 1);
     //n_written += sizeof(process->process_state);
     n_written += 20;
     allproc_head = allproc_head->next;
  }
  //well now go through the list of zombie processes
  if(!zombproc_head && zombproc_head->process == NULL)
  {
    goto end;
  }
  
  while(zombproc_head && zombproc_head->process != NULL)
  {
    if(n_written > size)
    {
      kprintf("size is full\n");
      return size;
    }
    process = zombproc_head->process;
    p_info = itoa(process->pid);
    my_memcpy((void *)(buf + n_written), (void *)p_info, my_strlen(p_info) + 1);
     //*(buf + n_written) = process->pid;
    n_written += 6;

    p_info = itoa(process->pptr->pid);
    my_memcpy((void *)(buf + n_written), (void *)p_info, my_strlen(p_info) + 1);
    n_written += 6;

    my_memcpy((void *)(buf + n_written), (void *)process->command, my_strlen(process->command) + 1);
    n_written += 20;
    
    zombproc_head = zombproc_head->next;
  }

  // now map kvirt_buf to buf
  end:
    return n_written;
}

