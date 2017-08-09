extern int8_t do_testflash1;
extern int8_t do_testflash2;
extern int8_t do_testadc;
void telnet_write (struct tcp_pcb *tn_write_pcb , uint8_t *buffer , int len );
void close_telnet (void);
void print_prompt PROTO ((void));
void print_help PROTO ((void));
void telnet_init (void);
