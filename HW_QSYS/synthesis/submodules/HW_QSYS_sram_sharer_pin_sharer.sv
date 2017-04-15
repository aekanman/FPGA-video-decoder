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



  
`timescale 1 ns / 1 ns

			 
module HW_QSYS_sram_sharer_pin_sharer (
 // ** Clock and Reset Connections
    input  logic clk
   ,input  logic reset

 // ** Arbiter Connections

 // *** Arbiter Grant Interface
   ,output logic ack
   ,input  logic [ 1 - 1 : 0 ] next_grant

// *** Arbiter Request Interface

    ,output logic arb_sram_tcm 
		
		     // ** Avalon TC Slave Interfaces




  // Slave Interface tcs0

    ,input  logic tcs0_request 
    ,output logic tcs0_grant   

  //sram.tcm signals
    ,input  logic[ 18 :0 ] tcs0_tcm_address_out
    ,input  logic[ 1 :0 ] tcs0_tcm_byteenable_n_out
    ,input  logic[ 0 :0 ] tcs0_tcm_outputenable_n_out
    ,input  logic[ 0 :0 ] tcs0_tcm_write_n_out
    ,output logic[ 15 :0 ]  tcs0_tcm_data_in
    ,input  logic[ 15 :0 ]  tcs0_tcm_data_out
    ,input  logic tcs0_tcm_data_outen
    ,input  logic[ 0 :0 ] tcs0_tcm_chipselect_n_out
		     
		     // ** Avalon TC Master Interface
    ,output logic request
    ,input  logic grant

		     // *** Passthrough Signals
		     
    ,output  logic[ 18 :0 ] sram_tcm_address_out
    ,output  logic[ 1 :0 ] sram_tcm_byteenable_n_out
    ,output  logic[ 0 :0 ] sram_tcm_outputenable_n_out
    ,output  logic[ 0 :0 ] sram_tcm_write_n_out
    ,input   logic[ 15 :0 ]  sram_tcm_data_in
    ,output  logic[ 15 :0 ]  sram_tcm_data_out
    ,output  logic sram_tcm_data_outen
    ,output  logic[ 0 :0 ] sram_tcm_chipselect_n_out
		     
                     // *** Shared Signals
		      	     

		     );

   function [1-1:0] getIndex;
      
      input [1-1:0] select;
      
      getIndex = 'h0;
      
      for(int i=0; i < 1; i = i + 1) begin
	 if( select[i] == 1'b1 )
           getIndex = i;
      end
      
   endfunction // getIndex

   logic[ 1 - 1 : 0 ] selected_grant;


   // Request Assignments

    assign arb_sram_tcm = tcs0_request;
   
   logic [ 1 - 1 : 0 ] concated_incoming_requests;
   
   
   assign 			      concated_incoming_requests = {						    
         tcs0_request 
				};
   
				       
   assign 			      request = | concated_incoming_requests;
  assign        tcs0_grant = selected_grant[0];

   
    // Perform Grant Selection						  
   always@(posedge clk, posedge reset) begin
     if(reset) begin
	selected_grant<=0;
	ack <= 0;
     end 
     else begin
       if(grant && (concated_incoming_requests[getIndex(selected_grant)] == 0 || selected_grant == 0 )) begin
	  if(~request)
	    selected_grant <= '0;
	  else
	    selected_grant <= next_grant;
	  
          ack<=1;
       end
       else begin
         ack<=0;
         selected_grant <= selected_grant;
       end
     end
   end // always@ (posedge clk, posedge reset)

// Passthrough Signals

    assign sram_tcm_address_out = tcs0_tcm_address_out;
    assign sram_tcm_byteenable_n_out = tcs0_tcm_byteenable_n_out;
    assign sram_tcm_outputenable_n_out = tcs0_tcm_outputenable_n_out;
    assign sram_tcm_write_n_out = tcs0_tcm_write_n_out;
    assign tcs0_tcm_data_in = sram_tcm_data_in;
    assign sram_tcm_data_out =  tcs0_tcm_data_out;
    assign sram_tcm_data_outen = tcs0_tcm_data_outen;
    assign sram_tcm_chipselect_n_out = tcs0_tcm_chipselect_n_out;
  
// Renamed Signals
  
// Shared Signals
  
endmodule   
					    



