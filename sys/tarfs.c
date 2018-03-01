#include "sys/tarfs.h"

void tarfs_print_all_contents()
{
  struct posix_header_ustar *tarfs_fs = (struct posix_header_ustar *)&_binary_tarfs_start;
  unsigned char *temp = (unsigned char *)tarfs_fs;

  while(my_memcmp(temp + 257, "ustar", 5) == 0)
  {
    uint64_t filesize = oct2dec((unsigned char *)(temp + 124), 11);
    //kprintf("filename : %s\n", temp);
    //kprintf("prefix : %s\n", temp + 345);
    //kprintf("mode : %s\n", temp + 100);
    temp += (((filesize + 511) / 512) + 1) * 512;
  }
  return;
}

// returns a string that represents the dentry name for parent directory
struct name *get_parent_and_child_directories(char *filename)
{
  // safe - although wastes memory
  struct name *temp = (struct name *)phys_mem_alloc();
  temp = (struct name *)((uint64_t)temp | KERNBASE);
  
  my_memset((void *)temp, 0, PAGE_BLOCK_SIZE);

  int k;

  if(!filename)
  {
    temp->parent = NULL;
    temp->child = NULL;
    return temp;
  }
  char *temps = filename;
  char *ntemps = filename;
  while(*temps != '\0') temps++;
  
  // temp now points to the last character of file
  if(*(--temps) == '/')
  {
    /* parent is root dentry */
    temp->parent = (char *)phys_mem_alloc();
    temp->parent = (char *)((uint64_t)temp->parent | KERNBASE);
    //my_memcpy((void *)temp->parent, "/", 1);
    temp->parent[0] = '/';
    temp->parent[1] = '\0';

    temp->child = (char *)phys_mem_alloc();
    temp->child = (char *)((uint64_t)temp->child | KERNBASE);

    for(k = 0; *(ntemps + k) != '/'; k++)
    {
      *(temp->child + k) = *(filename + k);
    }
    return temp;
  }

  while(temps != filename && *temps != '/') temps--;

  if(temps == filename)  // must be root parent
  {
    temp->child = temps;
    temp->parent = "/";   // parent is root
    return temp;
  }
  temp->child = ++temps;
  char *temp1 = filename;
  int i = 0;
  --temps;                    // ugly hack
  while(temp1 != temps)
  {
    temp1++;
    i++;
  }
  int j = i;
  temp->parent = (char *)phys_mem_alloc();
  temp->parent = (char *)((uint64_t)temp->parent | KERNBASE);

  for(i = 0;i < j; i++)temp->parent[i] = filename[i];
  temp->parent[i] = '\0';
  return temp;
}

