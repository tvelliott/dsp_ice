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

module spi_master ( 
    input clk,
    input rst,
    input miso,
    output mosi,
    output sck,
    input start,
    input[7:0] data_in,
    output[7:0] data_out,
    output busy,
    output new_data
);

localparam CLK_DIV=3;

localparam IDLE = 3'd0,
           TRANSFER = 3'd1;

reg [2:0] state;

reg [7:0] data;

reg [CLK_DIV-1:0] sck_q;
assign sck = (sck_q[CLK_DIV-1] && state!=IDLE);

reg mosi;
reg [2:0] ctr;
reg [7:0] data_out;

reg new_data;
reg busy;

always @(posedge clk) begin
    if (!rst) begin
        ctr <= 3'b0;
        sck_q <= 3'h0;
        mosi <= 1'b0;
        state <= IDLE;
        data_out <= 8'b0;
        new_data <= 1'b0;
    end else begin

      case (state)
          IDLE: begin
              sck_q <= 3'h0; 
              ctr <= 3'b0;

              if (start == 1'b1) begin
                  busy<=1'b1;
                  new_data <= 1'b0;
                  data <= data_in;
                  state <= TRANSFER;
              end
          end

          TRANSFER: begin
              sck_q <= sck_q + 1'b1;

              case(sck_q) 
                3'b000: begin
                end
                3'b001: begin
                end
                3'b010: begin
                  mosi <= data[7];
                end
                3'b011: begin
                end 
                3'b100: begin
                end
                3'b101: begin
                end
                3'b110: begin
                  data <= {data[6:0], miso};
                end
                3'b111: begin
                  ctr <= ctr + 1'b1;
                  if (ctr == 3'b111) begin
                      data_out <= data;
                      busy<=1'b0;
                      new_data <= 1'b1;
                      state <= IDLE;
                  end
                end
              endcase
          end

          default: begin
              state <= IDLE;
          end
      endcase
    end
end

endmodule
