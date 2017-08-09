module bram_delay(input clk, wen, input [12:0] addr, input [15:0] wdata, output reg [15:0] rdata);
  reg [15:0] mem [0:8191];
  always @(posedge clk) begin
        if (wen) mem[addr] <= wdata;
        rdata <= mem[addr];
  end
endmodule

