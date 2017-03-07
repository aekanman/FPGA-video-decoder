// twod_idct_st_hw.v

// This file was auto-generated as a prototype implementation of a module
// created in component editor.  It ties off all outputs to ground and
// ignores all inputs.  It needs to be edited to make it do something
// useful.
// 
// This file will not be automatically regenerated.  You should check it in
// to your version control system if you want to keep it.

`timescale 1 ps / 1 ps
module twod_idct_st_hw (
		input  wire        clk,       // clock.clk
		output wire [31:0] dst_data,  //   dst.data
		input  wire        dst_ready, //      .ready
		output wire        dst_valid, //      .valid
		input  wire        reset,     // reset.reset
		input  wire [31:0] src_data,  //   src.data
		output wire        src_ready, //      .ready
		input  wire        src_valid  //      .valid
	);

	// TODO: Auto-generated HDL template

	assign dst_valid = 1'b0;

	assign dst_data = 32'b00000000000000000000000000000000;

	assign src_ready = 1'b0;

endmodule
