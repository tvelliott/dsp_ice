<html>
<img src="https://github.com/tvelliott/dsp_ice/blob/master/images/dsp_ice_pcb_rev1.jpg">
<BR>
<BR><B>DSP_ICE</B>
<BR>
<B>Project Description</B>
<p>DSP_ICE is an ICE40-8K FPGA dev system that has been developed using gcc and
Project Icestorm toolchains.  The hardware has been fully tested except for the
PDM microphone.  Firmware/HDL examples are provided to test the hardware,
including an 8-channel, multi-waveform audio synthesizer and peak amplitude,
frequency measurement via CMSIS DSP FFT functions utilizing the LTC1420 ADC
expansion module.
</p>
<BR>
<B>Why Develop Another ICE40 Dev Board?</B>
<p>
DSP ICE was developed because other boards didn't have all the features desired.
The ultimate goal is to develop an SDR with swappable analog/RF front-ends.
Desired features that drove the development were:
<BR>1) 10/100 Ethernet
<BR>2) 8K ICE 40 device that could be done with 5mil spaced, 4-layer pcb
<BR>3) I2S audio output
<BR>4) STM32F4 device that is foot-print compatible with F7 and H7 series for future
<BR>5) 16-bit FSMC memory-bus interface between the MCU and FPGA
<BR>6) Low-noise power supplies with plenty of current for expansion modules
<BR>7) Plenty of expansion I/O pins designed to utilize upper limits on frequency with ICE40
</p>

<H3><a href="https://raw.githubusercontent.com/tvelliott/dsp_ice/master/firmware/audio_synth_test/dsp_ice_audio_synth_test.mp3">8-channel audio synth test.mp3</a> audio output from example audio-synth implemented in Verilog</H3> 

<BR>
<BR>
<B>Features:</B>
<BR>
<BR>STM32F417 MCU  (upgradable with drop-in for F7 and H7 series up to 400 MHz)
<BR>ICE40HX8K-CT256 FPGA (largest ICE40 device available)
<BR>16-Bit FSMC interface between MCU <-> FPGA.  Interface clock is running @ 80 MHz
<BR>10-100 Mbps Ethernet interface  (firmware includes iperf3 server for testing bandwidth)
<BR>ST-Link programming header  (can use STM32F4-Discovery as a programming device)
<BR>I2S Audio interface  (firmware includes example of 8-channel audio synth with sawtooth, pulse, noise waveforms and a song playback via I2S)
<BR>4 PMOD-Like ports  (include 5V and 3.3V and 8 I/O pins x 4)
<BR>USB FTDI-based serial interface  (update 8/13/2017 firmware to test the uart interface has been added)
<BR>40 MHz TCXO crystal driving both FPGA and MCU  (MCU clock div/2)

<BR>
<BR>
<B>Verilog Source Is Here</B>
<BR>
<a href="https://github.com/tvelliott/dsp_ice/tree/master/firmware/fpga/src">/firmware/fpga/src</a>
<BR>

<BR>
<B>DSP ICE MCU 16-bit Memory-Mapped FPGA registers</B>
<BR>
<pre>
0x60000000   spi1_ctrl
0x60000002   spi1_data_in
0x60000004   spi1_data_out
0x60000006   spi1_cs_ctrl

0x60000008   spi2_ctrl
0x6000000a   spi2_data_in
0x6000000c   spi2_data_out
0x6000000e   spi2_cs_ctrl

0x60000010   adc_data  (LTC1420 hw module, read-only, bit0 is OF) 

0x60001000   audio_ctrl
0x60001002   left_sample
0x60001004   right_sample
0x60001006   tone1_freq
0x60001008   tone2_freq
0x6000100a   tone3_freq
0x6000100c   tone4_freq
0x6000100e   tone5_freq
0x60001010   tone6_freq
0x60001012   tone7_freq
0x60001014   tone8_freq
0x60001016   waveform_ctrl

0x6000f000   16-bit random number (ro)
</pre>


<B>PCB Design</B>
<BR>4 layers
<BR>Designed for OSH Park 4 layer service.  (5 mil spacing)
<BR>Have assembled 4 boards without any connectivity issues on the 256-pin BGA

