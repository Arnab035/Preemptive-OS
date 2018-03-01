#ifndef  __VFS_H
#define  __VFS_H

#include <sys/defs.h>
#include <dirent.h>
#include <sys/phys_mmgr.h>
#include <sys/stat.h>
#include <sys/process.h>
#include <sys/types.h>

/* file system flags */

#define FS_FILE       0x1
#define FS_DIRECTORY  0x2

/* to be added/modified as needed */

struct vm_map_entry *vma;
struct file *fp;    

struct name
{
  char *parent;
  char *child;
};

struct file_system_type
{
  const char *name;   /* file system name */
  int fs_flags;
  struct super_block *(*get_sb) (struct file_system_type *, int, char *, void *);
  /* read the superblock off the disk */
  void (*kill_sb) (struct super_block *);
  /* terminate superblock access */
};

struct super_block
{
  struct super_block_operations *s_op;
  struct dentry *s_root;
  struct inode *s_inodes;  /* pointer to inode list */
  unsigned long s_magic;   /* magic number - eg. for tarfs let it be 100 */
};

struct super_block_operations
{
  struct inode *(*alloc_inode)();
  void (*destroy_inode)(struct inode *);
  int (*sync_fs)(struct super_block *sb, int wait);  /* doubtful */
  int (*write_inode)(struct inode *, int );
  void (*delete_inode)(struct inode *);
  void (*put_super)(struct super_block *);
  void (*write_super)(struct super_block *);
};

/* remove unnecessary fields */

struct inode 
{
  unsigned long i_number;
  int  i_size;
  char i_typeflag[1];  
  char *i_data;             /* only useful for tarfs */
  struct super_block *i_sb;   /* associated superblock */
  struct inode_operations *i_op;
  struct dentry *i_dentry;   /* one inode -> one dentry */
};

struct address_space
{
  struct inode *a_host;
  unsigned long nrpages;
};

struct inode_operations
{
  int (*create)(struct inode *, struct dentry *, int mode);
  struct dentry *(*lookup)(struct inode *, struct dentry *);
  int (*link) (struct dentry *,struct inode *,struct dentry *);
  int (*unlink) (struct inode *,struct dentry *);
  int (*symlink) (struct inode *,struct dentry *,const char *);
  int (*mkdir) (struct inode *, struct dentry *, int );
  int (*rmdir) (struct inode *, struct dentry *);
  int (*rename) (struct inode *, struct dentry *, struct inode *, struct dentry *);
  int (*permission)(struct inode *, int );
};

struct dentry
{
  char d_name[100];  /* name */
  struct dentry *d_parent;
  struct dentry_operations *d_op;
};

struct dentry_cache
{ 
  int inode_num;
  struct dentry *dentry_elem;
};

struct dentry_operations
{
  int (*d_compare)(struct dentry *);
  int (*d_delete) (struct dentry *);
};

struct file
{
  uint64_t f_offset;
  uint64_t f_refcount;
  mode_t f_mode;
  unsigned int f_flags;
  struct dentry *f_dentry;
  struct file_operations *f_op;
};

struct file_operations
{
  int (*open) (struct inode *, struct file *);  /* most fs use default open */
  int (*mmap) (struct file *, struct vm_map_entry *);
  ssize_t (*read) (struct file *, char *, size_t, off_t );
  ssize_t (*write) (struct file *, char *, size_t, off_t );
  off_t (*lseek) (struct file *file, off_t offset, int origin);
  int (*close) (struct file *file);
};

ssize_t vfs_read(struct file *, void *, size_t, off_t );

ssize_t vfs_write(struct file *, void *, size_t ,off_t);

uint32_t vfs_open(struct inode *, uint64_t);

uint32_t vfs_close(struct file *);

#endif
