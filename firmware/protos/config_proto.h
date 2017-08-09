extern sys_config config;
void config_changed PROTO ((void));
uint32_t GetSector (uint32_t Address );
void read_config PROTO ((void));
void write_config (sys_config *configtmp );
void factory_defaults (void);
void reset_config_to_defaults (void);
