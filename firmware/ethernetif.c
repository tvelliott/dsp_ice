
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
#include "netif/etharp.h"
#include "ethernetif.h"
//#include "stm32f4x7_eth.h"
#include "main.h"
#include "config.h"
#include <string.h>

/* Network interface name */
#define IFNAME0 's'
#define IFNAME1 't'

#define MAC_ADDR0 0x01
#define MAC_ADDR1 0x02
#define MAC_ADDR2 0x03
#define MAC_ADDR3 0x04
#define MAC_ADDR4 0x05
#define MAC_ADDR5 0x06

#include "stm32f4x7_eth.h"
#include "stm32f4x7_eth_bsp.h"

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif)
{
#ifdef CHECKSUM_BY_HARDWARE
  int i;
#endif
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  memcpy(netif->hwaddr, config.mac_addr, 6);

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;


}

void eth_raw_output(uint8_t *frame, int len)
{
  //u8 *buffer = &Tx_Buff[0];
  uint8_t *buffer =  (uint8_t *)(DMATxDescToSet->Buffer1Addr);
  memcpy( buffer, frame, len);
  ETH_Prepare_Transmit_Descriptors(len);
}

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  struct pbuf *q;
  int framelength = 0;

  //u8 *buffer = &Tx_Buff[0];
  uint8_t *buffer =  (uint8_t *)(DMATxDescToSet->Buffer1Addr);

  /* copy frame from pbufs to driver buffers */
  for(q = p; q != NULL; q = q->next) {
    memcpy((u8_t*)&buffer[framelength], q->payload, q->len);
    framelength = framelength + q->len;
  }

  /* Note: padding and CRC for transmitted frame
     are automatically inserted by DMA */

  /* Prepare transmit descriptors to give to DMA*/
  ETH_Prepare_Transmit_Descriptors(framelength);
  //eem_send_frame(&Tx_Buff[0], framelength);

  return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf * low_level_input(struct netif *netif, uint8_t *buffer, uint16_t len)
{
  struct pbuf *p, *q;
  int l =0;
  uint32_t i=0;


  p = NULL;

  /* get received frame */
  //frame = ETH_Get_Received_Frame();
  //TODO: add eem usb rx frame

  /* Obtain the size of the packet and put it into the "len" variable. */
  //len = frame.length;
  //buffer = (u8 *)frame.buffer;

  /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

  /* copy received frame to pbuf chain */
  if (p != NULL) {
    for (q = p; q != NULL; q = q->next) {
      memcpy((u8_t*)q->payload, (u8_t*)&buffer[l], q->len);
      l = l + q->len;
    }
  }

  return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
err_t ethernetif_input(struct netif *netif, uint8_t *buffer, int16_t len)
{
  err_t err;
  struct pbuf *p;

  /* move received packet into a new pbuf */
  p = low_level_input(netif, buffer, len);

  /* no packet could be read, silently ignore this */
  if (p == NULL) return ERR_MEM;

  /* entry point to the LwIP stack */
  err = netif->input(p, netif);

  if (err != ERR_OK) {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
    pbuf_free(p);
    p = NULL;
  }
  return err;
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init(netif);

  return ERR_OK;
}

////////////////////////////////////////////////////////////////////////
// TODO: implement
////////////////////////////////////////////////////////////////////////
uint32_t sys_now(void)
{
  return 0;
}
