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



// Do NOT use this file in any product.
// ***********************************
// This file is NOT Suitable for use in any product.
// No part of this file is Suitable for use in any product.

// This file is only provided for you to read because there is some possibility that 
// it may improve your understanding of the Cyclone 5 starter kit.

// This file is provided on CONDITION that the authors shall NOT be liable for the 
// consequences of your use of any part of it.
// Do NOT use this file in any product.
// If you do not agree then delete all copies of this file in your possession.
// ALL copies or Modified copies of this file MUST contain these warnings & conditions.


// Do NOT use this file in any product.
// ***********************************



////////////////////////////////////////////////////////////////
//
// Video Pixel Converter
//     (Pixel_Conv.v)
//
// THIS is a Special YCbCr Version of the original Pixel Converter by wwkong
//
//
// OVERVIEW
//     This core converts 32-bit data in the format of Cb-Y-Cr-0 to 24-bit data
//     of format Cr-Y-Cb by discarding the Least Significant Byte (LSB)
//     AND
//     SWAPPING the RED & BLUE Bytes
//
// DEPLOYMENT
//     This core is implemented as an SOPC Builder component with a
//     single Avalon-ST data-sink (input) and Avalon-ST data-source
//     (output) signals. An associated TCL Metadata file declares this
//     core and  its interfaces for use in an SOPC Builder system.
//                           
////////////////////////////////////////////////////////////////

module Pixel_Conv
(
  // Global Signals
  clk,
  reset_n,

  // Avalon ST sink
  ready_out,
  valid_in,
  data_in,
  sop_in,
  eop_in,
  empty_in,

  // Avalon ST source
  ready_in,
  valid_out,
  data_out,
  sop_out,
  eop_out,
  empty_out
);

  parameter	SOURCE_SYMBOLS_PER_BEAT = 3;
  localparam 	SOURCE_EMPTY_WIDTH = (log2(SOURCE_SYMBOLS_PER_BEAT));
  // Global Signals
  input			clk;
  input 		reset_n;
  

  // Avalon ST sink
  output 		ready_out;
  input 		valid_in;
  input [31:0]	data_in;
  input 		sop_in;
  input 		eop_in;
  input [1:0]   empty_in;

  // Avalon ST source
  input 		ready_in;
  output 		valid_out;
  output [23:0]	data_out;
  output 		sop_out;
  output 		eop_out;
  output [(SOURCE_EMPTY_WIDTH - 1):0]	empty_out;

  function integer log2;
   	input [31:0]        value;
	for (log2=0; value>0; log2=log2+1)
		value = value>>1;
  endfunction

  assign ready_out = ready_in;
  assign valid_out = valid_in;
  assign sop_out = sop_in;
  assign eop_out = eop_in;
  assign empty_out = empty_in;

  assign data_out[23:16] = data_in[15:8];
  assign data_out[15:8]  = data_in[23:16];
  assign data_out[7:0]   = data_in[31:24];

endmodule


// synthesis translate_off
// Testbench for the Pixel_Conv

