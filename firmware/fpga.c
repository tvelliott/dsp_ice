
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


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include  "fpga_image.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_fsmc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "misc.h"
#include "fpga.h"
#include "std_io.h"
#include "main.h"


extern volatile int uptime_seconds;

volatile int ne4_count;
volatile int8_t do_sample_mode;
volatile int8_t fsmc_init;

static int32_t prevChar=-10000;
static int32_t currChar;

static int8_t *in_ptr;
static uint8_t byte_in;
static int out_bytes;
static int ii;
static uint8_t count;
int fpga_bytes_out;

static volatile int16_t sample_left;
static volatile int16_t sample_right;
static volatile int16_t *sample_ptr;

static uint16_t val1;
static uint16_t val2;
static uint64_t rw_count=0;
static uint16_t *ptr;
static uint32_t deltime;
static int total_errors;
static volatile uint16_t *left = (uint16_t *) 0x60001002; //left sample register
static volatile uint16_t *right = (uint16_t *) 0x60001004; //left sample register
//these must be declared volatile
static volatile uint16_t *spi_ctrl;
static volatile uint16_t *spi_datain;
static volatile uint16_t *spi_dataout;
static volatile uint16_t *cs_ctrl;
/////////////////////////////////////////////////////////////////////////////////////////
// put the fpga into spi slave / programming mode
/////////////////////////////////////////////////////////////////////////////////////////
void fpga_init()
{
  GPIO_InitTypeDef  GPIO_InitStructure;


  //fpga
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOG , ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  GPIO_ResetBits(GPIOG, GPIO_Pin_8);  //fpga creset_b





  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC , ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_10 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  GPIO_ResetBits(GPIOC, GPIO_Pin_7);  //fpga spi_cs
  GPIO_SetBits(GPIOC, GPIO_Pin_10);  //fpga spi_sck

  delay_ms(500);

  GPIO_SetBits(GPIOG, GPIO_Pin_8);  //fpga creset_b

  delay_ms(500);

  //fpga config size for 8k lattice device is constant 135100
  fpga_rle_decode(rle_compressed_fpga_image, 135100 );

  GPIO_SetBits(GPIOC, GPIO_Pin_7);  //fpga spi_cs

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void fpga_init_interrupt(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  GPIO_InitTypeDef  GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource12);

  EXTI_InitStructure.EXTI_Line = EXTI_Line12;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static uint8_t write_fpga(uint8_t out_reg)
{
  uint8_t in_reg;
  uint8_t out_byte = out_reg;

  for(ii=0; ii<8; ii++) {
    in_reg <<=1;

    //setup mosi
    if(out_byte & 0x80) GPIO_SetBits(GPIOC, GPIO_Pin_12 );
    else GPIO_ResetBits(GPIOC, GPIO_Pin_12 );

    out_byte <<=1;

    //sclk low
    GPIO_ResetBits(GPIOC, GPIO_Pin_10 );

    if( GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_11) ) in_reg |= 0x01;

    //sclk high.  data is clocked in on rising edge of sclk
    GPIO_SetBits(GPIOC, GPIO_Pin_10 );

  }

  fpga_bytes_out++;
  return in_reg;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// uncompress the fpga image and send to fpga
//////////////////////////////////////////////////////////////////////////////////////////////
uint8_t fpga_rle_decode(const uint8_t *image, int len)
{
  in_ptr =image;
  out_bytes=0;
  fpga_bytes_out=0;

  prevChar = -10000;

  while (out_bytes<len+1) {

    byte_in = *in_ptr++;
    currChar = byte_in;

    //write byte_in to fpga
    write_fpga( byte_in );

    out_bytes++;

    if (currChar == prevChar) {

      byte_in = *in_ptr++;
      count = byte_in;

      while (count > 0) {
        //write byte_in to fpga = (uint8_t) (currChar&0xff);
        write_fpga( (uint8_t) (currChar&0xff) );

        out_bytes++;
        count--;
      }

      prevChar = -10000;
    } else {
      prevChar = currChar;
    }
  }


  //Lattice datasheet says keep clocking for 64 more cycles
  write_fpga( 0xff );
  write_fpga( 0xff );
  write_fpga( 0xff );
  write_fpga( 0xff );
  write_fpga( 0xff );
  write_fpga( 0xff );
  write_fpga( 0xff );
  write_fpga( 0xff );

  return out_bytes-1;
}
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
void init_fsmc()
{

  /* Connect PDx pins to FSMC Alternate function */
  //all but PD2
  GPIOD->AFR[0]  = 0xccccc0cc;
  GPIOD->AFR[1]  = 0xcccccccc;
  /* Configure PDx pins in Alternate function mode */
  //all but PD2
  GPIOD->MODER   = 0xaaaaaa8a;


  /* Configure PDx pins speed to 100 MHz */
  //all but PD2
  GPIOD->OSPEEDR = 0xffffffcf;
  /* Configure PDx pins Output type to push-pull */
  GPIOD->OTYPER  = 0x00000000;
  /* No pull-up, pull-down for PDx pins */
  GPIOD->PUPDR   = 0x00000000;

  /* Connect PEx pins to FSMC Alternate function */
  GPIOE->AFR[0]  = 0xc00cc0cc;
  GPIOE->AFR[1]  = 0xcccccccc;
  /* Configure PEx pins in Alternate function mode */
  GPIOE->MODER   = 0xaaaa828a;
  /* Configure PEx pins speed to 100 MHz */
  GPIOE->OSPEEDR = 0xffffc3cf;
  /* Configure PEx pins Output type to push-pull */
  GPIOE->OTYPER  = 0x00000000;
  /* No pull-up, pull-down for PEx pins */
  GPIOE->PUPDR   = 0x00000000;

  /* Connect PFx pins to FSMC Alternate function */
  GPIOF->AFR[0]  = 0x00cccccc;
  GPIOF->AFR[1]  = 0xcccc0000;
  /* Configure PFx pins in Alternate function mode */
  GPIOF->MODER   = 0xaa000aaa;
  /* Configure PFx pins speed to 100 MHz */
  GPIOF->OSPEEDR = 0xff000fff;
  /* Configure PFx pins Output type to push-pull */
  GPIOF->OTYPER  = 0x00000000;
  /* No pull-up, pull-down for PFx pins */
  GPIOF->PUPDR   = 0x00000000;

  /* Connect PGx pins to FSMC Alternate function */
  GPIOG->AFR[0]  = 0x00cccccc;
  GPIOG->AFR[1]  = 0x000000c0;
  /* Configure PGx pins in Alternate function mode */
  GPIOG->MODER   = 0x00080aaa;
  /* Configure PGx pins speed to 100 MHz */
  GPIOG->OSPEEDR = 0x000c0fff;
  /* Configure PGx pins Output type to push-pull */
  GPIOG->OTYPER  = 0x00000000;
  /* No pull-up, pull-down for PGx pins */
  GPIOG->PUPDR   = 0x00000000;

  /* Enable the FSMC interface clock */
  RCC->AHB3ENR         = 0x00000001;


  //BCR1
  //(*((uint32_t *)(0xa0000000))) = 0x000030db; //config for async memory interface with NOE, NWE, NADV signals
  (*((uint32_t *)(0xa0000000))) = 0x000813d7;  //bit 8 = sync, PSRAM


  //BTR1
  //(*((uint32_t *)(0xa0000004))) = 0x00001108; //clkdiv=0, good async settings for fpga clk_t of 80.0mhz  / mcu fsmc clk of 84mhz
  (*((uint32_t *)(0xa0000004))) = BTR_BRAM_RW_FAST; //clkdiv=1,

  //BWTR1 not used in sync mode
  //(*((uint32_t *)(0xa0000104))) = 0x0f1fffff;

  FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);

  //enable NADV
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  //output type push-pull
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  //pullup

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //alternate function
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_PinAFConfig( GPIOB, GPIO_PinSource7, GPIO_AF_FSMC);

  //CBSEL is output
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  //chip sel high
  uint16_t *cs_ctrl =    0x60000006;
  *cs_ctrl = 0;  //cs1=1

  //reset signal to HDL code
  GPIO_ResetBits(GPIOG, GPIO_Pin_6);  //fpga CBSEL is the rst (active low) input signal to FPGA
  delay_ms(10);
  GPIO_SetBits(GPIOG, GPIO_Pin_6);  //ready to go

  //audio sample-mod interrupt
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

  //memory address for the fpga  NE1 select signal
  ptr = 0x60000000;

  for(ii=0; ii<8; ii++) {
    printf("\r\nmem addr: %08x  : %04x", ptr, *ptr);
    ptr++;
    DelayClk3(50);  //wait between r/w
  }

  DelayClk3(50);  //wait between r/w
  ptr = 0x60001002; //random number generator
  printf("\r\nmem addr: %08x  : %04x", ptr, *ptr);

  DelayClk3(50);  //wait between r/w
  ptr = 0x60001004; //random number generator
  printf("\r\nmem addr: %08x  : %04x", ptr, *ptr);

  DelayClk3(50);  //wait between r/w
  ptr = 0x6000f000; //random number generator
  printf("\r\nmem addr: %08x  : %04x (RO)", ptr, *ptr);


  ptr = 0x6000d000;
  for(ii=0; ii<24; ii++) {
    printf("\r\nmem addr: %08x  : %04x", ptr, *ptr);
    ptr++;
    DelayClk3(50);  //wait between r/w
  }

  ptr = 0x6000e000;
  for(ii=0; ii<24; ii++) {
    printf("\r\nmem addr: %08x  : %04x", ptr, *ptr);
    ptr++;
    DelayClk3(50);  //wait between r/w
  }

  //fpga_init_interrupt();  //not currently using this

  fsmc_init=1;
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void test_mem()
{

  uptime_seconds = 0;

  (*((uint32_t *)(0xa0000004))) = BTR_REG_RW_FAST; //clkdiv=1,

  static int index=0;
  while(uptime_seconds<10) {

    val1 = ((uint16_t) rand()) & 0xffff;
    if(val1&0x01) {
      ptr = 0x60001002;
    } else {
      ptr = 0x60001002;
    }

    int dly = (int) (val1&0x3f)+1;

    DelayClk3( dly );  //wait between r/w

    val1 = ((uint16_t) rand()) & 0xffff;
    *ptr = val1;

    DelayClk3( dly );  //wait between r/w

    val2 = *ptr;


    if(val1!=val2) {
      printf("\r\nerror at address %08x, w:%04x, r:%04x", ptr,val1,val2);
      //delay_ms(100);
      total_errors++;
    } else if(index++%2048==0) {
      rw_count+=2048*4;
      printf("\r\n0x100x (FPGA D-REG), %llu bytes r/w random access, current vals: %04x,%04x, errors: %d", rw_count, val1,val2, total_errors);
    } else {
      do_main_tasks(1);
    }
  }

//can only write to BRAM if audio delay on fpga is not used for now
  /*
    (*((uint32_t *)(0xa0000004))) = BTR_BRAM_RW_FAST; //clkdiv=1,

    uptime_seconds = 0;

    index=0;
    while(uptime_seconds<10) {

      val1 = ((uint16_t) rand()) & 0xffff;
      ptr = (0x6000d000 + (val1&0x3f)*2 );

      int dly = (int) (val1&0x3f)+1;
      DelayClk3(dly);  //wait between r/w
      val1 = ((uint16_t) rand()) & 0xffff;
      *ptr = val1;

      DelayClk3(dly);  //wait between r/w
      val2 = *ptr;


      if(val1!=val2) {
        printf("\r\nerror at address %08x, w:%04x, r:%04x", ptr,val1,val2);
        //delay_ms(100);
        total_errors++;
      } else if(index++%2048==0) {
        rw_count+=2048*4;
        printf("\r\n0xd0xx (FPGA BRAM), %llu bytes r/w random access, current vals: %04x,%04x, errors: %d", rw_count, val1,val2, total_errors);
      }
      else {
        do_main_tasks(1);
      }
    }


    uptime_seconds = 0;

    index=0;
    while(uptime_seconds<10) {

      val1 = ((uint16_t) rand()) & 0xffff;
      ptr = (0x6000e000 + (val1&0x3f)*2 );

      int dly = (int) (val1&0x3f)+1;
      DelayClk3(dly);  //wait between r/w
      val1 = ((uint16_t) rand()) & 0xffff;
      *ptr = val1;

      DelayClk3(dly);  //wait between r/w
      val2 = *ptr;


      if(val1!=val2) {
        printf("\r\nerror at address %08x, w:%04x, r:%04x", ptr,val1,val2);
        //delay_ms(100);
        total_errors++;
      } else if(index++%2048==0) {
        rw_count+=2048*4;
        printf("\r\n0xe0xx (FGPA BRAM), %llu bytes r/w random access, current vals: %04x,%04x, errors: %d", rw_count, val1,val2, total_errors);
      }
      else {
        do_main_tasks(1);
      }
    }
  */
  print_prompt();
}
//////////////////////////////////////////////////////////////////////////////
// interrupt is generated by FPGA at I2S sample frequency
// when sample mode is enabled (bit 0) of waveform ctrl register 0x60001000
//////////////////////////////////////////////////////////////////////////////
void EXTI15_10_IRQHandler()
{
  EXTI_ClearITPendingBit(EXTI_Line12);

  if(!do_sample_mode) return;

}


///////////////////////////////////////////////////////////////////////////////////////////////
// test the spi_master.v verilog code over the expansion ports via MCU/FSMC control
///////////////////////////////////////////////////////////////////////////////////////////////
uint8_t fpga_spi_write_reg(uint8_t val, uint8_t port, int8_t term_cs)
{

  if(port==0) {
    spi_ctrl =    0x60000000;
    spi_datain =  0x60000002; //data to send to slave
    spi_dataout = 0x60000004; //data received from slave
    cs_ctrl = 0x60000006; //cs
  }
  else if(port==1) {
    spi_ctrl =    0x60000008;
    spi_datain =  0x6000000a; //data to send to slave
    spi_dataout = 0x6000000c; //data received from slave
    cs_ctrl = 0x6000000e; //cs
  }
  else {
    spi_ctrl =    0x60000000;
    spi_datain =  0x60000002; //data to send to slave
    spi_dataout = 0x60000004; //data received from slave
    cs_ctrl = 0x60000006; //cs
  }

  *cs_ctrl = 1;  //cs1=0

  *spi_datain = val; 

  *spi_ctrl = 1;  //start transfer/
  *spi_ctrl = 0;  

  //wait for new_data_ready 
  while(1) {
   uint8_t ctrl = *spi_ctrl;
   if( (ctrl & 0x04) ) break;
  }

  uint8_t status = (*spi_dataout&0xff);


  if(term_cs) *cs_ctrl = 0;  //cs=1

  return status;
}

///////////////////////////////////////////////////////////////////////////////////////////////
// test the spi_master.v verilog code over the expansion ports via MCU/FSMC control
///////////////////////////////////////////////////////////////////////////////////////////////
uint8_t fpga_spi_read_reg(uint8_t val, uint8_t port, int8_t term_cs)
{

  if(port==0) {
    spi_ctrl =    0x60000000;
    spi_datain =  0x60000002; //data to send to slave
    spi_dataout = 0x60000004; //data received from slave
    cs_ctrl = 0x60000006; //cs
  }
  else if(port==1) {
    spi_ctrl =    0x60000008;
    spi_datain =  0x6000000a; //data to send to slave
    spi_dataout = 0x6000000c; //data received from slave
    cs_ctrl = 0x6000000e; //cs
  }
  else {
    spi_ctrl =    0x60000000;
    spi_datain =  0x60000002; //data to send to slave
    spi_dataout = 0x60000004; //data received from slave
    cs_ctrl = 0x60000006; //cs
  }

  *cs_ctrl = 1;  //cs1=0

  //clock out val 
  *spi_datain = val; 

  *spi_ctrl = 1;  //start transfer/
  *spi_ctrl = 0;  

  //wait for new_data_ready 
  while(1) {
   uint8_t ctrl = *spi_ctrl;
   if( (ctrl & 0x04) ) break;
  }

  uint8_t reg_val = (*spi_dataout&0xff);

  if(term_cs) *cs_ctrl = 0;  //cs=1

  return reg_val;

}
