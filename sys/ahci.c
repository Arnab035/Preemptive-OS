
// ahci controller handler functions ***************

//**************** tasks to be performed ******************** 

// 1. find the first disk connected to ahci.
// 2. Write 100 4KB blocks to the disk, fill each block with corresponding byte
// 3. (block 0 filled with byte 0, block 1 filled with byte 1, etc.)
// 4. Read the data back to verify it was written to disk correctly


// hba_mem_t->pi will contain which port is implemented already
// find sata drives which are present among one of these


#include <sys/ahci.h>
#include <sys/defs.h>
#include <sys/kprintf.h>
#include <sys/idt.h>

#define AHCI_BASE 0x3FC0000

void find_first_disk(hba_mem_t *hba){
  uint32_t ports = hba->pi;
  //kprintf("ports : %d\n", ports);
  //kprintf("hba_mem_t : 0x%x\n", hba);
  //kprintf("%c\n", a);
  int i = 0;
  for(; i < 32; i++){
    if(ports & 1){
      int disk_type = find_type(&hba->ports[i]);
      //kprintf("disk_type is 0x%x\n", disk_type);
      switch(disk_type)     
      {
        case AHCI_DEV_SATA:
          //kprintf("found the sata disk at port %d\n", i);
          // from @atif, enable AHCI, interrupts and reset
          hba->ghc |= 1 << 31;  // enable ahci
          hba->ghc |= 1 << 1;
          hba->ghc |= 1 << 0;
          // write to second disk - first disk is at 0, second at 1
          port_rebase(&(hba->ports[i]), i);
            
          char *write_buf = (char *)0x600f000;  // write buffer
  	  // try with this physaddr
  	  int m,j;
  	  int k = 0;
  	  for(m = 0; m < 100; m++)
  	  {
    	    for(j = 0; j < 100; j++)
    	    {
      		write_buf[k + j] = m + '0';
    	    }
    	    k += 100;
  	  }
  // write 800 sectors starting from 0 to 799 -- each sector is 512 bytes in QEMU
  // you are writing 4kb * 100 = 400 kb data -- which is 409600/512 sectors -- figure it out
  	  int res = do_write_to_disk(&hba->ports[i], 0, 0, 800, write_buf);
  	  if(res == 0)
          {
    	     kprintf("error while writing disk\n");
  	  }
          //kprintf("Writing done successfully\n");
  	  char *read_buf = (char *)0x6040000;
  	  for(m = 0; m < 10000; m++) read_buf[m] = 0; 
  	  res = do_read_from_disk(&hba->ports[i], 0, 0, 800, read_buf);
  // read 800 sectors starting from 0 to 799 -- each sector is 512 bytes in QEMU
           if(res == 0)
  	   {
    	     kprintf("error while reading disk\n");
  	   }
          for(m = 0; m < 10000; m++)
          {
            kprintf("%d",read_buf[m]); 
          }
          kprintf("Reading done successfully\n");
          break;
        case AHCI_DEV_SATAPI:
          break;
        case AHCI_DEV_SEMB:
          break;
        case AHCI_DEV_PM:
          break;
        default:
          kprintf("Wrong disk type\n");
      }
    }
    ports >>= 1;
  }
}

int find_type(hba_port_t *port){
  uint32_t ssts = port->ssts;
  uint8_t ipm = (uint8_t)((ssts >> 8) & 0x0f);
  uint8_t det = (uint8_t)(ssts & 0x0f);

  if(ipm != 0x01) return 0;  // interface not active
  if(det != 0x03) return 0;  // device not detected

  switch(port->sig)
  {
    case SATA_SIG_ATAPI:
      return AHCI_DEV_SATAPI;
    case SATA_SIG_SEMB:
      return AHCI_DEV_SEMB;
    case SATA_SIG_PM:
      return AHCI_DEV_PM;
    default:
      return AHCI_DEV_SATA;
  } 
}

