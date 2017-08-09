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


//`define USE_CORDIC 1
`undef USE_CORDIC

`include "pll.v"

`ifdef USE_CORDIC
`include "cordic.v"
`endif

`include "bram_delay.v"

`include "spi_master.v"

module top (
	input  clk,
	output LED2,
	output LED3,
	output LED4,
	output AUDIO_SDIN,
	output AUDIO_SCLK,
	output AUDIO_LRCLK,
	output AUDIO_MCLK,
	input CBSEL,
  output PMOD1_1,
  input PMOD1_2, 
  input PMOD1_3, 
  output PMOD1_4,
  input PMOD1_7,
  output PMOD1_8,
  output PMOD1_9,
  input PMOD1_10,
  input PMOD2_1, 
  input PMOD2_2,
  input PMOD2_3, 
  input PMOD2_4,   
  input PMOD2_7, 
  input PMOD2_8, 
  input PMOD2_9, 
  input PMOD2_10, 
  input PMOD3_1, 
  input PMOD3_2,
  input PMOD3_3, 
  output PMOD3_4, 
  input PMOD3_7, 
  input PMOD3_8, 
  input PMOD3_9, 
  input PMOD3_10, 
  output PMOD4_1, 
  input PMOD4_2, 
  input PMOD4_3, 
  output PMOD4_4, 
  input PMOD4_7, 
  output PMOD4_8, 
  output PMOD4_9,
  input PMOD4_10,
  input P_A6,
  input P_A7,
  input P_A9,
  input P_A10,
  input P_A11,
  input P_B6,
  input P_B7,
  input P_B8,
  input P_B9,
  input P_B11,
  input A0,
  input A1,
  input A2,
  input A3,
  input A4,
  input A5,
  input A6,
  input A7,
  input A8,
  input A9,
  input A10,
  input A11,
  input A12,
  input A13,
  input A14,
  input A15,
  inout D0,
  inout D1,
  inout D2,
  inout D3,
  inout D4,
  inout D5,
  inout D6,
  inout D7,
  inout D8,
  inout D9,
  inout D10,
  inout D11,
  inout D12,
  inout D13,
  inout D14,
  inout D15,
  input FSMC_NOE,
  input FSMC_NWE,
  input FSMC_NE1,
  output FSMC_NE4,
  input FSMC_CLK,
  input FSMC_NL_NADV,
  input FPGA_SPI_SCLK,
  input FPGA_SPI_MOSI,
  input FPGA_SPI_MISO,
  input FPGA_SPI_CS
);


  //hw reset signal comes from MCU
  reg rst_t;
  assign rst_t = CBSEL; 

  //84mhz (HCLK/2) fsmc_clk signal is from MCU for bus synchronous transfers
  reg fsmc_clk;
  assign fsmc_clk = FSMC_CLK;
  assign LED2 = !FSMC_CLK;

  reg clk_t;
  reg clk_out;


  //40MHz in,  83.333 MHz out
  pll pll1 (
    .clock_in(clk),
    .clock_out(clk_t)
  );


  reg [15:0] spi1_ctrl;
  reg [15:0] cs1_ctrl;
  //SPI Port related
  wire miso_t_1;
  //wire up to physical pins via assigns
  wire sck_t_1;
  wire mosi_t_1;
  wire cc_gpio0_1;
  wire cc_gpio2_1;
  wire cc_gpio3_1;

  reg spi_reset1;

  reg [7:0] spi_data_in_t_1;
  wire [7:0] spi_data_out_t_1;
  wire spi_busy_t_1;
  wire spi_new_data_t_1;
  reg spi_start_t_1;

  spi_master spi_m1 (
    .clk(clk_t),
    .rst(spi_reset1),
    .miso(miso_t_1),
    .mosi(mosi_t_1),
    .sck(sck_t_1),
    .start(spi_start_t_1),
    .data_in(spi_data_in_t_1),
    .data_out(spi_data_out_t_1),
    .busy(spi_busy_t_1),
    .new_data(spi_new_data_t_1)
  );


  reg [15:0] spi2_ctrl;
  reg [15:0] cs2_ctrl;

  //SPI Port related
  wire miso_t_2;
  //wire up to physical pins via assigns
  wire sck_t_2;
  wire mosi_t_2;
  wire cc_gpio0_2;
  wire cc_gpio2_2;
  wire cc_gpio3_2;

  reg spi_reset2;

  reg [7:0] spi_data_in_t_2;
  wire [7:0] spi_data_out_t_2;
  wire spi_busy_t_2;
  wire spi_new_data_t_2;
  reg spi_start_t_2;

  spi_master spi_m2 (
    .clk(clk_t),
    .rst(spi_reset2),
    .miso(miso_t_2),
    .mosi(mosi_t_2),
    .sck(sck_t_2),
    .start(spi_start_t_2),
    .data_in(spi_data_in_t_2),
    .data_out(spi_data_out_t_2),
    .busy(spi_busy_t_2),
    .new_data(spi_new_data_t_2)
  );

`ifdef USE_CORDIC
  //CORDIC related for sine wave generation
  localparam width = 16; //width of x and y
  //localparam  angle_init = 32000/1.647;
  localparam  angle_init = 8192; 

  //Xin, Yin only need to be intialized at beginning for constant amplitude sin/cos 
  reg [width-1:0] Xin=angle_init;
  reg [width-1:0] Yin=0;

  reg [31:0] angle=0;
  wire [width-1:0] COSout;
  wire [width-1:0] SINout;

  CORDIC cordic1 (
    .clock(clk_t),
    .cosine(COSout),
    .sine(SINout),
    .x_start(Xin),
    .y_start(Yin),
    .angle(angle)
  );
