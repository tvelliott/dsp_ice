extern struct netif netif_e0;
extern uint32_t TCPTimer;
extern uint32_t ARPTimer;
extern uint32_t IPaddress;
void lwip_init (void);
void lwip_packet_in_eth (uint8_t *buffer , int16_t len );
void LwIP_Periodic_Handle (volatile uint32_t localtime );