// TODO: write 100 4kb blocks to the disk - fill each block with corresponding byte
// that is, byte 0 goes to block 0, byte 1 goes to block 1....

int do_write_to_disk(hba_port_t *port, uint32_t startl, uint32_t starth, uint16_t count, char *buf){
  port->is_rwc = -1;   // all interrupts cleared
  int spin=0;
  // remember the below command header is actually representing a command list - for easy pointer operations we abstract it to a cmd header
  // TODO: find slots
  int slot = find_slot(port); // one more parameter ?;
  if(slot == -1)
  {
    kprintf("No slot available\n");
    return -1;
  }
  hba_cmd_header_t *command_header = (hba_cmd_header_t *)port->clb;
  command_header += slot; 
  // advance the cmd header pointer by the slot number
  command_header->cfl = sizeof(fis_reg_h2d_t)/4;   // size of double word
  command_header->w = 1;   // write
  command_header->c = 1;
  command_header->prdtl = ((count-1) >> 4) + 1;  // 100 4kb blocks
  
  hba_cmd_tbl_t *command_header_table = (hba_cmd_tbl_t *)(command_header->ctba);

   my_memset(command_header_table, 0, sizeof(hba_cmd_tbl_t) + command_header->prdtl * sizeof(hba_prdt_entry_t));

  int i;

  for(i = 0; i < (command_header->prdtl)-1 ;i++)
  {
    (command_header_table->prdt_entry[i]).dba = (uint64_t)buf;
    //(command_header_table->prdt_entry[i]).dbau = (uint32_t)(( >> 32) & 0xffffffff) ;   
    (command_header_table->prdt_entry[i]).dbc = (8 * 1024)-1 ;  // 16 sectors = 8k at a time
    (command_header_table->prdt_entry[i]).i = 0;   // interrupt on completion
    count -= 16;
    buf += 4 * 1024;
  } 

  // final sector to write
  (command_header_table->prdt_entry[i]).dba = (uint64_t)buf;
  (command_header_table->prdt_entry[i]).dbc = count << 9;  // count is 1, 2^9
  (command_header_table->prdt_entry[i]).i = 0;
  
  // construct the command
  fis_reg_h2d_t* fis_h2d = (fis_reg_h2d_t *)(&command_header_table->cfis);
  fis_h2d->fis_type = FIS_TYPE_REG_H2D;
  fis_h2d->c = 1;
  fis_h2d->command = DMA_WRITE_EXT;

  fis_h2d->lba0 = (uint8_t)((startl) & 0xff);
  fis_h2d->lba1 = (uint8_t)((startl >> 8) & 0xff);
  fis_h2d->lba2 = (uint8_t)((startl >> 16) & 0xff);
  fis_h2d->device = CMD_FIS_DEV_LBA ;
  fis_h2d->lba3 = (uint8_t)((startl >> 24) & 0xff);
  fis_h2d->lba4 = (uint8_t)((starth) & 0xff);
  fis_h2d->lba5 = (uint8_t)((starth >> 8) & 0xff);
  
  fis_h2d->count = count;  
  // TODO: wait for write to complete
  while ((port->tfd & (ATA_STATUS_BSY | ATA_STATUS_DRQ)) && spin < 1000000)
  {
		spin++;
  }
  if (spin == 1000000)
  {
	kprintf("Port is hung\n");
	return 0;
  }
 
  port->ci = 1<<slot;	// Issue command
 
	// Wait for completion
  while (1)
  {
	if ((port->ci & (1<<slot)) == 0) 
		break;
	if (port->is_rwc & HBA_PxIS_TFES)	// Task file error
	{
                kprintf("sata status: %d\n", port->ssts);
		kprintf("Write disk error\n");
		return 0;
	}
  }
 
	// Check again
  if (port->is_rwc & HBA_PxIS_TFES)
  {
        kprintf("sata status: %d\n", port->ssts); 
	kprintf("Write disk error\n");
	return 0;
  }
 
	return 1;
}

