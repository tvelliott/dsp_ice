
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

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "std_io.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "misc.h" //nvic stuff
#include "global.h"
#include "crc.h"
#include "config.h"
#include "flash_sst.h"

#define FLASH_READ 0x03
#define FLASH_HIGH_SPEED_READ 0x0b
#define FLASH_ERASE_4k 0x20
#define FLASH_ERASE_32k 0x52
#define FLASH_ERASE_64k 0xd8
#define FLASH_CHIP_ERASE 0x60
#define FLASH_BYTE_PGM 0x02
#define FLASH_AAI_PGM 0xad
#define FLASH_RDSR 0x05
#define FLASH_EWSR 0x50
#define FLASH_WRSR 0x01
#define FLASH_WREN 0x06
#define FLASH_WRDI 0x04
#define FLASH_RDID 0x90
#define FLASH_EBSY 0x70
#define FLASH_DBSY 0x80

#define FLASH_BUSY (1<<0)
#define FLASH_WEL (1<<1)
#define FLASH_BP0 (1<<2)
#define FLASH_BP1 (1<<3)
#define FLASH_BP2 (1<<4)
#define FLASH_BP3 (1<<5)
#define FLASH_AAI (1<<6)
#define FLASH_BPL (1<<7)

uint8_t manf_id[2];
static uint32_t flash_addr;
static uint8_t * flash_ptr;
static uint8_t do_flash_write;
static int32_t flash_len;

volatile int flash0_busy_flg;
volatile int flash1_busy_flg;
static int i;

static GPIO_InitTypeDef GPIO_InitStruct;
static SPI_InitTypeDef SPI_InitStructure;
static I2S_InitTypeDef I2SInitStruct;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static void flash_select(int id)
{

  if(id==0) GPIO_ResetBits(GPIOC, GPIO_Pin_8);
  if(id==1) GPIO_ResetBits(GPIOC, GPIO_Pin_9);

  if(id==0) flash0_busy_flg=1;
  if(id==1) flash1_busy_flg=1;


}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static void flash_deselect(int id)
{
  //

  if(id==0) GPIO_SetBits(GPIOC, GPIO_Pin_8);
  if(id==1) GPIO_SetBits(GPIOC, GPIO_Pin_9);

  if(id==0) flash0_busy_flg=0;
  if(id==1) flash1_busy_flg=0;


}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static uint8_t xmit_spi2(uint8_t out_reg)
{
  uint8_t in_reg;

  while( !SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) ) {
    do_main_tasks(1);
  }
  SPI_I2S_SendData(SPI3, out_reg);

  while( !SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) ) {
    do_main_tasks(1);
  }
  return (uint8_t)SPI_I2S_ReceiveData(SPI3);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void init_flash_sst()
{



  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);


  GPIO_StructInit(&GPIO_InitStruct);

