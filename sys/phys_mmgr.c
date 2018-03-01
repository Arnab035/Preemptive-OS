// A page has 4 components -- phys addr, refcount, next ptr, free indicator

#include "sys/phys_mmgr.h"

//struct page *free_page_list;

uint64_t number_of_pages;

void set_free_page_list(uint64_t bit_num)
{
  free_page_list[bit_num].is_free_page = 1;
}

void clear_free_page_list(uint64_t bit_num)
{
  free_page_list[bit_num].is_free_page = 0;
}

uint32_t test_free_page_list(uint64_t bit_num)
{
  return free_page_list[bit_num].is_free_page;
}

uint64_t phys_addr_of_first_free_page()
{
  if(!free_page_list) return 0;
  int i = 0;
  for(; i < number_of_pages; i++)
  {
    if(test_free_page_list(i))
    {
      if(free_page_list[i].reference_count == 0)
      {
        free_page_list[i].reference_count = 1;
      }
      continue;  // if is_free_page is set,that means pageisused
    }
    else
    {
      set_free_page_list(i);
      free_page_list[i].reference_count += 1;
      return free_page_list[i].phys_addr_of_page;
    }
  }
  kprintf("No free pages left -- running out of memory\n");
  return 0;
}

void *phys_mem_alloc()
{
  if(is_paging == 1)
  {
    free_page_list = (page *)((uint64_t)free_page_list | (uint64_t )(0xFFFFFFFF80000000));
  }
  return (void *)phys_addr_of_first_free_page();
}


/* phys_mem_alloc_more allocates more than one page */
/* params : n is the number of pages you want */

void *phys_mem_alloc_more(int n)
{
  void *ret;
  int i;
  int index;
  if(is_paging == 1)
  {
    free_page_list = (page *)((uint64_t)free_page_list | (uint64_t)(0xFFFFFFFF80000000));
  }
  index = get_index_of_first_free_page();
  ret = (void *)free_page_list[index].phys_addr_of_page;
  // set is_free_page flag for the array
  for(i = index; i < (index + (n - 1)); i++)
  {
    set_free_page_list(i);  
  }
  return ret; 
}

int get_index_of_first_free_page()
{
  if(!free_page_list) return 0;
  int i = 0;
  for(; i < number_of_pages; i++)
  {
    if(test_free_page_list(i)) continue;
    else
    {
      return i;   // first free page index
    }
  }
  kprintf("No free pages left\n");
  return -1;
}


void phys_mem_free(uint64_t addr)
{
  
  if(is_paging == 1)
  {
    free_page_list  = (page *)((uint64_t)free_page_list | (uint64_t)(0xFFFFFFFF80000000));
  }
  int i;
  for(i=0; i < number_of_pages; i++)
  {
    if(free_page_list[i].phys_addr_of_page == addr)
    {
      if(test_free_page_list(i))
      {
        // page becomes reusable only if the (--reference count) of the page is 0
        free_page_list[i].reference_count -= 1;
        if(free_page_list[i].reference_count <= 0)
        {
          clear_free_page_list(i);
        }
      }
      break;
    }
  }
  return; 
}

struct page get_page_from_phys_addr(uint64_t phys_addr)
{
  struct page *null_page = (struct page *)phys_mem_alloc();
  null_page = (struct page *)((uint64_t)null_page | KERNBASE);

  my_memset((void *)null_page, 0, PAGE_BLOCK_SIZE);

  if(is_paging == 1)
  {
    free_page_list = (page *)((uint64_t)free_page_list | KERNBASE);
  }
  int i = 0;
  for(; i < number_of_pages; i++)
  {
    if(free_page_list[i].phys_addr_of_page == phys_addr)
    {
      return free_page_list[i];
    }
  }
  return *null_page;
}

void increment_reference_count_of_page(uint64_t phys_addr)
{
  int i = 0;
  if(is_paging == 1)
  {
    free_page_list = (struct page *)((uint64_t)free_page_list | KERNBASE);
  }
  for(; i < number_of_pages; i++)
  {
    if(free_page_list[i].phys_addr_of_page == phys_addr)
    {
      free_page_list[i].reference_count += 1;
    }
  }
  return;
}

void decrement_reference_count_of_page(struct page page)
{ 
  int i = 0;
  if(is_paging == 1)
  {
    free_page_list = (struct page *)((uint64_t)free_page_list | KERNBASE);
  }
  for(; i < number_of_pages; i++)
  {
    if(free_page_list[i].phys_addr_of_page == page.phys_addr_of_page)
    {
      free_page_list[i].reference_count -= 1;
    }
  }
  return;
}

void initialize_free_list(void *physbase)
{
  int reserved, num_pages_reserved;
  free_page_list = (struct page *)physbase;
  //kprintf("free list starts from %p\n", free_page_list);
  my_memset((void *)free_page_list, 0, number_of_pages * sizeof(page));
  int i = 0;
  for(; i < number_of_pages ; i++)
  {
    free_page_list[i].phys_addr_of_page = (uint64_t)free_page_list + (i * PAGE_BLOCK_SIZE);
  }
  // reserve physical pages for the free list - do not let other applications pick them up
  reserved = number_of_pages * sizeof(page);
  num_pages_reserved = (reserved/4096);

  for(i = 0; i < num_pages_reserved; i++)
    set_free_page_list(i);

  return;
}

// TODO: Initialization of free list 
void define_free_list(void *phys_start, void *phys_end)
{
  int i = 0;
  number_of_pages = 0;  
  for(; i < (((uint64_t)phys_end)-((uint64_t)phys_start+0x1000))/PAGE_BLOCK_SIZE ; i++)
  {
    number_of_pages++;
  }
  //kprintf("Total number of pages are : %d\n", number_of_pages);
  initialize_free_list(phys_start + 0x1000);
}


void initialize_tarfs_sb()
{
  uint64_t reserved = number_of_pages * sizeof(page);
  uint64_t num_pages_reserved = (reserved/(4096));
  int index;
  tarfs_sb = (struct super_block *)(0x31E000 + (num_pages_reserved * 0x1000));
  
  index = get_index_of_first_free_page();
  set_free_page_list(index);

  //kprintf("tarfs super block starts from 0x%p\n", tarfs_sb);
  return;
}

void initialize_dentry_cache()
{
  dentry_cache_array = (struct dentry_cache *)phys_mem_alloc();
  if(is_paging)
    dentry_cache_array = (struct dentry_cache *)((uint64_t)dentry_cache_array | KERNBASE);

  my_memset((void *)dentry_cache_array, 0, PAGE_BLOCK_SIZE);
  return;
}
