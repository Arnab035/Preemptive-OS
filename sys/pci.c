/******************************************************************

***    PCI CONFIGURATION SPACE ***********

*******************************************************************/

// portions of pseudocode has been copied from OSDEV

#include <sys/pci.h>
#include <sys/kprintf.h>
#include <sys/ahci.h>

// TODO: brute force scan of 32 devices on each of the 256 buses

int done=0;

void check_all_buses()
{
  uint8_t bus;
  uint8_t device;
  for(bus=0;bus<256;bus++){
    for(device=0;device<32;device++){
      if(done == 0)
        check_device(bus, device);
    }
  }
}

/*****************************************************************
  // CONFIG_ADDRESS REGISTER

31 	        30 - 24 	23 - 16 	15 - 11 	10 - 8 	                      7 - 2 	      1 - 0
Enable Bit 	Reserved 	Bus Number 	Device Number 	Function Number 	Register Number 	00 
*****************************************************************/

void check_device(uint8_t bus, uint8_t device){
  uint8_t function = 0;  // only 8 functions possible
  uint16_t vendor_details;
  uint8_t header;
  vendor_details = check_vendor_id(bus, device, function);
  if(vendor_details == 0xFFFF) return;
  check_function(bus, device, function);
  header = check_header(bus, device, function, 12);  // offset 0C
  if((header & 0x80) == 1) { // bit 7 is 1
    // has multiple functions
    for(function = 1; function < 8; function++){
        if(check_vendor_id(bus, device, function) != 0xFFFF)   
    	   check_function(bus, device, function);
    }
  }
}

uint8_t check_header(uint8_t bus, uint8_t device, uint8_t function, uint8_t regnum){
  // you have to read the config address register again
  uint32_t address;
  uint8_t ret = 0;
  address = ENABLE;
  address |= bus << 16;
  address |= device << 11;
  address |= function << 8;
  address |= regnum & 0xfc; 
  outbig(PCI_CONFIG_ADDRESS, address);
  // 3rd byte of 4 bytes
  ret = (uint8_t)((inbig(PCI_CONFIG_DATA) >> 16) & 0xff); // mask other bits
  return ret; 
}

void check_function(uint8_t bus, uint8_t device, uint8_t function){
  uint8_t base_class;
  uint8_t sub_class;
  uint8_t prog_if; 
  base_class = get_base_class(bus, device, function, 8);  // register number
  sub_class = get_sub_class(bus, device, function, 8);
  if(base_class == 0x01 && sub_class == 0x06){
    // you are almost there
    prog_if = get_prog_if(bus, device, function, 8);
    if(prog_if == 0x01){
      //kprintf("You have found the AHCI controller in bus %d and device %d with base class %d and sub class %d\n", bus, device, base_class, sub_class);//
      uint64_t bar5 = read_bar(bus, device, function, 36);
      kprintf("The bar5 value is 0x%x\n", bar5);
      bar5 >>= 2;
      write_bar(bus, device, function, 36, bar5);
      uint64_t newbar5 = read_bar(bus, device, function, 36);
      kprintf("The new bar5 is 0x%x\n", newbar5);
      find_first_disk((hba_mem_t *)newbar5);
      done=1;
    }      
  }
}

void write_bar(uint8_t bus, uint8_t device, uint8_t function, uint8_t regnum, uint32_t bar)
{
  uint32_t address;
  address = ENABLE;
  address |= bus << 16;
  address |= device << 11;
  address |= function << 8;
  address |= regnum & 0xfc;
  outbig(PCI_CONFIG_ADDRESS, address);
  outbig(PCI_CONFIG_DATA, bar);
  //ret = (uint32_t)(inbig(PCI_CONFIG_DATA) & 0xffffffff);
  //kprintf("new bar5 is : 0x%x\n", ret);
}

uint32_t read_bar(uint8_t bus, uint8_t device, uint8_t function, uint8_t regnum){
  uint32_t address;
  uint32_t ret = 0;
  address = ENABLE;
  address |= bus << 16;
  address |= device << 11;
  address |= function << 8;
  address |= regnum & 0xfc;
  outbig(PCI_CONFIG_ADDRESS, address);
  ret = (uint32_t)(inbig(PCI_CONFIG_DATA) & 0xffffffff);
  return ret;
}


uint8_t get_base_class(uint8_t bus, uint8_t device, uint8_t function, uint8_t regnum){
  uint32_t address;
  uint8_t ret = 0;
  address = ENABLE;
  address |= bus << 16;
  address |= device << 11;
  address |= function << 8;
  address |= regnum & 0xfc;
  outbig(PCI_CONFIG_ADDRESS, address);
  // 4th byte of 4 bytes
  ret = (uint8_t)((inbig(PCI_CONFIG_DATA) >> 24) & 0xff);
  return ret;
}

uint8_t get_sub_class(uint8_t bus, uint8_t device, uint8_t function, uint8_t regnum){
  uint32_t address;
  uint8_t ret = 0;
  address = ENABLE;
  address |= bus << 16;
  address |= device << 11;
  address |= function << 8;
  address |= regnum & 0xfc;
  outbig(PCI_CONFIG_ADDRESS, address);
  // 3rd byte of 4 bytes
  ret = (uint8_t)((inbig(PCI_CONFIG_DATA) >> 16) & 0xff);
  return ret;
}

uint8_t get_prog_if(uint8_t bus, uint8_t device, uint8_t function, uint8_t regnum){
  uint32_t address;
  uint32_t ret = 0;
  address = ENABLE;
  address |= bus << 16;
  address |= device << 11;
  address |= function << 8;
  address |= regnum & 0xfc;
  outbig(PCI_CONFIG_ADDRESS, address);
  // 2nd byte of 4 bytes
  ret = (uint8_t)((inbig(PCI_CONFIG_DATA) >> 8) & 0xff);
  return ret;
}

// When a configuration access attempts to select a device that does not exist, the host bridge will complete the access without error, 
// dropping all data on writes and returning all ones on reads.

uint16_t check_vendor_id(uint8_t bus, uint8_t device, uint8_t function){
  // choose the first register among the 64 registers available in PCI configuration space-
  // if the first register fails, that clearly means the device is not available
  return read_from_config_address_register(bus, device, function, 0);
}

uint16_t read_from_config_address_register(uint8_t bus, uint8_t device, uint8_t function, uint8_t regnum){
  uint32_t address;
  uint16_t ret = 0;
  address = ENABLE;
  address |= bus << 16;
  address |= device << 11;
  address |= function << 8;
  address |= regnum & 0xfc;  // last 2 bits are 0
  outbig(PCI_CONFIG_ADDRESS, address);
  ret =  (uint16_t)(inbig(PCI_CONFIG_DATA) >> ((regnum & 2) * 8) & 0xffff) ;
  return ret;
}