void tarfs_init()
{
  unsigned int inode_num = 3;
  char *name = (char *)phys_mem_alloc();

  //tarfs_print_all_contents();
  name = (char *)((uint64_t)name | KERNBASE);
  my_memset((void *)name, 0, PAGE_BLOCK_SIZE);

  uint64_t filesize;

  enum {NORMAL, HARD_LINK, SYM_LINK, CHARDEV, BLKDEV, DIRECTORY, NAMED_PIPE} filetype;
  /* initialize superblocks and inodes  */
  if(is_paging == 1)
  {
    tarfs_sb = (struct super_block *)((uint64_t)tarfs_sb | KERNBASE);
    dentry_cache_array = (struct dentry_cache *)((uint64_t)dentry_cache_array | KERNBASE);
  }

  tarfs_sb->s_op = (struct super_block_operations *)phys_mem_alloc();
  tarfs_sb->s_op = (struct super_block_operations *)((uint64_t)tarfs_sb->s_op | KERNBASE);

  tarfs_sb->s_op->alloc_inode = tarfs_alloc_inode;
  tarfs_sb->s_op->destroy_inode = tarfs_destroy_inode;
  tarfs_sb->s_op->sync_fs = NULL;
  tarfs_sb->s_op->write_inode = NULL;
  tarfs_sb->s_op->delete_inode = NULL;
  tarfs_sb->s_op->put_super = NULL;
  tarfs_sb->s_op->write_super = NULL;
  
  /* create a dentry for root */
  struct dentry *d_root;
  tarfs_sb->s_inodes = (struct inode *)phys_mem_alloc();
  tarfs_sb->s_inodes = (struct inode *)((uint64_t)tarfs_sb->s_inodes | KERNBASE);

  my_memset((void *)tarfs_sb->s_inodes, 0, PAGE_BLOCK_SIZE);

  d_root = create_root_dentry();

  tarfs_sb->s_root = d_root;
  tarfs_sb->s_magic = 100;

  struct posix_header_ustar *tarfs_fs = (struct posix_header_ustar *)&_binary_tarfs_start;
  char *temp = (char *)tarfs_fs;
  
  while(my_memcmp(temp + 257, "ustar", 5) == 0)
  {
    filesize = oct2dec((unsigned char *)(temp + 124), 11);

    struct inode tarfs_inode = {0};
    tarfs_inode.i_number = inode_num;   /* start from 3 */
    
    my_memcpy((void *)tarfs_inode.i_typeflag, (void *)(temp + 156), 1);
    tarfs_inode.i_size = filesize;  // only this works //

    if(tarfs_inode.i_typeflag[0] == '0')
      filetype = NORMAL;
    else if(tarfs_inode.i_typeflag[0] == '1')
      filetype = HARD_LINK;
    else if(tarfs_inode.i_typeflag[0] == '2')
      filetype = SYM_LINK;
    else if(tarfs_inode.i_typeflag[0] == '3')
      filetype = CHARDEV;
    else if(tarfs_inode.i_typeflag[0] == '4')
      filetype = BLKDEV;
    else if(tarfs_inode.i_typeflag[0] == '5')
      filetype = DIRECTORY;
    else filetype = NAMED_PIPE;

    if(filetype == NORMAL)
      tarfs_inode.i_data = temp + 512;
    else
      tarfs_inode.i_data = NULL;

    tarfs_inode.i_sb = tarfs_sb;
    
    /* inode operations */
    tarfs_inode.i_op = (struct inode_operations *)phys_mem_alloc();
    tarfs_inode.i_op = (struct inode_operations *)((uint64_t)tarfs_inode.i_op | KERNBASE);

    (tarfs_inode.i_op)->create = NULL;
    (tarfs_inode.i_op)->lookup = NULL;
    (tarfs_inode.i_op)->link = NULL;
    (tarfs_inode.i_op)->unlink = NULL;
    (tarfs_inode.i_op)->symlink = NULL;
    (tarfs_inode.i_op)->mkdir = NULL;
    (tarfs_inode.i_op)->rmdir = NULL;
    (tarfs_inode.i_op)->rename = NULL;
    (tarfs_inode.i_op)->permission = NULL;
    
    /* insert into list of inodes for the superblock */
    my_memcpy(name, temp, 100);  // sizeof(name)
    
    struct dentry *tarfs_dentry = dentry_for_this_inode(name);

    //kprintf("Name: %s Parent: %s ", tarfs_dentry->d_name, tarfs_dentry->d_parent->d_name);
    tarfs_inode.i_dentry = tarfs_dentry;

    insert_inode_into_superblock(tarfs_inode);
    insert_dentry_into_dentry_cache(inode_num, tarfs_dentry); 

    inode_num += 1;
    temp += (((filesize + 511) / 512) + 1) * 512;
  }
  return;
}

void insert_dentry_into_dentry_cache(int inode_num, struct dentry *tarfs_dentry)
{
  int i = 0;
  while(dentry_cache_array[i].inode_num != 0)
  {
    i++;
  }
  dentry_cache_array[i].inode_num = inode_num;
  dentry_cache_array[i].dentry_elem = tarfs_dentry;
  return;
}

struct dentry *check_dentry_cache(char *dentry_name)
{
  int i = 0;
  for(; i < MAX_DENTRIES_IN_CACHE; i++)
  {
    if(my_memcmp((void *)(dentry_cache_array[i].dentry_elem)->d_name, (void *)dentry_name, sizeof(dentry_name)) == 0)
    {
      return dentry_cache_array[i].dentry_elem;
    }
  }
  return NULL;
}

/* make dentry for inode */
struct dentry *dentry_for_this_inode(char *name)
{
  struct dentry *tarfs_dentry = (struct dentry *)phys_mem_alloc();
  tarfs_dentry = (struct dentry *)((uint64_t)tarfs_dentry | KERNBASE);

  my_memset((void *)tarfs_dentry, 0, sizeof(struct dentry));

  my_memcpy((void *)tarfs_dentry->d_name, (void *)name, 100);

