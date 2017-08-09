
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

#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/tcp.h"
#include "ethernetif.h"
#include "main.h"
#include "config.h"
#include "netconf.h"
#include <stdio.h>

struct netif netif_e0;

uint32_t TCPTimer = 0;
uint32_t ARPTimer = 0;
uint32_t IPaddress = 0;

#define TCP_TMR_INTERVAL 1

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
void lwip_init(void)
{
  struct ip_addr ipaddr;
  struct ip_addr netmask;
  struct ip_addr gw;

  mem_init();

  memp_init();

  //ethernet
  IP4_ADDR(&ipaddr, config.ip_addr[0], config.ip_addr[1], config.ip_addr[2], config.ip_addr[3] );
  IP4_ADDR(&netmask, config.netmask[0], config.netmask[1], config.netmask[2], config.netmask[3] );
  IP4_ADDR(&gw, config.gw_addr[0], config.gw_addr[1], config.gw_addr[2], config.gw_addr[3] );

  netif_add(&netif_e0, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);


  netif_set_default(&netif_e0);

  netif_set_up(&netif_e0);
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
void lwip_packet_in_eth(uint8_t *buffer, int16_t len)
{
  ethernetif_input(&netif_e0, buffer, len);
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
void LwIP_Periodic_Handle(volatile uint32_t localtime)
{
  if (localtime - TCPTimer >= TCP_TMR_INTERVAL) {
    TCPTimer =  localtime;
    tcp_tmr();
  }

  if ((localtime - ARPTimer) >= ARP_TMR_INTERVAL) {
    ARPTimer =  localtime;
    etharp_tmr();
  }
}
