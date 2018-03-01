/********************** all about elf **********************/

#include <sys/elf64.h>

uint16_t check_elf_magic(Elf64_Ehdr *elf_hdr)
{ 
  if(!elf_hdr) return 0;
  if(elf_hdr->e_ident[0] != ELFMAG0)
  {
    kprintf("first byte of magic invalid\n");
    return 0;
  }
  if(elf_hdr->e_ident[1] != ELFMAG1)
  {
    kprintf("second byte of magic invalid\n");
    return 0;
  }
  if(elf_hdr->e_ident[2] != ELFMAG2)
  {
    kprintf("third byte of magic invalid\n");
    return 0;
  }
  if(elf_hdr->e_ident[3] != ELFMAG3)
  {
    kprintf("fourth byte of magic invalid\n");
    return 0;
  }
  return 1;
}

uint16_t check_elf_file_validity(Elf64_Ehdr *elf_hdr)
{
  if(!check_elf_magic(elf_hdr)) return 0;
  if(elf_hdr->e_ident[4] != ELFCLASS64) 
  {
    kprintf("elf file belongs to different architecture\n");
    return 0;
  }
  if(elf_hdr->e_ident[5] != ELFDATA2LSB)
  {
    kprintf("elf file does not have correct byte order\n");  
    return 0;
  }
  /*
  if(elf_hdr->e_machine != EM_386)
  {
    kprintf("elf file has wrong processor identification\n");
    return 0;
  }*/
  return 1;
}

Elf64_Phdr *load_elf_phdr(Elf64_Ehdr *ehdr, struct file *file)
{
  char *phdr = NULL;
  int size;
  off_t offset = ehdr->e_phoff;

  size = sizeof(Elf64_Phdr) * ehdr->e_phnum;

  phdr = (char *)phys_mem_alloc();
  phdr = (char *)((uint64_t)phdr | KERNBASE);

  int bytes = vfs_read(file, phdr, size, offset);
  if(bytes < 0)
  {
    phys_mem_free((uint64_t)phdr);
    return NULL;
  }
  return (Elf64_Phdr *)phdr;
}

Elf64_Ehdr *load_elf_ehdr(struct file *file)
{
  char *ehdr = NULL;
  int size;

  size = sizeof(Elf64_Ehdr);

  ehdr = (char *)phys_mem_alloc();
  ehdr = (char *)((uint64_t)ehdr | KERNBASE);

  int bytes = vfs_read(file, (void *)ehdr, size, 0);
  if(bytes < 0)
  {
    phys_mem_free((uint64_t)ehdr);
    return NULL;
  }
  return (Elf64_Ehdr *)ehdr;
}

int load_elf_binary(struct mm_struct *mm)
{
  int i, err;
  struct file *filp = mm->file;
  Elf64_Phdr *phdr;
  Elf64_Phdr *ppnt;

  Elf64_Ehdr *ehdr = load_elf_ehdr(filp);
  
  if(!check_elf_file_validity(ehdr))
  {
    return -1;   // wrong elf format
  }
  mm->rip = ehdr->e_entry;   // entry point
  // map the entry point to a phys address ?
  phdr = load_elf_phdr(ehdr, filp);

  ppnt = phdr;
  for(i = 0, ppnt = phdr; i < (ehdr->e_phnum); i++, ppnt++)
  {
    if(ppnt->p_type == PT_LOAD)
    {
      err = elf_map(filp, ppnt);
      if(err < 0)
      {
        kprintf("error in mapping elf file\n");
        return err; 
      }
    }    
  }
  //kprintf("file backed mmap done\n");
  return 0;    // no PT_LOAD ???
}

uint64_t elf_map(struct file *file, Elf64_Phdr *phdr)
{
  uint64_t elf_offset = phdr->p_offset;
  uint64_t elf_prot = phdr->p_flags;
  uint64_t prot = 0;
  if(((elf_prot) & (ELF_READ)) != 0)
  {
    prot |= PROT_READ;
  }
  else if(((elf_prot) & (ELF_WRITE)) != 0)
  {
    prot |= PROT_WRITE;
  }
  
  else if(((elf_prot) & (ELF_EXECUTE)) != 0)
  {
    prot |= PROT_EXEC;
  }
  else prot = 0;
  uint64_t elf_vaddr = phdr->p_vaddr;
  uint64_t elf_filesz = phdr->p_filesz;
  uint64_t elf_memsz = phdr->p_memsz;

  return mmap_region(file, elf_vaddr, elf_filesz, elf_memsz, elf_offset, prot);
}

