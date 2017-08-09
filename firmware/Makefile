
include ./makedefs_fp

VPATH+=./usb_eem
VPATH+=./third_party/lwip-1.4.0/src/core
VPATH+=./third_party/lwip-1.4.0/src/core/ipv4
VPATH+=./third_party/lwip-1.4.0/src/api
VPATH+=./third_party/lwip-1.4.0/src/netif
VPATH+=./third_party/lwip-1.4.0/port/STM32F4x7/arch
VPATH+=STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/STM32F4xx_StdPeriph_Driver/src/
VPATH+=STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/CMSIS/Include/
VPATH+=STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/CMSIS/Device/ST/STM32F4xx/Include
VPATH+=STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/CMSIS/DSP_Lib/Source/TransformFunctions/
VPATH+=STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/CMSIS/DSP_Lib/Source/CommonTables
VPATH+=STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/CMSIS/DSP_Lib/Source/ComplexMathFunctions/
VPATH+=STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/CMSIS/DSP_Lib/Source/BasicMathFunctions/
VPATH+=STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/CMSIS/DSP_Lib/Source/SupportFunctions/

IPATH=.
IPATH+=../inc
IPATH+=./third_party/lwip-1.4.0/port/STM32F4x7/
IPATH+=./third_party/lwip-1.4.0/src/include/
IPATH+=./third_party/lwip-1.4.0/src/include/ipv4
IPATH+=STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/STM32F4xx_StdPeriph_Driver/inc/
IPATH+=STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/CMSIS/Include/
IPATH+=STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/CMSIS/Device/ST/STM32F4xx/Include

all:
all: ${COMPILER}
all: ${COMPILER}/main.elf
all: list

list: ${COMPILER}/main.lst ${COMPILER}/main.elf
	@cp ${COMPILER}/main.bin build_image/main_image_0x8000000.bin
	@cp ${COMPILER}/main.bin /tmp 
	tail fpga/src/fpga_top.rpt

