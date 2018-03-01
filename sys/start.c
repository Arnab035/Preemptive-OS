/************   definition of the first task with process 0 pid ************/

#include <sys/start.h>

/* important notes : init  process is the child of idle process(0) - the idle process will be statically allocated.. init would of course copy everything barring files and mm from the idle process like a normal fork */


void start_kernel()
{
  // console_init();   // TODO: add later --> clears console
  kprintf("!!!*** Welcome to BLEH OS ***!!!\n");

  disable_all_interrupts();
  /** all initializations should follow **/
  init_idt();
  initialize();
  init_setup_virtual_memory();
  init_syscall(/*0x2B, 0x8 */);   // user64_CS is 0x2B   
  i86_initialize_page_fault_handler();  
  tarfs_init();      
  scheduler_init(); 
  ttyinit();

  enable_all_interrupts();
  final_init();
}

void final_init()
{
  kernel_thread(kernel_init, NULL);
  /* run schedule atleast once to set the system in motion - this will pick up and do exec sbush */
  schedule();
}

int kernel_init()
{
  // terminal will be opened here 
  int fd;
  fd = terminal_open(O_RDWR, 0);
  //kprintf("fd %d is opened\n", fd);  // stdin
  fd = terminal_open(O_RDWR, 0);
  //kprintf("fd %d is opened\n", fd);   // stdout
  fd = terminal_open(O_RDWR, 0);
  kprintf("fd %d is opened\n", fd);
  int ret = run_init_process("/bin/init");
  if(ret)
  {
    kprintf("something is wrong with /bin/init");
  }
  return ret;  
}

int run_init_process(const char *filename)
{
  char *name = getname(filename);

  const char *argv_init[] = {name, NULL};
  const char *envp_init[] = {"HOME=/", NULL};
  
  return do_execve(name, argv_init, envp_init);
}

/* create a kernel thread - kernel threads do not copy/involve with vmas */

void kernel_thread(int (*fn)(), void *arg)
{
  //kprintf("--calling do_fork--\n");
  do_fork(CLONE_VM|CLONE_FS, (unsigned long)fn, (unsigned long)arg);  // CLONE_VM means do not copy vmas
}