<B>Left To Do</B>
<BR>The PDM michrophone has not been tested yet due to trouble getting the part.  
<BR>Digikey has the part in stock, but Cirrus Logic will not allow them to sell 
<BR>it to certain individuals including the author of this project. There were
<BR>no further explanations from Cirrus Logic.
<BR>
<BR>
<B>Building The Firmware</B>
<PRE>
Instructions (tested on Debian 8 system)

1) download GNU ARM Embedded Toolchain from here:

<a href="https://launchpad.net/gcc-arm-embedded"> https://launchpad.net/gcc-arm-embedded </a>

The test firmare was compiled with version:
gcc-arm-none-eabi-5_4-2016q3-20160926-linux.tar.bz2

2) *edit the makedefs_fp and change PRE_PREFIX to point to the absolute path of
the toolchain directory *optionally,  move the uncompressed toolchain folder to
the project folder and rename to gnu_arm_toolchain

3) From the project directory, In order to re-build the FPGA binary image, type
'sh build_fpga_image.sh'  (requires that Project Icestorm tools are installed)


  yosys  (synthesis tool)  <a href="http://www.clifford.at/icestorm/"> http://www.clifford.at/icestorm/</a>
  ice-tools  (tools for maniuplating text/binaries and programming dev boards) 
  arache-pnr  (place-and-route)  <a href="https://github.com/cseed/arachne-pnr"> https://github.com/cseed/arachne-pnr </a>

  Note:  The fpga binary configuration image will be compressed and converted
to a header file for inclusion to the MCU firmware image:  fpga_image.h The
compressed image in fpga_image.h will be included in fpga.c on the next mcu
firmware build  (step 4) The MCU uncompresses and transfers the fpga config
image during startup.

4) From the project directory, type 'make clean' followed by  'make' The
resulting binary will be copied to /tmp folder and also the build_image folder
in the current project directory Note: the prototypes in protos can be
re-generated with 'make genprotos' if cfunctions is installed.

5) An ST-Link programmer can be used to load the resulting binary
(main_image_0x8000000.bin) to address 0x8000000

   A $15 STM32F4 Discovery board with the jumpers set to "ST Link"  (pulled
off) and a straight-thru, 6-pin 0.1" pitch cable will work.

   Follow instructions for compiling OpenOCD  <a href="http://openocd.org/"> http://openocd.org/</a>

   start the debugger:  'sudo src/openocd -s tcl -f tcl/board/stm32f4discovery.cfg'

   'telnet localhost 4444'
    > reset halt
    > flash write_image erase build_image/main_image_0x8000000.bin 0x8000000

6) Test the hardware.
   telnet 192.168.1.240   (default ip address.  edit this in config.c)
   
Trying 192.168.1.240...
Connected to 192.168.1.240.
Escape character is '^]'.

Connected To STM32F417 Running @ 168 MHz
DSP_Ice 192.168.1.240:~$ help

Available commands:

testmem
testsnd
testflash1
testflash2
testadc
reset
exit

DSP_Ice 192.168.1.240:~$ testadc
<B>Example measurement of 500 KHz sine wave, 100mVpp utilize LTC1420 ADC HW module</B>
<B>This example utilizes the CMSIS DSP Co-Processor functions of the MCU for FFT calcs.</B>
<B>Using DMA to transfer ADC results from FPGA BRAM would dramatically improve sample rate</B>
peak signal 502675.781 Hz,  0.107 Volts p-p, elapsed time 3.51 ms  
<a href="https://github.com/tvelliott/dsp_ice/blob/master/hw_modules/ltc1420_adc/fft_plot_500khz_100mVpp.gif">[see fft plot 500khz, 100mV p-p]</a> 
<a href="https://github.com/tvelliott/dsp_ice/blob/master/hw_modules/ltc1420_adc/ltc1420_capture_3.5vpp_no_filter_325khz_freqdomain.gif">[see fft plot 325Khz, 3500mV p-p]</a> 
<a href="https://github.com/tvelliott/dsp_ice/blob/master/hw_modules/ltc1420_adc/ltc1420_capture_3.5vpp_no_filter_500khz_timedomain.gif">[see time-domain plot 500khz, 3500mV p-p]</a> 
 
DSP_Ice 192.168.1.240:~$ testmem   (this tests the 16-bit FSMC memmory-mapped, MCU<->FPGA interface)