clean:
	@rm -f ${COMPILER}/* ${wildcard *~} t.out t?.out

${COMPILER}/main.lst:
	@echo ...................................................................
	@${SIZE} ${COMPILER}/main.elf 
	@${SIZE} ${COMPILER}/main.elf >${COMPILER}/main.lst
	#@${OBJDUMP} -SzhD --show-raw-insn ${COMPILER}/main.elf >>${COMPILER}/main.lst
	@${OBJDUMP} -Szh --show-raw-insn ${COMPILER}/main.elf >>${COMPILER}/main.lst

${COMPILER}:
	@mkdir -p ${COMPILER}
	@mkdir -p protos 

genprotos: 
	@touch main.c
	#./build_fpga_image.sh
	touch fpga.c
	@echo re-gen prototypes
	@cfunctions -x <telnet.c >protos/telnet_proto.h
	@cfunctions -x <std_io.c >protos/std_io_proto.h
	@cfunctions -x <main.c >protos/main_proto.h
	@cfunctions -x <config.c >protos/config_proto.h
	@cfunctions -x <flash_sst.c >protos/flash_sst_proto.h
	@cfunctions -x <crc.c >protos/crc_proto.h
	@cfunctions -x <main.c >protos/main_proto.h
	@cfunctions -x <iperf.c >protos/iperf_proto.h
	@cfunctions -x <syscalls_newlib.c >protos/syscalls_newlib_proto.h
	@cfunctions -x <stm_timers.c >protos/stm_timers_proto.h
	@cfunctions -x <ethernetif.c >protos/ethernetif_proto.h
	@cfunctions -x <netconf.c >protos/netconf_proto.h
	@cfunctions -x <fpga.c >protos/fpga_proto.h
	@cfunctions -x <Playtune.c >protos/Playtune_proto.h

${COMPILER}/main.elf: ${COMPILER}/syscalls_newlib.o
${COMPILER}/main.elf: ${COMPILER}/startup_stm32f4xx.o
${COMPILER}/main.elf: ${COMPILER}/system_stm32f4xx.o

${COMPILER}/main.elf: ${COMPILER}/main.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_it.o

${COMPILER}/main.elf: ${COMPILER}/stm_timers.o
${COMPILER}/main.elf: ${COMPILER}/fpga.o

#DSP stuff
${COMPILER}/main.elf: ${COMPILER}/arm_cfft_radix2_init_f32.o
${COMPILER}/main.elf: ${COMPILER}/arm_cfft_radix2_f32.o
${COMPILER}/main.elf: ${COMPILER}/arm_cfft_radix8_f32.o
${COMPILER}/main.elf: ${COMPILER}/arm_cmplx_mag_f32.o
${COMPILER}/main.elf: ${COMPILER}/arm_bitreversal.o
${COMPILER}/main.elf: ${COMPILER}/arm_common_tables.o
${COMPILER}/main.elf: ${COMPILER}/arm_const_structs.o

${COMPILER}/main.elf: ${COMPILER}/arm_mult_f32.o
${COMPILER}/main.elf: ${COMPILER}/arm_q15_to_float.o
${COMPILER}/main.elf: ${COMPILER}/arm_scale_f32.o
${COMPILER}/main.elf: ${COMPILER}/arm_copy_f32.o

${COMPILER}/main.elf: ${COMPILER}/std_io.o
${COMPILER}/main.elf: ${COMPILER}/config.o
${COMPILER}/main.elf: ${COMPILER}/crc.o
${COMPILER}/main.elf: ${COMPILER}/delay.o

${COMPILER}/main.elf: ${COMPILER}/stm32f4x7_eth_bsp.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4x7_eth.o
${COMPILER}/main.elf: ${COMPILER}/flash_sst.o
${COMPILER}/main.elf: ${COMPILER}/ethernetif.o
${COMPILER}/main.elf: ${COMPILER}/netconf.o
${COMPILER}/main.elf: ${COMPILER}/Playtune.o

#LWIP 

${COMPILER}/main.elf: ${COMPILER}/etharp.o
${COMPILER}/main.elf: ${COMPILER}/timers.o

#${COMPILER}/main.elf: ${COMPILER}/api_lib.o
#${COMPILER}/main.elf: ${COMPILER}/api_msg.o
${COMPILER}/main.elf: ${COMPILER}/err.o
${COMPILER}/main.elf: ${COMPILER}/netbuf.o
#${COMPILER}/main.elf: ${COMPILER}/netdb.o
#${COMPILER}/main.elf: ${COMPILER}/netifapi.o
${COMPILER}/main.elf: ${COMPILER}/sockets.o
${COMPILER}/main.elf: ${COMPILER}/tcpip.o
#${COMPILER}/main.elf: ${COMPILER}/autoip.o
${COMPILER}/main.elf: ${COMPILER}/icmp.o
#${COMPILER}/main.elf: ${COMPILER}/igmp.o
${COMPILER}/main.elf: ${COMPILER}/inet.o
${COMPILER}/main.elf: ${COMPILER}/inet_chksum.o
${COMPILER}/main.elf: ${COMPILER}/ip.o
${COMPILER}/main.elf: ${COMPILER}/ip_addr.o
${COMPILER}/main.elf: ${COMPILER}/ip_frag.o
#${COMPILER}/main.elf: ${COMPILER}/dhcp.o
#${COMPILER}/main.elf: ${COMPILER}/dns.o
#${COMPILER}/main.elf: ${COMPILER}/init.o
${COMPILER}/main.elf: ${COMPILER}/mem.o
${COMPILER}/main.elf: ${COMPILER}/memp.o
${COMPILER}/main.elf: ${COMPILER}/netif.o
${COMPILER}/main.elf: ${COMPILER}/pbuf.o
${COMPILER}/main.elf: ${COMPILER}/raw.o
${COMPILER}/main.elf: ${COMPILER}/stats.o
${COMPILER}/main.elf: ${COMPILER}/sys.o
${COMPILER}/main.elf: ${COMPILER}/tcp.o
${COMPILER}/main.elf: ${COMPILER}/tcp_in.o
${COMPILER}/main.elf: ${COMPILER}/tcp_out.o
${COMPILER}/main.elf: ${COMPILER}/udp.o
${COMPILER}/main.elf: ${COMPILER}/def.o

${COMPILER}/main.elf: ${COMPILER}/iperf.o

#telnet
${COMPILER}/main.elf: ${COMPILER}/telnet.o

${COMPILER}/main.elf: ${COMPILER}/misc.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_adc.o
#${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_can.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_crc.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_cryp.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_cryp_aes.o
#${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_cryp.o
#${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_cryp_des.o
#${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_cryp_tdes.o
#${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_dac.o
#${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_dbgmcu.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_dcmi.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_dma.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_exti.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_flash.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_fsmc.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_gpio.o
#${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_hash.o
#${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_hash_md5.o
#${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_hash_sha1.o
#${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_i2c.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_iwdg.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_pwr.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_rcc.o
#${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_rng.o
#${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_rtc.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_sdio.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_spi.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_syscfg.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_tim.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_usart.o
${COMPILER}/main.elf: ${COMPILER}/stm32f4xx_wwdg.o

#linker file
SCATTERgcc_main=main.ld
#main entry point
ENTRY_main=main

