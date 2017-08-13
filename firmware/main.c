
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

#define ARM_MATH_CM4

#include "arm_math.h"
#include "arm_const_structs.h"
#include "bh_fft_win.c"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"

#include "stm32f4xx_flash.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_iwdg.h"
#include "stm32f4x7_eth.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"

#include "netconf.h"
#include "global.h"
#include "std_io.h"
#include "telnet.h"
#include "crc.h"
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include "config.h"
#include "iperf.h"
#include "flash_sst.h"
#include "fpga.h"
#include "Playtune.h"


//FFT stuff
#define SAMPLES                    2048            //real 
#define FFT_SIZE                SAMPLES / 2        // FFT size 1024 
#define DIRECTFFT    0
#define INVERSEFFT   1
#define NOREVERSE    1
#define REVERSE    1

float32_t Input[SAMPLES];
float32_t Output[FFT_SIZE];
arm_cfft_radix2_instance_f32 fftS;    //ARM CFFT module 

RCC_ClocksTypeDef RCC_Clocks;

static int n;
volatile uint32_t prev_ms_timer;
volatile int midi_off_time;

volatile uint32_t local_time_ms;
volatile uint32_t ms_timer;
volatile int8_t all_initialized;

int do_test_mem;
int mem_test_cont;

int8_t do_play_midi;

volatile uint32_t tick_count;
volatile uint32_t LocalTime;

extern int ethernet_link_initialized;
extern int ethernet_post_link_done;

volatile int uptime_seconds;
volatile int prev_uptime_seconds;
volatile int fw_reset_time;
volatile int uptime_mod;
volatile int do_reset_flag=0;