int do_read_from_disk(hba_port_t *port, uint32_t startl, uint32_t starth, uint16_t count, char *buf){
  port->is_rwc = -1;
  int spin = 0;
  // TODO: find slots
  int slot = find_slot(port);
  if(slot == -1)
  {
    kprintf("no slot available\n");
    return 0;
  }
  hba_cmd_header_t *command_header = (hba_cmd_header_t *)port->clb;
  command_header += slot;

  command_header->cfl = sizeof(fis_reg_h2d_t)/4;
  command_header->w = 0;
  command_header->prdtl = ((count-1) >> 4) + 1;

  hba_cmd_tbl_t *command_header_table = (hba_cmd_tbl_t *)(command_header->ctba);

  my_memset(command_header_table, 0, sizeof(hba_cmd_tbl_t) + command_header->prdtl * sizeof(hba_prdt_entry_t));
  
  int i;
  for(i=0; i < (command_header->prdtl)-1 ; i++)
  {
    (command_header_table->prdt_entry[i]).dba = (uint64_t)buf;
    //(command_header_table->prdt_entry[i]).dbau = (uint32_t)((buf >> 32) & 0xffffffff) ;
    (command_header_table->prdt_entry[i]).dbc = (8 * 1024)-1;
    (command_header_table->prdt_entry[i]).i = 0;   // INTERRUPT ON COMPLETION
    count -= 16;
    buf += 4 * 1024;
  }
  
  (command_header_table->prdt_entry[i]).dba = (uint64_t)buf;
  (command_header_table->prdt_entry[i]).dbc = count << 9;
  (command_header_table->prdt_entry[i]).i = 0;
  
  // construct the command
  fis_reg_h2d_t *fis_h2d = (fis_reg_h2d_t *)(&command_header_table->cfis);
  fis_h2d->fis_type = FIS_TYPE_REG_H2D;
  fis_h2d->c = 1;
  fis_h2d->command = DMA_READ_EXT;
 
  fis_h2d->lba0 = (uint8_t)((startl) & 0xff);
  fis_h2d->lba1 = (uint8_t)((startl >> 8) & 0xff);
  fis_h2d->lba2 = (uint8_t)((startl >> 16) & 0xff);
  fis_h2d->device = CMD_FIS_DEV_LBA;
  fis_h2d->lba3 = (uint8_t)((startl >> 24) & 0xff);
  fis_h2d->lba4 = (uint8_t)(starth & 0xff);
  fis_h2d->lba5 = (uint8_t)((starth >> 8) & 0xff);
  
  fis_h2d->count = count;
  // TODO: wait for read to complete
  while ((port->tfd & (ATA_STATUS_BSY | ATA_STATUS_DRQ)) && spin < 1000000)
  {
		spin++;
  }
  if (spin == 1000000)
  {
     kprintf("Port is hung\n");
     return 0;
  }
 
  port->ci = 1<<slot;	// Issue command
 
	// Wait for completion
  while (1)
  {
	if ((port->ci & (1<<slot)) == 0) 
		break;
	if (port->is_rwc & HBA_PxIS_TFES)	// Task file error
	{
	    kprintf("Read disk error\n");
	    return 0;
	}
  }
  if (port->is_rwc & HBA_PxIS_TFES)
  {
	kprintf("Read disk error\n");
	return 0;
  }
        return 1;
}

// TODO: WRITE CODE TO FIND A SLOT

int find_slot(hba_port_t *port)
{ 
  uint32_t free_slots = (port->sact | port->ci) ;
  int i=0;
  for(; i < 32; i++)
  {
    if((free_slots & 1) == 0)  // if not set in either sact and ci, slot is free
    {
      return i;
    }
    free_slots >>= 1;
  }
  kprintf("no free slot found\n");
  return -1;
}


