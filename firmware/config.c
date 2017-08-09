
//MIT License
//
//Copyright (c) 2017 tvelliott
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include <stdint.h>
#include <string.h>
#include "crc.h"
#include "global.h"
#include "config.h"
#include "stm32f4xx_flash.h"
#include "stm32f4xx_iwdg.h"
#include "std_io.h"
#include "flash_sst.h"

sys_config config;

//////////////////////////////////////////////////
//////////////////////////////////////////////////
void config_changed()
{
  config.mac_addr[0] = 0x03;
  config.mac_addr[1] = 0x01;
  config.mac_addr[2] = 0x02;
  config.mac_addr[3] = 0x03;
  config.mac_addr[4] = 0x04;
  config.mac_addr[5] = 0x05;

}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0)) {
    sector = FLASH_Sector_0;
  } else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1)) {
    sector = FLASH_Sector_1;
  } else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2)) {
    sector = FLASH_Sector_2;
  } else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3)) {
    sector = FLASH_Sector_3;
  } else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4)) {
    sector = FLASH_Sector_4;
  } else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5)) {
    sector = FLASH_Sector_5;
  } else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6)) {
    sector = FLASH_Sector_6;
  } else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7)) {
    sector = FLASH_Sector_7;
  } else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8)) {
    sector = FLASH_Sector_8;
  } else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9)) {
    sector = FLASH_Sector_9;
  } else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10)) {
    sector = FLASH_Sector_10;
  } else { /*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
    sector = FLASH_Sector_11;
  }

  return sector;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void read_config()
{
  static uint8_t *ptr = (uint8_t *) FLASH_USER_START_ADDR;

  //validate configuration or revert to defaults
  if ( check_struct_crc( (uint8_t *) FLASH_USER_START_ADDR, sizeof(sys_config)) ) {
    reset_config_to_defaults();
    factory_defaults();
    write_config(&config);
  } else {
    memcpy(&config, ptr, sizeof(sys_config) );
  }

  config_changed();
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
void write_config(sys_config *configtmp)
{
  uint32_t StartSector = 0, EndSector = 0, Address = 0, i = 0 ;
  __IO uint32_t data32 = 0 , MemoryProgramStatus = 0 ;


  //validate configuration
  update_struct_crc( (uint8_t *)configtmp, sizeof(sys_config) );

  FLASH_Unlock();

  // Clear pending flags (if any)
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);

  StartSector = GetSector(FLASH_USER_START_ADDR);
  //not used
  EndSector = GetSector(FLASH_USER_END_ADDR);

  //erase sector
  if (FLASH_EraseSector(StartSector, VoltageRange_3) != FLASH_COMPLETE) {
    //error if we get here
  }

  int config_size_w32 = (sizeof(sys_config)/4)+8; //plus a few extra
  uint32_t *ptr = (uint32_t *) configtmp;

  Address = FLASH_USER_START_ADDR;
  //write configuration to flash in 32 bit words
  for(i=0; i<config_size_w32; i++) {
    if (FLASH_ProgramWord(Address, *ptr++) != FLASH_COMPLETE) {
      //error if we get here
    } else {
      Address+=4;
    }
  }

  FLASH_Lock();

}

///////////////////////////////////////////////////////
// doesn't reset Everything, called by factory command
///////////////////////////////////////////////////////
void factory_defaults(void)
{

  config.tcp_mss = 1460;
  config.tcp_win = 1460;
  config_changed();
}


///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void reset_config_to_defaults(void)
{

  memset( (uint8_t *) &config, 0x00, sizeof(sys_config) );


  config.mac_addr[0] = 0x00;
  config.mac_addr[1] = 0x04;
  config.mac_addr[2] = 0x3f;
  config.mac_addr[3] = 0x1c;
  config.mac_addr[4] = 0x9e;

  config.mac_addr[5] = 0xfa;

  config.ip_addr[0] = 192;
  config.ip_addr[1] = 168;
  config.ip_addr[2] = 1;
  config.ip_addr[3] = 240;

  config.gw_addr[0] = 192;
  config.gw_addr[1] = 168;
  config.gw_addr[2] = 1;
  config.gw_addr[3] = 1;

  config.netmask[0] = 255;
  config.netmask[1] = 255;
  config.netmask[2] = 255;
  config.netmask[3] = 0;

  config.tftp_serv_ip[0] = 172;
  config.tftp_serv_ip[1] = 18;
  config.tftp_serv_ip[2] = 9;
  config.tftp_serv_ip[3] = 2;

  config.tcp_mss = 1460;
  config.tcp_win = 1460;
  config.tcp_sack = 0;

  config_changed();
}