//flash selects
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
  GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_SetBits(GPIOC, GPIO_Pin_7);
  GPIO_SetBits(GPIOC, GPIO_Pin_8);
  GPIO_SetBits(GPIOC, GPIO_Pin_9);

  GPIO_PinLockConfig(GPIOC, GPIO_Pin_7);
  GPIO_PinLockConfig(GPIOC, GPIO_Pin_8);
  GPIO_PinLockConfig(GPIOC, GPIO_Pin_9);


  SPI_I2S_DeInit(SPI3);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);


  GPIO_StructInit(&GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;

  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
  GPIO_Init(GPIOC, &GPIO_InitStruct);

  //GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
  GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);



  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = 0;
  SPI_InitStructure.SPI_CPHA = 0;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;

  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;

  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;

  SPI_Init(SPI3, &SPI_InitStructure);

  SPI_Cmd(SPI3, ENABLE);


  delay_ms(10);

  GPIO_PinLockConfig(GPIOC, GPIO_Pin_10);
  GPIO_PinLockConfig(GPIOC, GPIO_Pin_11);
  GPIO_PinLockConfig(GPIOC, GPIO_Pin_12);

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void wait_for_flash(int id)
{

  while( flash_isbusy(id) ) {
    do_main_tasks(1);
  }


}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint8_t * flash_get_manf_id(int id)
{

  //select flash
  flash_select(id);

  xmit_spi2( FLASH_RDID );
  xmit_spi2( 0x00 );
  xmit_spi2( 0x00 );
  xmit_spi2( 0x00 );

  manf_id[0] = xmit_spi2(0xff);

  //deselect flash
  flash_deselect(id);



  //select flash
  flash_select(id);

  xmit_spi2( FLASH_RDID );
  xmit_spi2( 0x00 );
  xmit_spi2( 0x00 );
  xmit_spi2( 0x01 );

  manf_id[1] = xmit_spi2(0xff);

  //deselect flash
  flash_deselect(id);

  return manf_id;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_unprotect_all(int id)
{



  //select flash
  flash_select(id);

  xmit_spi2( FLASH_EWSR ); //enable status register write

  //deselect flash
  flash_deselect(id);


  //select flash
  flash_select(id);

  xmit_spi2( FLASH_WRSR );
  xmit_spi2( 0x00 ); //disable write protect

  //deselect flash
  flash_deselect(id);

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_read(uint32_t address, uint8_t *buffer, int16_t len, int id)
{

  int16_t i;

  wait_for_flash(id);

  //select flash
  flash_select(id);

  xmit_spi2( FLASH_READ );

  xmit_spi2( (address >> 16) );
  xmit_spi2( (address >> 8) );
  xmit_spi2( (address & 0xff) );

  for (i=0; i<len; i++) {
    buffer[i] = xmit_spi2(0xff);
  }


  //deselect flash
  flash_deselect(id);


}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void test_flash(int id)
{
  int i;
  uint8_t *ptr = 0x00000000;
  uint8_t val;

  if(id!=0 && id!=1) id=0;

  printf("\r\nerasing flash, id: %d", id);
  do_main_tasks(1);

  //flash_erase_all(id);

  for(i=0; i<1024; i++) {
    printf("\r\nwriting 1k block %d", i);
    flash_write_blocking( (uint32_t) ptr, (uint8_t *) ptr, 1024, id);
    ptr+=1024;
    do_main_tasks(1);
  }

  ptr = 0x00000000;
  //verify from mcu to external flash
  for(i=0; i<1024*1024; i++) {
    flash_read(i, &val, 1, id);
    if(i%1024==0) printf("\r\nverifying block %d", i/1024);

    if(val != *ptr) {
      printf("\r\nproblem verifying flash: %02x, %02x", *ptr, val);
    }
    ptr++;
    do_main_tasks(1);
  }

  printf("\r\nverified okay.\r\n ");
  print_prompt();
  
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_write_blocking(uint32_t address, uint8_t *buffer, int32_t len, int id)
{

  int16_t i;

  wait_for_flash(id);

  flash_addr = address;
  flash_ptr = buffer;


  for (i=0; i<len; i++) {
    //auto erase
    if ( (flash_addr & 0xfff) == 0) {
      flash_erase_4k( flash_addr, id );
      //printf("\r\nerasing 4k %x", flash_addr);
    }

    flash_write_byte( flash_addr++, *flash_ptr++, id);
  }


}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_write_byte(uint32_t address, uint8_t val, int id)
{


  wait_for_flash(id);

  //select flash
  flash_select(id);

  xmit_spi2( FLASH_WREN );   //enable write / erase

  //deselect flash
  flash_deselect(id);

  //select flash
  flash_select(id);

  xmit_spi2( FLASH_BYTE_PGM );

  xmit_spi2( (uint8_t) (address >> 16)&0xff );
  xmit_spi2( (uint8_t) (address >> 8)&0xff );
  xmit_spi2( (uint8_t) (address & 0xff) );

  xmit_spi2( val );

  //deselect flash
  flash_deselect(id);



}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_erase_4k(uint32_t erase_addr, int id)
{


  wait_for_flash(id);

  flash_unprotect_all(id);

  //select flash
  flash_select(id);

  xmit_spi2( FLASH_WREN );   //enable write / erase

  //deselect flash
  flash_deselect(id);



  //select flash
  flash_select(id);

  xmit_spi2( FLASH_ERASE_4k ); //do erase op
  xmit_spi2( (uint8_t) (erase_addr >> 16)&0xff );
  xmit_spi2( (uint8_t) (erase_addr >> 8)&0xff );
  xmit_spi2( (uint8_t) (erase_addr & 0xff) );

  //deselect flash
  flash_deselect(id);

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_erase_32k(uint32_t erase_addr, int id)
{


  wait_for_flash(id);

  flash_unprotect_all(id);

  //select flash
  flash_select(id);

  xmit_spi2( FLASH_WREN );   //enable write / erase

  //deselect flash
  flash_deselect(id);



  //select flash
  flash_select(id);

  xmit_spi2( FLASH_ERASE_32k );  //do erase op
  xmit_spi2( (uint8_t) (erase_addr >> 16)&0xff );
  xmit_spi2( (uint8_t) (erase_addr >> 8)&0xff );
  xmit_spi2( (uint8_t) (erase_addr & 0xff) );

  //deselect flash
  flash_deselect(id);


}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_erase_64k(uint32_t erase_addr, int id)
{


  wait_for_flash(id);

  flash_unprotect_all(id);

  //select flash
  flash_select(id);

  xmit_spi2( FLASH_WREN );   //enable write / erase

  //deselect flash
  flash_deselect(id);



  //select flash
  flash_select(id);

  xmit_spi2( FLASH_ERASE_64k );  //do erase op
  xmit_spi2( (uint8_t) (erase_addr >> 16)&0xff );
  xmit_spi2( (uint8_t) (erase_addr >> 8)&0xff );
  xmit_spi2( (uint8_t) (erase_addr & 0xff) );

  //deselect flash
  flash_deselect(id);

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_erase_all(int id)
{

  flash_unprotect_all(id);


  //select flash
  flash_select(id);

  xmit_spi2( FLASH_WREN );   //enable write / erase

  //deselect flash
  flash_deselect(id);



  //select flash
  flash_select(id);

  xmit_spi2( FLASH_CHIP_ERASE ); //do erase op

  //deselect flash
  flash_deselect(id);

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint8_t flash_isbusy(int id)
{

  uint8_t status;



  //select flash
  flash_select(id);

  xmit_spi2( FLASH_RDSR );
  status = xmit_spi2(0xff);

  //printf("\r\nflash status: %02x", status);

  //deselect flash
  flash_deselect(id);



  if (status & FLASH_BUSY) return 1;
  else return 0;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint8_t flash_isprotected(int id)
{

  uint8_t status;

  uint32_t prim;
  prim = __get_PRIMASK();
  __disable_irq();


  //select flash
  flash_select(id);

  xmit_spi2( FLASH_RDSR );
  status = xmit_spi2(0xff);

  //printf("\r\nflash status: %02x", status);

  //deselect flash
  flash_deselect(id);



  if(!prim) {
    __enable_irq();
  }

  if (status & (FLASH_BP0 | FLASH_BP1 | FLASH_BP2 | FLASH_BP3) ) return 1;
  else return 0;
}
