void init_stm_timers (void);
uint32_t get_timer4 PROTO ((void));
uint32_t clear_timer4 PROTO ((void));
uint32_t get_timer2 PROTO ((void));
uint32_t clear_timer2 PROTO ((void));
uint32_t set_timer2 (uint32_t val );
uint32_t get_timer3 PROTO ((void));
uint32_t clear_timer3 PROTO ((void));
uint32_t set_timer3 (uint32_t val );
void TIM3_IRQHandler PROTO ((void));