  tarfs_dentry->d_parent = get_parent_dentry(name);  /* TODO:Investigate */
  
  tarfs_dentry->d_op = (struct dentry_operations *)phys_mem_alloc();
  tarfs_dentry->d_op = (struct dentry_operations *)((uint64_t)tarfs_dentry->d_op | KERNBASE);

  tarfs_dentry->d_op->d_compare = NULL;
  tarfs_dentry->d_op->d_delete = NULL;

  return tarfs_dentry;
}

struct dentry *get_parent_dentry(char *name)
{
  struct name *temp;

  char *parent;
  int i;
  temp = get_parent_and_child_directories(name);
  //kprintf("%s %s %s ", name, temp->parent, temp->child); 
  if(temp->parent != NULL)
  {
    /*
    if(my_strchr(temp->parent, '/') != NULL)
    {
      new_temp = get_parent_and_child_directories(temp->parent);
      parent = new_temp->child;
    }
    else
    {*/
      parent = temp->parent;
    /*}*/
  }
  else parent = temp->parent;
  if(parent == NULL)
  {
    return NULL;
  }
  /* find parent dentry whose name is "parent" */
  struct inode *i_list = tarfs_sb->s_inodes;
  i = 0;
  while(i_list[i].i_number != 0)
  {
    if(my_memcmp((i_list[i].i_dentry)->d_name, parent, my_strlen(parent)) == 0)
    {
      return i_list[i].i_dentry;
    }
    i++;
  }
  return NULL; 
}

struct inode get_inode_from_dentry(struct dentry *f_dentry)
{
  struct inode *i_list = tarfs_sb->s_inodes;
  struct inode bare_inode = {0};
  int i;
  i = 0;
  while(i_list[i].i_number != 0)
  {
    if(my_memcmp((void *)i_list[i].i_dentry->d_name, (void *)f_dentry->d_name, my_strlen(f_dentry->d_name)) == 0)
    {
      return i_list[i];
    }
    i++;
  }
  return bare_inode;
}

void insert_inode_into_superblock(struct inode tarfs_inode)
{
  struct inode *inode_list = tarfs_sb->s_inodes;
  int i = 0;
  while(inode_list[i].i_number != 0)
  {
    i++;
  }
  /* deep copy */
  inode_list[i].i_number = tarfs_inode.i_number;
  inode_list[i].i_size = tarfs_inode.i_size;
  my_memcpy((void *)inode_list[i].i_typeflag, (void *)tarfs_inode.i_typeflag, 1);

  inode_list[i].i_data = tarfs_inode.i_data;
  inode_list[i].i_sb = tarfs_inode.i_sb;
  inode_list[i].i_op = tarfs_inode.i_op;
  inode_list[i].i_dentry = tarfs_inode.i_dentry;

  return;
}

struct inode *tarfs_alloc_inode()
{
  struct inode *tarfs_inode = (struct inode *)phys_mem_alloc();
  tarfs_inode = (struct inode *)((uint64_t)tarfs_inode | KERNBASE);

  return tarfs_inode; 
}

void tarfs_destroy_inode(struct inode *tarfs_inode)
{
  phys_mem_free((uint64_t)tarfs_inode);
  return;
}

struct file *get_file_from_fd(int fp)
{
  struct pcb *curr;
  curr = get_current_process();
  return curr->fd[fp];
}

struct dentry *do_find_dentry(char *pathname, int flags)
{
  struct dentry *d_entry;
  int relative = 1;
  char *temp_pathname;

  //char *temp_dest = (char *)phys_mem_alloc();
  //temp_dest = (char *)((uint64_t)temp_pathname | KERNBASE);
  //my_memset((void *)temp_dest, 0, PAGE_BLOCK_SIZE);

  if(pathname[0] == '/')
  {
    relative = 0;
  }
  if(relative)
  {
    temp_pathname = my_strcat(get_current_process()->cwd, pathname);
  }
  else
  {
    temp_pathname = (char *)phys_mem_alloc();
    temp_pathname = (char *)((uint64_t)temp_pathname | KERNBASE);
    my_memset((void *)temp_pathname, 0, PAGE_BLOCK_SIZE);
    my_memcpy(temp_pathname, pathname, my_strlen(pathname));
  }
  /* return dentry corresponding to the actual file */
  d_entry = check_correct_file(temp_pathname + 1);
  return d_entry;
}

