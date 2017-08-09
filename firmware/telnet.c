
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
#include <string.h>
#include "telnet.h"

#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "std_io.h"
#include "global.h"
#include "config.h"
#include "main.h"
#include "Playtune.h"
#include "flash_sst.h"

static struct tcp_pcb *telnet_pcb;
static uint8_t cmd_buffer[1500];
static uint8_t time_str[64];
static do_close;
static int i;
static int totlen;
static char *ptr;
static struct pbuf *q;
static int len;
static int cmd_idx;
int8_t do_testflash1;
int8_t do_testflash2;
int8_t do_testadc;

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
static err_t telnet_poll(void *arg, struct tcp_pcb *pcb)
{


  if(pcb!=NULL && net_buffer[0]!=0 ) {
    int len = strlen(net_buffer);

    int send_len = len;
    if(send_len > 1460) send_len=1460;

    if(send_len>0) {
      telnet_write(pcb, net_buffer,send_len);
      memcpy(&net_buffer[0], &net_buffer[send_len], MAX_NET_BUFFER-send_len-1 );
    }
  }
  if(pcb!=NULL && do_close) {
    tcp_close(pcb);
    do_close=0;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void telnet_write(struct tcp_pcb *tn_write_pcb, uint8_t *buffer, int len)
{

  if(tn_write_pcb==NULL) return;

  if(len>1400) {
    tcp_write(tn_write_pcb, buffer, 1400, 1);
    len-=1400;
    if(len>0) tcp_write(tn_write_pcb, &buffer[1400], len, 1);
  } else {
    tcp_write(tn_write_pcb, buffer, len, 1);
  }

  tcp_output(tn_write_pcb);
  tcp_output(tn_write_pcb);

}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
static err_t telnet_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  totlen = p->tot_len;
  q=p;
  len = q->len;
  ptr = q->payload;
  cmd_idx=0;

  if (err == ERR_OK && p != NULL) {

    tcp_recved(pcb, p->tot_len);

    //cat until \r or \n
    for(i=0; i<sizeof(cmd_buffer)-totlen; i++) {
      if( cmd_buffer[i]==0x00 ) {
        cmd_idx = i;
        break;
      }
    }

    while(q) {
      for(i=0; i<len; i++) {
        cmd_buffer[cmd_idx++] = *ptr++;
      }
      q = q->next;
      if(q!=NULL) {
        len=q->len;
        ptr = q->payload;
      }
    }

    //telnet negotiation
    if(cmd_buffer[0]==0xff) {

      cmd_buffer[0] = 0xff;  //IAC
      cmd_buffer[1] = 0xfc;  //won't
      cmd_buffer[2] = 0x03;  //echo

      cmd_buffer[3] = 0xff;  //IAC
      cmd_buffer[4] = 0xfe;  //don't
      cmd_buffer[5] = 0x1f;  //negotiate

      cmd_buffer[6] = 0xff;  //IAC
      cmd_buffer[7] = 0xfe;  //don't
      cmd_buffer[8] = 0x22;  //linemode

      telnet_write(pcb, cmd_buffer, 9);
      memset(cmd_buffer,0x00,sizeof(cmd_buffer));
    } else {
      if( memcmp(cmd_buffer, "quit", 4) == 0 || memcmp(cmd_buffer,"exit",4)==0 ) {  //close telnet session
        close_telnet();
        memset(cmd_buffer,0x00,sizeof(cmd_buffer));
      } else {
        for(i=0; i<cmd_idx; i++) {
          if(cmd_buffer[i]=='\r' || cmd_buffer[i]=='\n') {
            memset(net_buffer,0x00,MAX_NET_BUFFER);

            /////////////////////////////////////////////////////////
            // available commands via telnet
            /////////////////////////////////////////////////////////
            if( strncmp(cmd_buffer,"testmem",7)==0) { //test the 16-bit FSMC memory-mapped interface to the FPGA
              do_test_mem=1;
              mem_test_cont=1;
            } else if( strncmp(cmd_buffer,"testsnd",7)==0) {    //test the verilog audio synthesizer / I2S 
              uint16_t *audio_ctrl_ptr = (uint16_t *) 0x60001000; //plays an 8-channel song converted from midi source
              uint16_t *waveform_ptr = (uint16_t *) 0x60001016;

              uint16_t val = *audio_ctrl_ptr;
              val |= 2;
              *audio_ctrl_ptr = val;

              *waveform_ptr=0x5555;
              do_play_midi=1;
              tune_playscore((const int8_t *) score1);
            } else if( strncmp(cmd_buffer,"reset",5)==0) {  //close telnet session and reset the device
              do_reset_flag=1;
            } else if( strncmp(cmd_buffer,"testflash1",10)==0) {  //test 8-pin SOIC flash #1
              do_testflash1=1;
            } else if( strncmp(cmd_buffer,"testflash2",10)==0) {  //test 8-pin SOIC flash #2
              do_testflash2=1;
            } else if( strncmp(cmd_buffer,"testadc",7)==0) {  //test ltc1420 expansion module 
              do_testadc=1;
            } else if( strncmp(cmd_buffer,"help",4)==0 || strncmp(cmd_buffer,"?",1)==0 ) {  //close telnet session and reset the device
              print_help();
            } else {
              print_prompt(); //unknown command
            }
            /////////////////////////////////////////////////////////



            telnet_poll((void *) NULL, pcb);
            memset(cmd_buffer,0x00,sizeof(cmd_buffer));
          }
          if(cmd_buffer[i]==0x00) break;
        }
      }
    }

    pbuf_free(p);
  }


  if(err == ERR_OK && p == NULL) {
    tcp_close(pcb);

    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);

  }

  return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
void close_telnet(void)
{
  do_close=1;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
static err_t telnet_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);

  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, telnet_recv);
  tcp_poll(pcb, telnet_poll, 1);

  cmd_buffer[0]=0;
  cmd_buffer[1]=0;
  cmd_buffer[2]=0;

  memset(net_buffer,0x00,MAX_NET_BUFFER);

  uint32_t clk_mhz = SystemCoreClock/1e6;
  printf("\r\nConnected To STM32F417 Running @ %u MHz", clk_mhz);
  print_prompt();

  //telnet_poll((void *) NULL, pcb);
  memset(cmd_buffer,0x00,sizeof(cmd_buffer));

  return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
void print_prompt()
{
  printf( "\r\nDSP_Ice %d.%d.%d.%d:~$ ",config.ip_addr[0],config.ip_addr[1],config.ip_addr[2],config.ip_addr[3] );
}
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
void print_help()
{
  printf("\r\nAvailable commands:");
  printf("\r\n\r\ntestmem");
  printf("\r\ntestsnd");
  printf("\r\ntestflash1");
  printf("\r\ntestflash2");
  printf("\r\ntestadc");
  printf("\r\nreset");
  printf("\r\nexit\r\n\r\n ");
  print_prompt();
}
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
void telnet_init(void)
{

  telnet_pcb = tcp_new();
  tcp_bind(telnet_pcb, IP_ADDR_ANY, 23);
  telnet_pcb = tcp_listen(telnet_pcb);
  tcp_accept(telnet_pcb, telnet_accept);
}
