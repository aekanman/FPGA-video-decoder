// (C) 2001-2015 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


// (C) 2001-2013 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.

 
// $Id: //acds/rel/13.1/ip/.../simpleFifo.sv.terp#1 $
// $Revision: #1 $
// $Date: 2013/09/27 $
// $Author: dmunday, korthner$

//  --------------------------------------------------------------------------------
// | simple_atlantic_fifo
//  --------------------------------------------------------------------------------

// ------------------------------------------
// Generation parameters:
//   output_name:        ECE423_QSYS_avalon_st_adapter_timing_adapter_0_fifo
//   depth:              8
//   depthBits:          3
//   data_width          36
//   use_fill_level:     true
//   in_use_ready:       true
//   in_use_valid:       true
//   out_use_ready:      true
//   out_use_valid:      true
//   out_ready_latency:  0
//   in_ready_latency:   0
// ------------------------------------------

`timescale 1ns / 100ps

module ECE423_QSYS_avalon_st_adapter_timing_adapter_0_fifo (
  output reg [3:0] fill_level,
                     
//Interface: in
  output reg         in_ready,
  input              in_valid,
  input [36-1:0]      in_data,
//Interface: out
  input              out_ready,
  output reg         out_valid,
  output reg [36-1:0] out_data,
  // Interface: clk
 input              clk,
 // Interface: reset
 input              reset_n

/*AUTOARG*/);


   // ---------------------------------------------------------------------
   //| Internal Parameters
   // ---------------------------------------------------------------------
   parameter DEPTH = 8;
   parameter DATA_WIDTH = 36;   
   parameter ADDR_WIDTH = 3;
             
   // ---------------------------------------------------------------------
   //| Signals
   // ---------------------------------------------------------------------
   reg [ADDR_WIDTH-1:0] wr_addr;
   reg [ADDR_WIDTH-1:0] rd_addr;
   reg [ADDR_WIDTH-1:0] next_wr_addr;
   reg [ADDR_WIDTH-1:0] next_rd_addr;
   reg [ADDR_WIDTH-1:0] mem_rd_addr;
   reg [DATA_WIDTH-1:0] mem[DEPTH-1:0];
   reg                  empty;
   reg                  full;

   reg [0:0] out_ready_vector;
   
   // ---------------------------------------------------------------------
   //| FIFO Status
   // ---------------------------------------------------------------------
   always @* begin
//      out_valid = !empty;
      out_ready_vector[0] = out_ready;
      in_ready  = !full;
      next_wr_addr = wr_addr + 1'b1;
      next_rd_addr = rd_addr + 1'b1;
      fill_level[ADDR_WIDTH-1:0] = wr_addr - rd_addr;
      fill_level[ADDR_WIDTH] = 0;
      if (full)
        fill_level = DEPTH[ADDR_WIDTH:0];
   end
   
   // ---------------------------------------------------------------------
   //| Manage Pointers
   // ---------------------------------------------------------------------
   always @ (negedge reset_n, posedge clk) begin
      if (!reset_n) begin
       wr_addr   <= 0;
       rd_addr   <= 0;
       empty     <= 1;
       rd_addr   <= 0;
       full      <= 0;
       out_valid <= 0;
      end else begin
       out_valid <= !empty;
       if (in_ready && in_valid) begin
        wr_addr <= next_wr_addr;
        empty   <= 0;
        if (next_wr_addr == rd_addr)
         full <= 1;
      end
              
      if (out_ready_vector[0] && out_valid) begin
        rd_addr <= next_rd_addr;
        full    <= 0;
        if (next_rd_addr == wr_addr) begin
          empty <= 1;
          out_valid <= 0;
        end
      end
                                               
      if (out_ready_vector[0] && out_valid && in_ready && in_valid) begin
        full  <= full;
        empty <= empty;
      end
    end
   end // always @ (negedge reset_n, posedge clk)
   
   always @* begin
      mem_rd_addr = rd_addr;
      if (out_ready && out_valid) begin
        mem_rd_addr = next_rd_addr;
      end
   end
   

   // ---------------------------------------------------------------------
   //| Infer Memory
   // ---------------------------------------------------------------------
   always @ (posedge clk) begin
      if (in_ready && in_valid)
        mem[wr_addr] <= in_data;
      out_data <= mem[mem_rd_addr];
   end
   
endmodule // simple_atlantic_fifo


