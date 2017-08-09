
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


#include <string.h>
#include <stdint.h>
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "iperf.h"
#include "std_io.h"
#include "main.h"

/* iperf3 states */
#define TEST_START 1
#define TEST_RUNNING 2
#define RESULT_REQUEST 3
#define TEST_END 4
#define STREAM_BEGIN 5
#define STREAM_RUNNING 6
#define STREAM_END 7
#define ALL_STREAMS_END 8
#define PARAM_EXCHANGE 9
#define CREATE_STREAMS 10
#define SERVER_TERMINATE 11
#define CLIENT_TERMINATE 12
#define EXCHANGE_RESULTS 13
#define DISPLAY_RESULTS 14
#define IPERF_START 15
#define IPERF_DONE 16
#define ACCESS_DENIED (-1)
#define SERVER_ERROR (-2)


#define SEG_SIZE (TCP_MSS)

static struct tcp_pcb *iperf_control_pcb = NULL;
static struct tcp_pcb *iperf_data_pcb = NULL;

static int i;

static int local_state;
static int32_t parm_len;
static long total;

static int start_time;
static int do_reverse;
static int seg_out_count;

static struct pbuf *q;
static int len;
static char *ptr;
static uint8_t c;
static uint8_t buffer[SEG_SIZE];
static int tot_len;
static uint8_t *ptr_in;
static uint16_t send_len;
static int8_t err;
static int timeout;


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
static void tcp_iperf_close(struct tcp_pcb *iperf_pcb)
{
  tcp_sent(iperf_pcb, NULL);
  tcp_recv(iperf_pcb, NULL);
  tcp_poll(iperf_pcb, NULL, 0);

  tcp_close(iperf_pcb);

  start_time=0;
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
static err_t tcp_iperf_poll(void *arg, struct tcp_pcb *iperf_pcb)
{
  uint8_t c;
  if(start_time!=0 && (uptime_seconds-start_time)>1 && local_state==4) {
    local_state=5;
    start_time=uptime_seconds;
    c=TEST_START;
    tcp_write(iperf_control_pcb, &c, 1, TCP_WRITE_FLAG_COPY);
    tcp_output(iperf_control_pcb);
  }

  /*
  if(local_state!=0 && timeout++>3000) {
    local_state=0;
    tcp_iperf_close(iperf_control_pcb);
    tcp_iperf_close(iperf_data_pcb);
  }
  */
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
static void do_detect_direction(uint8_t *buff, int buff_len)
{

  do_reverse=0;

  for(i=0; i<buff_len-8; i++) {
    if( memcmp(&buff[i],"reverse",7)==0 ) {
      do_reverse=1;
      return;
    }
  }

}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
static err_t tcp_iperf_rec(void *arg, struct tcp_pcb *iperf_pcb, struct pbuf *p, err_t err)
{

  if(p != NULL) {

    q=p;
    len = q->len;
    ptr = q->payload;


    tot_len = q->tot_len;
    total += q->tot_len;

    ptr_in = buffer;

    if(tot_len>SEG_SIZE) tot_len=SEG_SIZE;

    tcp_recved(iperf_pcb, q->tot_len);

    timeout=0;

    //if(uptime_seconds%2==0) printf("\r\ntotal: %ld", total);

    if(local_state==6 && tot_len>1) {
      ptr_in += tot_len;
    } else {
      while(q) {

        for(i=0; i<len; i++) {
          *ptr_in++ = (uint8_t)*ptr++;
        }

        q = q->next;
        if(q!=NULL) {
          len=q->len;
          ptr = q->payload;
        }
      }
    }

    c = *(ptr_in-1); //get last input char

    if( tot_len==1 && c==CLIENT_TERMINATE )  {
      local_state=0;
      tcp_iperf_close(iperf_control_pcb);
      tcp_iperf_close(iperf_data_pcb);
    } else if( local_state==0 && c==0x00 && tot_len>30) {
      local_state=1;    //received first null-term connect cookie string
      c=PARAM_EXCHANGE;
      tcp_write(iperf_control_pcb, &c, 1, TCP_WRITE_FLAG_COPY);
      tcp_output(iperf_control_pcb);
    } else if(local_state==1 && tot_len==4) { //parm len
      local_state=2;
    } else if(local_state==2 && tot_len>10) { //parm string
      do_detect_direction(buffer, tot_len);

      local_state=3;
      c=CREATE_STREAMS;
      tcp_write(iperf_control_pcb, &c, 1, TCP_WRITE_FLAG_COPY);
      tcp_output(iperf_control_pcb);
    } else if( local_state==3 && c==0x00 && tot_len>30) {
      local_state=4;    //received second connect null-term cookie string
      start_time = uptime_seconds;

      if(do_reverse) {
        local_state=5;
        c=TEST_START;
        tcp_write(iperf_control_pcb, &c, 1, TCP_WRITE_FLAG_COPY);
        tcp_output(iperf_control_pcb);
      }
    } else if( local_state==6 && c==TEST_END && tot_len==1) {
      local_state=7;
      c=IPERF_DONE; //iperf done
      tcp_write(iperf_control_pcb, &c, 1, TCP_WRITE_FLAG_COPY);
      tcp_output(iperf_control_pcb);
      start_time=0;

      if(!do_reverse) {
        local_state=0;
        tcp_close(iperf_control_pcb);
        tcp_close(iperf_data_pcb);
      }
    }

    pbuf_free(p);
  }
  return err;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static err_t tcp_iperf_sent(void *arg, struct tcp_pcb *iperf_pcb, uint16_t len)
{


  if(local_state==4) {
    local_state=5;
    c=TEST_START;
    tcp_write(iperf_control_pcb, &c, 1, TCP_WRITE_FLAG_COPY);
    tcp_output(iperf_control_pcb);
  } else if(local_state==5) {
    local_state=6;
    c=TEST_RUNNING;
    tcp_write(iperf_control_pcb, &c, 1, TCP_WRITE_FLAG_COPY);
    tcp_output(iperf_control_pcb);
    total=0;
    start_time=uptime_seconds;
  } else if(iperf_pcb!=NULL && local_state==6 && do_reverse) {

    send_len = tcp_sndbuf(iperf_data_pcb);
    err=0;

    tcp_write(iperf_data_pcb, buffer, SEG_SIZE, TCP_WRITE_FLAG_COPY);
    tcp_write(iperf_data_pcb, buffer, SEG_SIZE, TCP_WRITE_FLAG_COPY);
    /*
        for(i=0; i < send_len/SEG_SIZE; i++) {
          if( tcp_write(iperf_data_pcb, buffer, SEG_SIZE, TCP_WRITE_FLAG_COPY) == ERR_OK) {
            timeout=0;
          } else {
            err=1;
            break;
          }
        }
    */
    tcp_output(iperf_data_pcb);

    return(ERR_OK);

  } else if(local_state==7) {
    local_state=0;
    tcp_close(iperf_data_pcb);
    tcp_close(iperf_control_pcb);
  }

  return(ERR_OK);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
static err_t tcp_iperf_accept(void *arg, struct tcp_pcb *iperf_pcb, err_t err)
{

  if(local_state==0) {
    //printf("\r\ncontrol: local state: %d", local_state);
    iperf_control_pcb = iperf_pcb;
  } else if(local_state==3) {
    //printf("\r\ndata: local state: %d", local_state);
    iperf_data_pcb = iperf_pcb;
  } else {
    local_state=0;
    iperf_control_pcb = iperf_pcb;
  }



  tcp_accepted(iperf_pcb);
  tcp_recv(iperf_pcb, tcp_iperf_rec);
  tcp_sent(iperf_pcb, tcp_iperf_sent);
  tcp_poll(iperf_pcb, tcp_iperf_poll, 1);

  timeout=0;

  return(ERR_OK);
}

////////////////////////////////////////////
////////////////////////////////////////////
void tcp_iperf_init()
{
  iperf_control_pcb = tcp_new();
  tcp_bind(iperf_control_pcb, IP_ADDR_ANY, 5201);
  iperf_control_pcb = tcp_listen(iperf_control_pcb);

  tcp_accept(iperf_control_pcb, tcp_iperf_accept);
  tcp_arg( iperf_control_pcb, iperf_control_pcb);

}