struct dentry *check_correct_file(char *filename)
{
  int i = 0;
  for(; i < MAX_DENTRIES_IN_CACHE; i++)
  {
    if(dentry_cache_array[i].inode_num == 0) break;
    else
    {
      if(my_memcmp((void *)dentry_cache_array[i].dentry_elem->d_name, (void *)filename, my_strlen(filename)) == 0)
      {
        return dentry_cache_array[i].dentry_elem;
      }
    }
  }
  return NULL;  /* dentry ain't found */  
}


/* TODO: did you insert root into dentry_cache - YES */

struct dentry *create_root_dentry()
{
  struct dentry *d_root;
  d_root = (struct dentry *)phys_mem_alloc();
  d_root = (struct dentry *)((uint64_t)d_root | KERNBASE);
 
  my_memset((void *)d_root, 0, sizeof(struct dentry));

  my_memcpy((void *)d_root->d_name, "/", my_strlen("/"));

  struct inode d_inode = {0};
  
  d_inode.i_number = 2;
  //insert_inode_into_superblock(d_inode);
  d_root->d_parent = NULL;

  d_root->d_op = (struct dentry_operations *)phys_mem_alloc();
  d_root->d_op = (struct dentry_operations *)((uint64_t)d_root->d_op | KERNBASE);

  d_root->d_op->d_compare = NULL;
  d_root->d_op->d_delete = NULL;

  d_inode.i_dentry = d_root;

  insert_inode_into_superblock(d_inode);
  insert_dentry_into_dentry_cache(2, d_root);
  return d_root;
}

ssize_t tarfs_read(struct file *fp, char *buf, size_t size, off_t offset)
{
  struct dentry *fp_dentry = fp->f_dentry;
  ssize_t copy_size = size;
  struct inode fp_inode = get_inode_from_dentry(fp_dentry);
  
  int filesize = fp_inode.i_size;
  if(offset + size >= filesize)
  {
    copy_size = (ssize_t)(filesize-offset);
  }
  
  my_memcpy((void *)buf, (void *)(fp_inode.i_data + offset), copy_size);
  fp->f_offset += copy_size;

  return copy_size;   /* how many bytes read */
}


size_t sys_getdents(int fd, char *data, int size)
{
  char *temp = data;
  my_memset((void *)temp, 0, size);
  struct file *fp = get_file_from_fd(fd);
  //struct dentry_cache null_entry = {0};
  int index = 0;
  int n_written = 0;
  if(fp->f_flags != FS_DIRECTORY)
  {
    return 0;
  }
  struct dentry *ex_dentry = fp->f_dentry;
  for(; dentry_cache_array[index].inode_num != 0; index++)
  {
    if(n_written > size)
    {
      return size;
    }
    if(dentry_cache_array[index].dentry_elem->d_parent->d_name == NULL)
    {
      continue;
    }
    else
    {
      if(my_memcmp((void *)ex_dentry->d_name, (void *)dentry_cache_array[index].dentry_elem->d_parent->d_name, my_strlen((void *)ex_dentry->d_name)) == 0)
      {
        my_memcpy((void *)(temp + n_written), (void *)(dentry_cache_array[index].dentry_elem->d_name), my_strlen(dentry_cache_array[index].dentry_elem->d_name));
        n_written += 32;
      }
    }
  }
  return n_written;
}

/* return_child_dentry_cache_entry */

/**
the dentry cache stores the parent and child dentries together along with the inode numbers of the child - this will help while using getdents - it will lookup the child dentries and return for a given parent which is the directory name - usage of dentry cache may not be correct conceptually, but should work here anyway...
**/

struct dentry_cache return_child_dentry_cache_entry(struct dentry *cmp_dentry)
{
  static int i = 0;
  int index;
  struct dentry_cache null_entry = {0};
  for(index = i; index < MAX_DENTRIES_IN_CACHE; index++)
  {
    if(dentry_cache_array[index].dentry_elem->d_parent->d_name == NULL)
    {
      continue;
    }
    if(my_memcmp((void *)cmp_dentry->d_name, (void *)dentry_cache_array[index].dentry_elem->d_parent->d_name, my_strlen(cmp_dentry->d_name)) == 0)
    {
      i = index;
      return dentry_cache_array[index];
    }  
  }
  return null_entry;
}