x100x (FPGA D-REG), 4015677440 bytes r/w random access, current vals: f3e7,f3e7, errors: 0
0x100x (FPGA D-REG), 4015685632 bytes r/w random access, current vals: 69c4,69c4, errors: 0
0x100x (FPGA D-REG), 4015693824 bytes r/w random access, current vals: af9a,af9a, errors: 0
0x100x (FPGA D-REG), 4015702016 bytes r/w random access, current vals: bc6a,bc6a, errors: 0
0x100x (FPGA D-REG), 4015710208 bytes r/w random access, current vals: 8732,8732, errors: 0
0x100x (FPGA D-REG), 4015718400 bytes r/w random access, current vals: 06f4,06f4, errors: 0
0x100x (FPGA D-REG), 4015726592 bytes r/w random access, current vals: 32af,32af, errors: 0
.....

DSP_Ice 192.168.1.240:~$ sh test_iperf.sh

Connecting to host 192.168.1.240, port 5201
[  4] local 192.168.1.3 port 45612 connected to 192.168.1.240 port 5201
[ ID] Interval           Transfer     Bandwidth       Retr  Cwnd
[  4]   0.00-1.00   sec  8.91 MBytes  74.7 Mbits/sec    0   14.3 KBytes       
[  4]   1.00-2.00   sec  8.88 MBytes  74.5 Mbits/sec    0   14.3 KBytes       
[  4]   2.00-3.00   sec  8.86 MBytes  74.3 Mbits/sec    0   14.3 KBytes       
[  4]   3.00-4.00   sec  8.86 MBytes  74.3 Mbits/sec    0   14.3 KBytes       
[  4]   4.00-5.00   sec  8.88 MBytes  74.5 Mbits/sec    0   14.3 KBytes       
[  4]   5.00-6.00   sec  8.87 MBytes  74.4 Mbits/sec    0   14.3 KBytes       
[  4]   6.00-7.00   sec  8.86 MBytes  74.3 Mbits/sec    0   14.3 KBytes       
[  4]   7.00-8.00   sec  8.86 MBytes  74.3 Mbits/sec    0   14.3 KBytes       
[  4]   8.00-9.00   sec  8.86 MBytes  74.3 Mbits/sec    0   14.3 KBytes       

iperf Done.

....
DSP_Ice 192.168.1.240:~$ testsnd   (song playback via 8-channel audio synth implemented on fpga)

change instrument: ch: 0,  instr: 29, mask: 5555
chan: 0, freq: 554
change instrument: ch: 1,  instr: 37, mask: 5555
chan: 1, freq: 92
change instrument: ch: 2,  instr: 16, mask: 5575
chan: 2, freq: 233
change instrument: ch: 3,  instr: 29, mask: 5575
chan: 3, freq: 370
change instrument: ch: 4,  instr: 16, mask: 5775
chan: 4, freq: 131
change instrument: ch: 5,  instr: 16, mask: 5f75
chan: 5, freq: 131
duration: 91
duration: 91
chan: 0, freq: 494
chan: 3, freq: 330
duration: 91
duration: 91
chan: 2, freq: 208
chan: 0, freq: 554
chan: 3, freq: 370
duration: 91
duration: 91
chan: 0, freq: 494
chan: 3, freq: 330
duration: 91
duration: 91
chan: 0, freq: 554
chan: 3, freq: 370
duration: 91
duration: 91

<B>Images Of LTC1420 HW Module and PCB cad rendering</B>
<img src="https://github.com/tvelliott/dsp_ice/blob/master/hw_modules/ltc1420_adc/ltc1420_hw_module.jpg">
<img src="https://github.com/tvelliott/dsp_ice/blob/master/images/dsp_ice_pcb_cad_render.gif">

....
</PRE>

<B>Links To Similar Projects</B>
<BR>
<a href="https://github.com/emeb/iceRadio">https://github.com/emeb/iceRadio</a>
<BR><BR>
<B>Links To Related Resources</B>
<BR>
<BR><a href="https://launchpad.net/gcc-arm-embedded"> https://launchpad.net/gcc-arm-embedded </a>
<BR><a href="http://www.clifford.at/icestorm/"> http://www.clifford.at/icestorm/</a>
<BR><a href="https://github.com/cseed/arachne-pnr"> https://github.com/cseed/arachne-pnr </a>
<BR><a href="http://openocd.org/"> http://openocd.org/</a>

</html>
