extern volatile int ne4_count;
extern volatile int8_t do_sample_mode;
extern volatile int8_t fsmc_init;
extern int fpga_bytes_out;
void fpga_init PROTO ((void));
void fpga_init_interrupt (void);
uint8_t fpga_rle_decode (const uint8_t *image , int len );
void init_fsmc PROTO ((void));
void test_mem PROTO ((void));
void EXTI15_10_IRQHandler PROTO ((void));
uint8_t fpga_spi_write_reg (uint8_t val , uint8_t port , int8_t term_cs );
uint8_t fpga_spi_read_reg (uint8_t val , uint8_t port , int8_t term_cs );