module test_Pixel_Conv;
   integer      result;
   
   reg          clk;
   reg          reset_n;
   
   reg [31:0]   data_in;
   reg          valid_in;
   reg          sop_in;
   reg          eop_in;
   reg [1:0]    empty_in;
   reg          ready_in;
   
   wire [23:0]  data_out;
   wire         valid_out;
   wire         sop_out;
   wire         eop_out;
   wire [1:0]   empty_out;
   wire         ready_out;
   
   /* The DUT */
   Pixel_Conv dut (
     // Global Signals
     .clk(clk),
     .reset_n(reset_n),
   
     // Avalon ST sink
     .ready_out(ready_out),
     .valid_in(valid_in),
     .data_in(data_in),
     .sop_in(sop_in),
     .eop_in(eop_in),
     .empty_in(empty_in),

     // Avalon ST source
     .ready_in(ready_in),
     .valid_out(valid_out),
     .data_out(data_out),
     .sop_out(sop_out),
     .eop_out(eop_out),
     .empty_out(empty_out)
   );
   
   
   
   /* Clock Generator */
   always 
   begin
      clk <= 1'b1 ; 
      #10; 
      clk <= 1'b0 ; 
      #10;
   end
   
   
      
   initial
   begin
      result <= 1;
		
		/* Reset the system */
		reset_n <= 0;
		@(negedge clk);
		reset_n <= 1;
		
		/* Testing Valid Signal */
		$display("\n### Testing Valid Signal ... ###\n");
		data_in  <= 32'h0;
      valid_in <= 1'h1;
      sop_in   <= 1'h0;
      eop_in   <= 1'h0;
      empty_in <= 2'h0;
      ready_in <= 1'h0;
      #1;
		if (data_out  == 24'h0 &&
          valid_out == 1'h1 &&
          sop_out   == 1'h0 &&
          eop_out   == 1'h0 &&
          empty_out == 2'h0 &&
          ready_out == 1'h0)
		begin
			$display("---Passed");
		end
		else
		begin
			$display("---Failed");
			result <= 0;
		end
		
		
		
		/* Testing SOP Signal */
		$display("\n### Testing SOP Signal ... ###\n");
		data_in  <= 32'h0;
      valid_in <= 1'h0;
      sop_in   <= 1'h1;
      eop_in   <= 1'h0;
      empty_in <= 2'h0;
      ready_in <= 1'h0;
      #1;
		if (data_out  == 24'h0 &&
          valid_out == 1'h0 &&
          sop_out   == 1'h1 &&
          eop_out   == 1'h0 &&
          empty_out == 2'h0 &&
          ready_out == 1'h0)
		begin
			$display("---Passed");
		end
		else
		begin
			$display("---Failed");
			result <= 0;
		end
		
		
		
		/* Testing EOP Signal */
		$display("\n### Testing EOP Signal ... ###\n");
		data_in  <= 32'h0;
      valid_in <= 1'h0;
      sop_in   <= 1'h0;
      eop_in   <= 1'h1;
      empty_in <= 2'h0;
      ready_in <= 1'h0;
      #1;
		if (data_out  == 24'h0 &&
          valid_out == 1'h0 &&
          sop_out   == 1'h0 &&
          eop_out   == 1'h1 &&
          empty_out == 2'h0 &&
          ready_out == 1'h0)
		begin
			$display("---Passed");
		end
		else
		begin
			$display("---Failed");
			result <= 0;
		end
		
		
		
		/* Testing Ready Signal */
		$display("\n### Testing Ready Signal ... ###\n");
		data_in  <= 32'h0;
      valid_in <= 1'h0;
      sop_in   <= 1'h0;
      eop_in   <= 1'h0;
      empty_in <= 2'h0;
      ready_in <= 1'h1;
      #1;
		if (data_out  == 24'h0 &&
          valid_out == 1'h0 &&
          sop_out   == 1'h0 &&
          eop_out   == 1'h0 &&
          empty_out == 2'h0 &&
          ready_out == 1'h1)
		begin
			$display("---Passed");
		end
		else
		begin
			$display("---Failed");
			result <= 0;
		end
		
		
		
		/* Testing Data Signal */
		$display("\n### Testing Data Signal ... ###\n");
		data_in  <= 32'h11223344;
      valid_in <= 1'h0;
      sop_in   <= 1'h0;
      eop_in   <= 1'h0;
      empty_in <= 2'h0;
      ready_in <= 1'h0;
      #1;
		if (data_out  == 24'h112233 &&
          valid_out == 1'h0 &&
          sop_out   == 1'h0 &&
          eop_out   == 1'h0 &&
          empty_out == 2'h0 &&
          ready_out == 1'h0)
		begin
			$display("---Passed");
		end
		else
		begin
			$display("---Failed");
			result <= 0;
		end
		
		
		
		/* Display overall result */
		#1;
		if (result == 1)
		begin
			$display("\n\n------ Simulation Passed ------\n\n");
		end
		else
		begin
			$display("\n\n------ Simulation Failed ------\n\n");
		end
		
		
		$stop;
		
       
   end
   
   

endmodule

// synthesis translate_on