// TODO: determine AHCI BASE

/*
BIOS may have already configured all the necessary AHCI memory spaces. But the OS usually needs to reconfigure them to make them fit its requirements. It should be noted that Command List must be located at 1K aligned memory address and Received FIS be 256 bytes aligned.

Before rebasing Port memory space, OS must wait for current pending commands to finish and tell HBA to stop receiving FIS from the port. Otherwise an accidently incoming FIS may be written into a partially configured memory area. This is done by checking and setting corresponding bits at the Port Command And Status register (HBA_PORT.cmd). The example subroutines stop_cmd() and start_cmd() do the job.
*/

void port_rebase(hba_port_t *port, int portno)
{
  kprintf("Port rebase called\n");
  stop_cmd(port);
  port->clb = AHCI_BASE + (portno<<10);
  kprintf("port->clb is 0x%x\n", port->clb);

  my_memset((void *)(port->clb), 0, 1024);
  
  // fb is at 32k + 256 * portno from the ahci base -- 32k since you have the 32 command headers each  // of 32k
  port->fb = AHCI_BASE + (32 << 10) + (portno << 8);
  my_memset((void *)port->fb, 0, 1024);

  //command header table is at offset 40k + 8k * portno since 8k = 32 * 256 bytes for each port having a FIS struc  // ture with it

  hba_cmd_header_t *hba_header = (hba_cmd_header_t *)port->clb;
  
  int i=0;
  // TODO: can be modified also
  for(; i < 32; i++)
  {
    hba_header[i].prdtl = 64;  // a larger value -- for 100 4k blocks
    // each command table will have a size of 128 + 64 * 32
    hba_header[i].ctba = AHCI_BASE + (40 << 10) + 36864 * portno + 1152 * i;
    my_memset((void *)(hba_header[i].ctba), 0, 1152); 
  }
  start_cmd(port);
  
}

// Start command engine
void start_cmd(hba_port_t *port)
{
	// Wait until CR (bit15) is cleared
  while (port->cmd & HBA_PxCMD_CR);
  port->cmd |= HBA_PxCMD_FRE;
  port->cmd |= HBA_PxCMD_ST; 
}
 
// Stop command engine
void stop_cmd(hba_port_t *port)
{
	// Clear ST (bit0)
  port->cmd &= ~HBA_PxCMD_ST;
  while(1)
  {
    if (port->cmd & HBA_PxCMD_FR)
	continue;
    if (port->cmd & HBA_PxCMD_CR)
	continue;
    break;
  }
  port->cmd &= ~HBA_PxCMD_FRE;
}

/*
void init_ahci(hba_mem_t *hba)
{
  int port = find_first_disk(hba);
  port_rebase(hba->pi, port);

  char *write_buf;  // write buffer
  // try with this physaddr
  write_buf = (char *)0x600f000;
  int i,j;
  int k = 0;
  for(i = 0; i < 100; i++)
  {
    for(j = 0; j < 100; j++)
    {
      write_buf[k + j] = i+'0';
      
    }
    k += 100;
  }
  // write 800 sectors starting from 0 to 799 -- each sector is 512 bytes in QEMU
  // you are writing 4kb * 100 = 400 kb data -- which is 409600/512 sectors -- figure it out
  int res = do_write_to_disk((hba_port_t *)hba->pi, 0, 799, 800, write_buf);
  if(res == -1)
  {
    kprintf("error while writing disk\n");
  }
  char *read_buf;  // read buffer
  read_buf = (char *)0x604f000;
  for(i = 0; i < 10000; i++) read_buf[i] = 0; 
  res = do_read_from_disk((hba_port_t *)hba->pi, 0, 799, 800, read_buf);
  // read 800 sectors starting from 0 to 799 -- each sector is 512 bytes in QEMU
  if(res == -1)
  {
    kprintf("error while reading disk\n");
  }
}
 */