`endif

  //linear feedback shift register LFSR for noise waveform generation
  reg [15:0] rand16_1; 
  reg r16_lf_1; 


  //wave form generator related
  reg signed [15:0] tone1_hz;
  reg signed [15:0] tone2_hz; 
  reg signed [15:0] tone3_hz; 
  reg signed [15:0] tone4_hz; 
  reg signed [15:0] tone5_hz;
  reg signed [15:0] tone6_hz; 
  reg signed [15:0] tone7_hz; 
  reg signed [15:0] tone8_hz; 
  reg signed [20:0] left_channel; 
  reg signed [20:0] right_channel; 

  reg [31:0] angle1_hz;
  reg [31:0] angle2_hz;
  reg [31:0] angle3_hz;
  reg [31:0] angle4_hz;
  reg [31:0] angle5_hz;
  reg [31:0] angle6_hz;
  reg [31:0] angle7_hz;
  reg [31:0] angle8_hz;

  //current state of waveform generator
  reg [3:0] current_tone_state;

  //mem-mapped registers 
  reg [15:0] tone1_q=0;
  reg [15:0] tone2_q=0;
  reg [15:0] tone3_q=0;
  reg [15:0] tone4_q=0;
  reg [15:0] tone5_q=0;
  reg [15:0] tone6_q=0;
  reg [15:0] tone7_q=0;
  reg [15:0] tone8_d=0; //0x6000000e;
  reg [15:0] tone8_q=0;
  reg [15:0] waveform_q=0; //2-bit waveform selection register for all 8 channels

  reg [15:0] audio_ctrl_q=16'h0000; //bit 0,  0=wave form generator on,  1=sample-based sound generation on, bit 1 = delay on/off

  reg do_sample_mode=0;

  wire do_delay;
  assign do_delay = audio_ctrl[1];
  reg [2:0] delay_cnt;  //3-bits = ~200ms delay, noticable aliasing occurs above that 
  
  reg signed [15:0] left_sample_q=0;//sample-based waveform output.  NE4 signal is used to signal MCU to update sample-based registers 
  reg signed [15:0] right_sample_q=0;


  reg [12:0] mem_addr=13'h000;
  reg [15:0] rdata;
  reg [15:0] wdata=0;
  reg signed [15:0] l_ch_data = 16'h0000;
  reg signed [15:0] r_ch_data = 16'h0000;


  //I2S audio-related
  reg mclk; //I2S master clock
  reg [3:0] audio_cnt;
  reg signed [15:0] audio_sample;
  reg [3:0] lrcnt;
  reg sdata;  //I2S data input
  reg sclk;   //I2S sclk
  reg lrclk;  //I2S left-right clock
  
  reg signed [20:0] left_channel;
  reg signed [20:0] right_channel;
  reg signed [15:0] left_sample;
  reg signed [15:0] right_sample;
  reg signed [15:0] audio_ctrl=0;


  reg do_sample_mode;
  assign do_sample_mode = audio_ctrl[0];

  localparam WEN_DELAY=3;
  reg [WEN_DELAY-1:0] wen_sr;
  reg [WEN_DELAY-1:0] wen_sr_q;


  //wire wen;
  //assign wen = wen_sr_q[WEN_DELAY-1]; //read / write from mcu over fsmc

  wire wen;
  assign wen = (lrcnt==4'd1); 


  reg [15:0] rdata;
  reg [15:0] wdata;
  reg [15:0] wdata_q;
  reg [12:0] mem_addr;
  reg [12:0] mem_addr_d;

  //this memory geometry (13-bit x 16-bit) will be inferred as internal BRAM of fpga by yosys
  bram_delay delay(
    //.clk(fsmc_clk), //if read/write from mcu, then use fsmc_clk
    .clk(clk_t), //for audio delay 
    .wen(wen),
    .addr(mem_addr[12:0]),
    .wdata(wdata_q),
    .rdata(rdata)
  );


  //led counter and delays
	localparam BITS = 3;
	localparam LOG2DELAY = 24;
	reg [BITS+LOG2DELAY-1:0] counter = 1;
	reg [BITS-1:0] outcnt;


  //MCU / FPGA 16-bit memory interface signals
  wire [15:0] addr;
  wire [15:0] dbus_in;
  reg [15:0] dbus_out;
  reg [15:0] data_in;
  reg [15:0] addr_in;
  wire noe;
  wire nwe;
  wire ne1; //select signal for external memory address range starting at 0x60000000 on stm32f4
  reg exti_15_10_int;  //audio output sample interrupt
  wire nadv;


  //SPI port signals (not currently used)
  wire spi_sclk;
  wire spi_miso;
  wire spi_mosi;
  wire spi_cs;

  //LTC1420 ADC
  wire adc_clk10mhz;
  wire [11:0] adc_data_pins;
  reg [15:0] adc_data;
  wire adc_of;


  //lattice ICE40 specific high-z data lines so arachne-pnr can deal with it
  SB_IO #(
    .PIN_TYPE(6'b 1010_01),
    .PULLUP(1'b 0)
  ) dbus [15:0] (
    .PACKAGE_PIN({D15,D14,D13,D12,D11,D10,D9,D8,D7,D6,D5,D4,D3,D2,D1,D0}),
    .OUTPUT_ENABLE(!noe),
    .D_OUT_0(dbus_out),
    .D_IN_0(dbus_in)
  );

	always @(posedge fsmc_clk && !ne1) begin

      wen_sr <= wen_sr<<1;


      if(!nadv) begin
        addr_in <= dbus_in<<1;   //address latch 16-bit word from multiplexed address/data
        mem_addr_d <= dbus_in<<1; 
      end

      else if(!noe) begin  //mcu read from fpga operation, base address on MCU is 0x60000000


          case(addr_in[15:8]) 

            //address range 0x00xx
            8'h00: begin
              case(addr_in[7:0]) 

                //spi1 related
                8'h00:  begin
                  dbus_out <= spi1_ctrl; 
                end
                8'h02:  begin
                  dbus_out <= spi_data_in_t_1; 
                end
                8'h04:  begin
                  dbus_out <= spi_data_out_t_1; //read-only
                end
                8'h06:  begin
                  dbus_out <= cs1_ctrl; 
                end

                //spi2 related
                8'h08:  begin
                  dbus_out <= spi2_ctrl; 
                end
                8'h0a:  begin
                  dbus_out <= spi_data_in_t_2; 
                end
                8'h0c:  begin
                  dbus_out <= spi_data_out_t_2; //read-only
                end
                8'h0e:  begin
                  dbus_out <= cs2_ctrl; 
                end


                8'h10:  begin
                  dbus_out <= adc_data;
                end


                default : begin
                  dbus_out <= 16'h0000; 
                end
              endcase
            end


            //address range 0x10xx
            8'h10: begin
              case(addr_in[7:0]) 
                8'h00:  begin
                  dbus_out <= audio_ctrl; 
                end
                8'h02:  begin
                  dbus_out <= left_sample; 
                end
                8'h04:  begin
                  dbus_out <= right_sample; 
                end
                8'h06:  begin
                  dbus_out <= tone1_q; 
                end
                8'h08:  begin
                  dbus_out <= tone2_q; 
                end
                8'h0a:  begin
                  dbus_out <= tone3_q; 
                end
                8'h0c:  begin
                  dbus_out <= tone4_q; 
                end
                8'h0e:  begin
                  dbus_out <= tone5_q; 
                end
                8'h10:  begin
                  dbus_out <= tone6_q; 
                end
                8'h12:  begin
                  dbus_out <= tone7_q; 
                end
                8'h14:  begin
                  dbus_out <= tone8_q; 
                end
                8'h16:  begin
                  dbus_out <= waveform_q; 
                end
                default : begin
                  dbus_out <= 16'h0000; 
                end
              endcase
            end


            //address range 0xd0xx
            8'hd0: begin
              dbus_out <= rdata;
            end

            //address range 0xe0xx
            8'he0: begin
              dbus_out <= rdata;
            end

            //address range 0xf0xx
            8'hf0: begin
              case(addr_in[7:0]) 
                8'h00:  begin
                  dbus_out <= rand16_1;  //read-only 16-bit random number 
                end
                default : begin
                  dbus_out <= 16'h0000; 
                end
              endcase
            end


            default : begin
              dbus_out <= 16'h0000; 
            end

          endcase
      end 

      else if(!nwe) begin  //mcu write to fpga operation


          case(addr_in[15:8]) 

            //adress range 0x00xx
            8'h00: begin
              case(addr_in[7:0]) 

                //spi1 related
                8'h00:  begin
                  //bits 1, 2 are read-only
                  {spi1_ctrl[15:3],spi1_ctrl[0]} <= {dbus_in[15:3],dbus_in[0]};
                end
                8'h02:  begin
                  spi_data_in_t_1 <= dbus_in;
                end
                8'h04:  begin
                  //read only
                end
                8'h06:  begin
                  cs1_ctrl <= dbus_in;
                end

                //spi2 related
                8'h08:  begin
                  //bits 1, 2 are read-only
                  {spi2_ctrl[15:3],spi2_ctrl[0]} <= {dbus_in[15:3],dbus_in[0]};
                end
                8'h0a:  begin
                  spi_data_in_t_2 <= dbus_in;
                end
                8'h0c:  begin
                  //read only
                end
                8'h0e:  begin
                  cs2_ctrl <= dbus_in;
                end

              endcase
            end

            //adress range 0x10xx
            8'h10: begin
              case(addr_in[7:0]) 
                8'h00:  begin
                  audio_ctrl <= dbus_in;
                end
                8'h02:  begin
                  left_sample <= dbus_in;
                end
                8'h04:  begin
                  right_sample <= dbus_in;
                end
                8'h06:  begin
                  tone1_q <= dbus_in;
                end
                8'h08:  begin
                  tone2_q <= dbus_in;
                end
                8'h0a:  begin
                  tone3_q <= dbus_in;
                end
                8'h0c:  begin
                  tone4_q <= dbus_in;
                end
                8'h0e:  begin
                  tone5_q <= dbus_in;
                end
                8'h10:  begin
                  tone6_q <= dbus_in;
                end
                8'h12:  begin
                  tone7_q <= dbus_in;
                end
                8'h14:  begin
                  tone8_q <= dbus_in;
                end
                8'h16:  begin
                  waveform_q <= dbus_in;
                end
              endcase
            end

            //adress range 0xd0xx
            8'hd0: begin
              wdata <= dbus_in;
              wen_sr[WEN_DELAY-1:0] <= {WEN_DELAY{1'b1}}; 
            end

            //adress range 0xe0xx
            8'he0: begin
              wdata <= dbus_in;
              wen_sr[WEN_DELAY-1:0] <= {WEN_DELAY{1'b1}}; 
            end

            default: begin
            end

          endcase
      end
  end



	always @(posedge clk_t) begin


   if(!rst_t) begin
   end else begin

    spi1_ctrl[1] <= spi_busy_t_1;
    spi1_ctrl[2] <= spi_new_data_t_1;
    spi_start_t_1 <= spi1_ctrl[0];

    spi2_ctrl[1] <= spi_busy_t_2;
    spi2_ctrl[2] <= spi_new_data_t_2;
    spi_start_t_2 <= spi2_ctrl[0];

    //if(!ne1) begin
    //  mem_addr <= mem_addr_d; 
    //  wdata_q <= wdata;
    //  wen_sr_q <= wen_sr;
    //end  
    

    //LED and TEST signals using counter
		counter <= counter + 1;
    
    //10mhz adc clock
    adc_clk10mhz <= counter[2];

    //capture on rising edge of clk when fsmc bus is inactive
    if( counter[2:0]==3'b110 && ne1) begin
      adc_data <= adc_data_pins*16;
    end

    //pwm brightness control of leds
    if( counter%16==0 ) begin
      outcnt <= counter >> LOG2DELAY;
    end else begin
      outcnt <= 0; 
    end


    //Update input to CORDIC
    //18.63 milli-Hz resolution @80mhz
    current_tone_state <= current_tone_state+1;
    case(current_tone_state) 
      4'b0000:
        begin

          if(do_sample_mode==1'b1) begin
              //PCM waveform output.  registers controlled by MCU device
              left_channel <=  left_sample_q*16;
              right_channel <=  right_sample_q*16;
          end else begin
            //mix odd and even channels to left-right
            //delay mixed to same channel as source creates re-generative delay
            //mixing to opposite side would be non-regenerative
            left_channel <= (tone1_hz + tone3_hz + tone5_hz + tone7_hz + l_ch_data*16 );
            right_channel <= (tone2_hz + tone4_hz + tone6_hz + tone8_hz + r_ch_data*16 );
          end


        end
      4'b0001:
        begin
          angle1_hz <= angle1_hz + (18'h35*tone1_q*16);   

          case(waveform_q[1:0]) 
            2'b00:
              begin
                `ifdef USE_CORDIC
                tone1_hz <= COSout;   //sine
                `endif
              end
            2'b01:
              begin
              tone1_hz <= angle1_hz[31:16]; //sawtooth 
              end
            2'b10:
              begin
                if(angle1_hz[31]) begin
                  tone1_hz <= 16'h7fff;  //pulse 
                end else begin
                  tone1_hz <= 16'h8000;  //pulse 
                end
              end
            2'b11:
              begin
                //if(angle1_hz[31]) begin
                //  tone1_hz <= ~angle1_hz[31:16];  //triangle
                //end else begin
                //  tone1_hz <= angle1_hz[31:16]; 
                //end
                if(tone1_q!=16'h0000) begin
                  tone1_hz <= rand16_1; //noise 
                end else begin
                  tone1_hz <= 16'h0000; 
                end
              end
          endcase

        end
      4'b0010:
        begin
          `ifdef USE_CORDIC
          angle <= angle2_hz; 
          `endif
        end
      4'b0011:
        begin
          angle2_hz <= angle2_hz + (18'h35*tone2_q*16);   

          case(waveform_q[3:2]) 
            2'b00:
              begin
                `ifdef USE_CORDIC
                tone2_hz <= COSout;   //sine
                `endif
              end
            2'b01:
              begin
              tone2_hz <= angle2_hz[31:16]; //sawtooth 
              end
            2'b10:
              begin
                if(angle2_hz[31]) begin
                  tone2_hz <= 16'h7fff;  //pulse 
                end else begin
                  tone2_hz <= 16'h8000;  
                end
              end
            2'b11:
              begin
                //if(angle2_hz[31]) begin
                //  tone2_hz <= ~angle2_hz[31:16];  //triangle
                //end else begin
                //  tone2_hz <= angle2_hz[31:16]; 
                //end
                if(tone2_q!=16'h0000) begin
                  tone2_hz <= rand16_1; //noise 
                end else begin
                  tone2_hz <= 16'h0000; 
                end
              end
          endcase
        end
      4'b0100:
        begin
          `ifdef USE_CORDIC
          angle <= angle3_hz; 
          `endif
        end
      4'b0101:
        begin
          angle3_hz <= angle3_hz + (18'h35*tone3_q*16);   

          case(waveform_q[5:4]) 
            2'b00:
              begin
                `ifdef USE_CORDIC
                tone3_hz <= COSout;   //sine
                `endif
              end
            2'b01:
              begin
              tone3_hz <= angle3_hz[31:16]; //sawtooth 
              end
            2'b10:
              begin
                if(angle3_hz[31]) begin
                  tone3_hz <= 16'h7fff;  //pulse 
                end else begin
                  tone3_hz <= 16'h8000;  
                end
              end
            2'b11:
              begin
                //if(angle3_hz[31]) begin
                //  tone3_hz <= ~angle3_hz[31:16];  //triangle
                //end else begin
                //  tone3_hz <= angle3_hz[31:16]; 
                //end
                if(tone3_q!=16'h0000)  begin
                  tone3_hz <= rand16_1; //noise 
                end else begin
                  tone3_hz <= 16'h0000; 
                end
              end
          endcase
        end
      4'b0110:
        begin
          `ifdef USE_CORDIC
          angle <= angle4_hz; 
          `endif
        end
      4'b0111:
        begin
          angle4_hz <= angle4_hz + (18'h35*tone4_q*16);   

          case(waveform_q[7:6]) 
            2'b00:
              begin
                `ifdef USE_CORDIC
                tone4_hz <= COSout;   //sine
                `endif
              end
            2'b01:
              begin
              tone4_hz <= angle4_hz[31:16]; //sawtooth 
              end
            2'b10:
              begin
                if(angle4_hz[31]) begin
                  tone4_hz <= 16'h7fff;  //pulse 
                end else begin
                  tone4_hz <= 16'h8000;  
                end
              end
            2'b11:
              begin
                //if(angle4_hz[31]) begin
                //  tone4_hz <= ~angle4_hz[31:16];  //triangle
                //end else begin
                //  tone4_hz <= angle4_hz[31:16]; 
                //end
                if(tone4_q!=16'h0000) begin
                  tone4_hz <= rand16_1; //noise 
                end else begin
                  tone4_hz <= 16'h0000; 
                end
              end
          endcase
        end
      4'b1000:
        begin
          `ifdef USE_CORDIC
          angle <= angle5_hz; 
          `endif
        end
      4'b1001:
        begin
          angle5_hz <= angle5_hz + (18'h35*tone5_q*16);   

          case(waveform_q[9:8]) 
            2'b00:
              begin
                `ifdef USE_CORDIC
                tone5_hz <= COSout;   //sine
                `endif
              end
            2'b01:
              begin
              tone5_hz <= angle5_hz[31:16]; //sawtooth 
              end
            2'b10:
              begin
                if(angle5_hz[31]) begin
                  tone5_hz <= 16'h7fff;  //pulse 
                end else begin
                  tone5_hz <= 16'h8000;  
                end
              end
            2'b11:
              begin
                //if(angle5_hz[31]) begin
                //  tone5_hz <= ~angle5_hz[31:16];  //triangle
                //end else begin
                //  tone5_hz <= angle5_hz[31:16]; 
                //end
                if(tone5_q!=16'h0000) begin
                  tone5_hz <= rand16_1; //noise 
                end else begin
                  tone5_hz <= 16'h0000; 
                end
              end
          endcase
        end
      4'b1010:
        begin
          `ifdef USE_CORDIC
          angle <= angle6_hz; 
          `endif
        end
      4'b1011:
        begin
          angle6_hz <= angle6_hz + (18'h35*tone6_q*16);   

          case(waveform_q[11:10]) 
            2'b00:
              begin
                `ifdef USE_CORDIC
                tone6_hz <= COSout;   //sine
                `endif
              end
            2'b01:
              begin
              tone6_hz <= angle6_hz[31:16]; //sawtooth 
              end
            2'b10:
              begin
                if(angle6_hz[31]) begin
                  tone6_hz <= 16'h7fff;  //pulse 
                end else begin
                  tone6_hz <= 16'h8000;  
                end
              end
            2'b11:
              begin
                //if(angle6_hz[31]) begin
                //  tone6_hz <= ~angle6_hz[31:16];  //triangle
                //end else begin
                //  tone6_hz <= angle6_hz[31:16]; 
                //end
                if(tone6_q!=16'h0000) begin
                  tone6_hz <= rand16_1; //noise 
                end else begin
                  tone6_hz <= 16'h0000; 
                end
              end
          endcase
        end
      4'b1100:
        begin
          `ifdef USE_CORDIC
          angle <= angle7_hz; 
          `endif
        end
      4'b1101:
        begin
          angle7_hz <= angle7_hz + (18'h35*tone7_q*16);   

          case(waveform_q[13:12]) 
            2'b00:
              begin
                `ifdef USE_CORDIC
                tone7_hz <= COSout;   //sine
                `endif
              end
            2'b01:
              begin
              tone7_hz <= angle7_hz[31:16]; //sawtooth 
              end
            2'b10:
              begin
                if(angle7_hz[31]) begin
                  tone7_hz <= 16'h7fff;  //pulse 
                end else begin
                  tone7_hz <= 16'h8000;  
                end
              end
            2'b11:
              begin
                //if(angle7_hz[31]) begin
                //  tone7_hz <= ~angle7_hz[31:16];  //triangle
                //end else begin
                //  tone7_hz <= angle7_hz[31:16]; 
                //end
                if(tone7_q!=16'h0000) begin
                  tone7_hz <= rand16_1; //noise 
                end else begin
                  tone7_hz <= 16'h0000; 
                end
              end
          endcase
        end
      4'b1110:
        begin
          `ifdef USE_CORDIC
          angle <= angle8_hz; 
          `endif
        end
      4'b1111:
        begin
          angle8_hz <= angle8_hz + (18'h35*tone8_q*16);   
        `ifdef USE_CORDIC
          angle <= angle1_hz; 
        `endif

          case(waveform_q[15:14]) 
            2'b00:
              begin
                `ifdef USE_CORDIC
                tone8_hz <= COSout;   //sine
                `endif
              end
            2'b01:
              begin
              tone8_hz <= angle8_hz[31:16]; //sawtooth 
              end
            2'b10:
              begin
                if(angle8_hz[31]) begin
                  tone8_hz <= 16'h7fff;  //pulse 
                end else begin
                  tone8_hz <= 16'h8000;  
                end
              end
            2'b11:
              begin
                //if(angle8_hz[31]) begin
                //  tone8_hz <= ~angle8_hz[31:16];  //triangle
                //end else begin
                //  tone8_hz <= angle8_hz[31:16]; 
                //end
                if(tone8_q!=16'h0000) begin
                  tone8_hz <= rand16_1; //noise 
                end else begin
                  tone8_hz <= 16'h0000; 
                end
              end
          endcase
        end
    endcase



    //audio mclk = 80mhz/2 
    mclk <= ~mclk;

    //linear feedback shift registers for noise waveform
    if(counter[10]==0) begin
      r16_lf_1 <= !(rand16_1[15]^rand16_1[3]);
      rand16_1 <= { rand16_1[14:0], r16_lf_1 }; 
    end

    //output bits to I2S audio dac
    //with clk_t running @ 80 MHz, this configuration ends up with audio sample rate of 39.0625 KHz
    //mclk = 80e6/2 = 40 Mhz
    //sclk = 40e6/16/2 = 1.25 MHz
    //Fs = 1.25e6 / (16*2) = 39.0625 KHz

    audio_cnt <= audio_cnt+1;

    wdata_q <= { left_channel[20:13], right_channel[20:13] };

    if(do_sample_mode==1'b1) begin
        //PCM waveform output.  registers controlled by MCU device
        left_channel <=  left_sample*16;
        right_channel <=  right_sample*16;
    end 

    if( audio_cnt==0 ) begin  //time to toggle I2S sclk

      sclk <= ~sclk; 
      if(sclk) begin

        if(lrcnt==4'd2) begin
          //write on lrcnt==1
          //mem addr +1 on lrcnt==2
          //read on lrcnt=4

          delay_cnt <= delay_cnt+1;
          if(delay_cnt==0) begin
            mem_addr <= mem_addr+1;
          end
        end


        if(lrcnt==4'd4) begin

          if(do_sample_mode) begin
            exti_15_10_int <= lrclk;  //send interrupt signal to MCU for next sample data words if sample mode selected
          end 

          if(do_delay==1'b1 ) begin
            l_ch_data[15:8] <= rdata[15:8];
            r_ch_data[15:8] <= rdata[7:0];
          end else begin
            l_ch_data[15:0] <= 16'h0000; 
            r_ch_data[15:0] <= 16'h0000; 
          end
        end

        if(lrcnt==4'd0) begin //time to output frame an audio sample for left or right
          lrclk <= ~lrclk;


          if(lrclk) begin
            audio_sample <= (left_channel)>>5 ;  //shift right by 4 to get rid of some bit growth from mixing

          end else begin
            audio_sample <= (right_channel)>>5;  
          end


        end

        //MSB first to I2S dac
        sdata <= audio_sample[15-lrcnt];

        lrcnt <= lrcnt+1;

      end 

    end

   end
	end



	//assign {LED2, LED3, LED4} = ~outcnt;
	assign {LED3, LED4} = ~outcnt;

  assign AUDIO_MCLK = mclk;
  assign AUDIO_SDIN = sdata;
  assign AUDIO_SCLK = sclk;
  assign AUDIO_LRCLK = lrclk;
  //assign A[15:0] = addr[15:0];  //not currently used.  address is multiplexed with data lines for current FSMC mode
  assign FSMC_NOE = noe;
  assign FSMC_NWE = nwe;
  assign FSMC_NE1 = ne1;
  assign FSMC_NE4 = exti_15_10_int;
  assign FSMC_NL_NADV = nadv;
  assign FPGA_SPI_SCLK = spi_sclk;
  assign FPGA_SPI_MOSI = spi_mosi;
  assign FPGA_SPI_MISO = spi_miso;
  assign FPGA_SPI_CS = spi_cs;


  assign PMOD1_1 = spi_reset1;
  assign PMOD1_2 = cc_gpio2_1;
  assign miso_t_1 = PMOD1_3;
  assign PMOD1_4 = !cs1_ctrl[0];  //chip select
  assign PMOD1_10 = cc_gpio0_1;
  assign PMOD1_9 = sck_t_1;
  assign PMOD1_8 = mosi_t_1;
  assign PMOD1_7 = cc_gpio3_1;

  assign PMOD4_1 = spi_reset2;
  assign PMOD4_2 = cc_gpio2_2;
  assign miso_t_2 = PMOD4_3;
  assign PMOD4_4 = !cs2_ctrl[0];  //chip select
  assign PMOD4_10 = cc_gpio0_2;
  assign PMOD4_9 = sck_t_2;
  assign PMOD4_8 = mosi_t_2;
  assign PMOD4_7 = cc_gpio3_2;

  assign spi_reset1 = rst_t;
  assign spi_reset2 = rst_t;

  assign adc_clk10mhz = PMOD3_4;
  assign adc_of = PMOD3_10;
  assign adc_data_pins = { PMOD2_8, PMOD2_2, PMOD2_9, PMOD2_3, PMOD2_10, PMOD2_4, PMOD3_1, PMOD3_7, PMOD3_2, PMOD3_8, PMOD3_3, PMOD3_9 };
  

endmodule
