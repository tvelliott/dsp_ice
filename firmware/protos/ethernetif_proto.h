void eth_raw_output (uint8_t *frame , int len );
err_t ethernetif_input (struct netif *netif , uint8_t *buffer , int16_t len );
err_t ethernetif_init (struct netif *netif );
uint32_t sys_now (void);