/////////////////////////////////////////////////////////////////////
//variables touched in SysTick Interrupt Handler
/////////////////////////////////////////////////////////////////////
volatile int8_t do_eth_init_tick;
volatile int do_print_prompt;
volatile int lwip_initialized;
volatile int check_firmware_update;
static volatile int8_t do_tcp_poll;

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
int should_do_tcp_poll()
{
  if(do_tcp_poll) {
    do_tcp_poll=0;
    return 1;
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void do_system_reset()
{
  int secs=0;

  while(1) {
    handle_ethernet(1);

    do_main_tasks(0);

    if(prev_uptime_seconds!=uptime_seconds) {
      prev_uptime_seconds=uptime_seconds;
      secs++;
    }

    if(secs>1) break;

  } //handle lwip, but don't reset watchdog

  NVIC_SystemReset();
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void save_config()
{
  uint32_t prim = __get_PRIMASK();
  __disable_irq();

  write_config(&config);

  if(!prim) {
    __enable_irq();
  }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int main(void)
{
  int i;
  uint8_t *ptr;
  GPIO_InitTypeDef  GPIO_InitStructure;

  //SystemInit();

  srand(1);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  RCC_GetClocksFreq(&RCC_Clocks);


  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD
                         | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);

  SysTick_Config(RCC_Clocks.HCLK_Frequency / 50);


  //enable FPU
  *((volatile unsigned long*)0xE000ED88) = 0xF << 20;


  read_config();
  fpga_init();
  ETH_BSP_Config();

  //do {
  init_flash_sst();

  config_changed(); //leave this after read_pdata, so mur_mode gets updated properly

  uart3_init(115200);

  if(!lwip_initialized) {
    lwip_initialized=1;
    lwip_init();
    telnet_init();
    tcp_iperf_init();
  }



  init_stm_timers();


  all_initialized=1;


  //flush sdi buffers


  set_timer2(0);

  init_fsmc();

  while(1) {

    do_main_tasks(1);


    if(do_reset_flag) {
      do_reset_flag=0;
      close_telnet();
      do_system_reset();
    }

    if(do_test_mem) {
      if(!mem_test_cont) do_test_mem=0;
      test_mem();
    }

    if(do_testflash1) {
      do_testflash1=0;
      test_flash(0);
    }

    if(do_testflash2) {
      do_testflash2=0;
      test_flash(1);
    }

    if(do_testadc) {
      do_testadc=0;
      test_adc();
      print_prompt();
    }

    if(do_eth_init_tick) {
      do_eth_init_tick=0;
      eth_init_tick();
    }


  } //while


}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void fft_shift(float32_t *data, int len) {
float32_t tmp[2048];
int i;
    if(len>2048) len=2048;

    for(i=0;i<len/2;i++) {
        tmp[i] = data[i+len/2]; 
    }
    for(i=0;i<len/2;i++) {
        tmp[i+len/2] = data[i]; 
    }
    for(i=0;i<len;i++) {
      data[i] = tmp[i];
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// fft results indicate that Fs is approximately 1.2 MHz in this async direct sample loop
// With system clock of 168 MHz and currently configured FSMC clk (including all delays)
// We could reach the full Fs of 10 MHz by buffering the ADC in BRAM of FPGA and analyzing after DMA transfer
// to the MCU.  The BRAM is currenlty in use by the audio-synth delay effect example
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void test_adc() {
  static int i;
  static int16_t *adc_ptr = 0x60000010;


  //BTR1
  (*((uint32_t *)(0xa0000004))) = BTR_BRAM_RW_FAST; 


  int index=0;
  for(i=0;i<2048;i++) {
    Input[index++] = (float32_t) *adc_ptr;  //real
    Input[index++] = (float32_t) 0;  //complex 
    DelayClk3(7);
  }

  uint32_t stimer = get_timer2();

  //mult by fft blackman harris window
  if(SAMPLES==1024) {
    arm_mult_f32( Input, fft_win_1024, Input, FFT_SIZE*2);
  }
  else if(SAMPLES==512) {
    arm_mult_f32( Input, fft_win_512, Input, FFT_SIZE*2);
  }
  else if(SAMPLES==2048) {
    arm_mult_f32( Input, fft_win_2048, Input, FFT_SIZE*2);
  }

  //initialize FFT  intflag=0  bitreverse=1
  arm_cfft_radix2_init_f32(&fftS, FFT_SIZE, 0, 1);
  //do FFT_SIZE point complex fft 32-bit 
  arm_cfft_radix2_f32(&fftS,Input);

  //calculate magnitude of fft for each bin 
  arm_cmplx_mag_f32(Input, Output, FFT_SIZE);

  //not needed for this application
  //fft_shift(Output, FFT_SIZE/2);

  int32_t total_time_ms = get_timer2() - stimer;
  float32_t tot_time_ms_f = (float32_t) total_time_ms / 42000.0f;

  //NOTE: if the sample rate, delays, etc are changed.  These need to be adjusted
  #define VPP_SCALE 7.530865e-7f 
  #define FREQ_SCALE 1279.07324f 

  printf("\r\n");
  for(i=0;i<512;i++) {  //we throw out the last half of results since input imaginary was zero and results in image in second half.
    printf("%4.5f,", Output[i]*VPP_SCALE);  //scale to voltage peak-to-peak for periodic sine wave input
  }

  float32_t max = -1e6f;
  int max_bin=0;
  //don't look in first few bins to ignore any dc offset
  for(i=5;i<512;i++) {  
    if(Output[i]>max) {
      max = Output[i];
      max_bin=i;
    }
  }

  
  //results should be fairly accurate for 10khz to 650khz, < 2 millivolts p-p,  up to 4 volts p-p with current configuration
  if(max_bin>0) {
    float32_t freq = FREQ_SCALE * (float32_t) max_bin;
    printf("\r\npeak signal %4.3f Hz,  %4.3f Volts p-p, elapsed FFT Mag time %3.2f ms", freq, Output[max_bin]*VPP_SCALE, tot_time_ms_f); 
  }
  

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void check_ms_timer()
{
  uint32_t timer2 = get_timer2();
  uint32_t prim;

  //has millisecond passed yet?
  if(timer2>42000) {
    clear_timer2();
    local_time_ms++;
    LwIP_Periodic_Handle(local_time_ms);

    if(do_play_midi) {
      tune_stepscore();
      if(!tune_playing) do_play_midi=0;
      midi_off_time=0;
    } else {
      //turn delay off
      if(midi_off_time==2) {
        uint16_t *audio_ctrl_ptr = (uint16_t *) 0x60001000;
        *audio_ctrl_ptr&=0xfffd; //turn on waveform generation
      }
    }

    if(prev_uptime_seconds!=uptime_seconds) {
      prev_uptime_seconds=uptime_seconds;
      //printf("\r\nuptime_seconds: %d", uptime_seconds);

      //if(ne4_count!=0) printf("\r\nne4 count: %d", ne4_count);
      //ne4_count=0;
      midi_off_time++;

      puts_u3("\r\ntest uart3", 12);
    }

  }

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void do_main_tasks(int ltick)
{

  handle_ethernet(ltick);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void handle_ethernet(int ltick)
{
  extern void *pdev_device;
  int offset;
  int count;
  int16_t len;
  int i;
  FrameTypeDef frame;

  while( ETH_CheckFrameReceived() ) {
    frame = ETH_Get_Received_Frame();

    // Release descriptors to DMA
    // Check if frame with multiple DMA buffer segments
    if (DMA_RX_FRAME_infos->Seg_Count > 1) {
      DMARxNextDesc = DMA_RX_FRAME_infos->FS_Rx_Desc;
    } else {
      DMARxNextDesc = frame.descriptor;
    }

    // Set Own bit in Rx descriptors: gives the buffers back to DMA
    for (i=0; i<DMA_RX_FRAME_infos->Seg_Count; i++) {
      DMARxNextDesc->Status = ETH_DMARxDesc_OWN;
      DMARxNextDesc = (ETH_DMADESCTypeDef *)(DMARxNextDesc->Buffer2NextDescAddr);
    }

    // Clear Segment_Count
    DMA_RX_FRAME_infos->Seg_Count =0;

    // When Rx Buffer unavailable flag is set: clear it and resume reception
    if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET) {
      // Clear RBUS ETHERNET DMA flag
      ETH->DMASR = ETH_DMASR_RBUS;
      // Resume DMA reception
      ETH->DMARPDR = 0;
    }

    if( frame.length > 0 && frame.length<=1514) {
      //if raw filter doesn't do anything, then pass on to lwip
      memcpy(Rx_Buff, (uint8_t *) frame.buffer, frame.length);

      lwip_packet_in_eth((uint8_t *) Rx_Buff, frame.length);
    }
  }
  if(ltick) check_ms_timer();

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void handle_systick(void)
{

  if(++uptime_mod%50==0) uptime_seconds++;

  if(!all_initialized) return;

  if( LocalTime%200==0) {
    do_tcp_poll=1;
  }

  do_eth_init_tick=1;

  if(fw_reset_time>0) fw_reset_time--;

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void assert_param(int pass)
{
}
