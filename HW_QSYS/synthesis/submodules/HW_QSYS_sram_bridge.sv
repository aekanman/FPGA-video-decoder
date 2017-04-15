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


// $Id: //acds/rel/15.1/ip/merlin/altera_tristate_conduit_bridge/altera_tristate_conduit_bridge.sv.terp#1 $
// $Revision: #1 $
// $Date: 2015/08/09 $
// $Author: swbranch $

//Defined Terp Parameters


			    

`timescale 1 ns / 1 ns
  				      
module HW_QSYS_sram_bridge (
     input  logic clk
    ,input  logic reset
    ,input  logic request
    ,output logic grant
    ,output logic[ 15 :0 ] tcs_sram_tcm_data_in
    ,input  logic[ 15 :0 ] tcs_sram_tcm_data_out
    ,input  logic tcs_sram_tcm_data_outen
    ,inout  wire [ 15 :0 ]  sram_tcm_data_out
    ,input  logic[ 18 :0 ] tcs_sram_tcm_address_out
    ,output  wire [ 18 :0 ] sram_tcm_address_out
    ,input  logic[ 0 :0 ] tcs_sram_tcm_outputenable_n_out
    ,output  wire [ 0 :0 ] sram_tcm_outputenable_n_out
    ,input  logic[ 0 :0 ] tcs_sram_tcm_chipselect_n_out
    ,output  wire [ 0 :0 ] sram_tcm_chipselect_n_out
    ,input  logic[ 1 :0 ] tcs_sram_tcm_byteenable_n_out
    ,output  wire [ 1 :0 ] sram_tcm_byteenable_n_out
    ,input  logic[ 0 :0 ] tcs_sram_tcm_write_n_out
    ,output  wire [ 0 :0 ] sram_tcm_write_n_out
		     
   );
   reg grant_reg;
   assign grant = grant_reg;
   
   always@(posedge clk) begin
      if(reset)
	grant_reg <= 0;
      else
	grant_reg <= request;      
   end
   


 // ** Bidirectional Pin sram_tcm_data_out 
   
    reg                       sram_tcm_data_outen_reg;
  
    always@(posedge clk) begin
	 sram_tcm_data_outen_reg <= tcs_sram_tcm_data_outen;
     end
  
  
    reg [ 15 : 0 ] sram_tcm_data_out_reg;   

     always@(posedge clk) begin
	 sram_tcm_data_out_reg   <= tcs_sram_tcm_data_out[ 15 : 0 ];
      end
         
  
    assign 	sram_tcm_data_out[ 15 : 0 ] = sram_tcm_data_outen_reg ? sram_tcm_data_out_reg : 'z ;
       
  
    reg [ 15 : 0 ] 	sram_tcm_data_in_reg;
								    
    always@(posedge clk) begin
	 sram_tcm_data_in_reg <= sram_tcm_data_out[ 15 : 0 ];
    end
    
  
    assign      tcs_sram_tcm_data_in[ 15 : 0 ] = sram_tcm_data_in_reg[ 15 : 0 ];
        


 // ** Output Pin sram_tcm_address_out 
 
    reg                       sram_tcm_address_outen_reg;     
  
    always@(posedge clk) begin
	 if( reset ) begin
	   sram_tcm_address_outen_reg <= 'b0;
	 end
	 else begin
	   sram_tcm_address_outen_reg <= 'b1;
	 end
     end		     
   
 
    reg [ 18 : 0 ] sram_tcm_address_out_reg;   

     always@(posedge clk) begin
	 sram_tcm_address_out_reg   <= tcs_sram_tcm_address_out[ 18 : 0 ];
      end
          
 
    assign 	sram_tcm_address_out[ 18 : 0 ] = sram_tcm_address_outen_reg ? sram_tcm_address_out_reg : 'z ;
        


 // ** Output Pin sram_tcm_outputenable_n_out 
 
    reg                       sram_tcm_outputenable_n_outen_reg;     
  
    always@(posedge clk) begin
	 if( reset ) begin
	   sram_tcm_outputenable_n_outen_reg <= 'b0;
	 end
	 else begin
	   sram_tcm_outputenable_n_outen_reg <= 'b1;
	 end
     end		     
   
 
    reg [ 0 : 0 ] sram_tcm_outputenable_n_out_reg;   

     always@(posedge clk) begin
	 sram_tcm_outputenable_n_out_reg   <= tcs_sram_tcm_outputenable_n_out[ 0 : 0 ];
      end
          
 
    assign 	sram_tcm_outputenable_n_out[ 0 : 0 ] = sram_tcm_outputenable_n_outen_reg ? sram_tcm_outputenable_n_out_reg : 'z ;
        


 // ** Output Pin sram_tcm_chipselect_n_out 
 
    reg                       sram_tcm_chipselect_n_outen_reg;     
  
    always@(posedge clk) begin
	 if( reset ) begin
	   sram_tcm_chipselect_n_outen_reg <= 'b0;
	 end
	 else begin
	   sram_tcm_chipselect_n_outen_reg <= 'b1;
	 end
     end		     
   
 
    reg [ 0 : 0 ] sram_tcm_chipselect_n_out_reg;   

     always@(posedge clk) begin
	 sram_tcm_chipselect_n_out_reg   <= tcs_sram_tcm_chipselect_n_out[ 0 : 0 ];
      end
          
 
    assign 	sram_tcm_chipselect_n_out[ 0 : 0 ] = sram_tcm_chipselect_n_outen_reg ? sram_tcm_chipselect_n_out_reg : 'z ;
        


 // ** Output Pin sram_tcm_byteenable_n_out 
 
    reg                       sram_tcm_byteenable_n_outen_reg;     
  
    always@(posedge clk) begin
	 if( reset ) begin
	   sram_tcm_byteenable_n_outen_reg <= 'b0;
	 end
	 else begin
	   sram_tcm_byteenable_n_outen_reg <= 'b1;
	 end
     end		     
   
 
    reg [ 1 : 0 ] sram_tcm_byteenable_n_out_reg;   

     always@(posedge clk) begin
	 sram_tcm_byteenable_n_out_reg   <= tcs_sram_tcm_byteenable_n_out[ 1 : 0 ];
      end
          
 
    assign 	sram_tcm_byteenable_n_out[ 1 : 0 ] = sram_tcm_byteenable_n_outen_reg ? sram_tcm_byteenable_n_out_reg : 'z ;
        


 // ** Output Pin sram_tcm_write_n_out 
 
    reg                       sram_tcm_write_n_outen_reg;     
  
    always@(posedge clk) begin
	 if( reset ) begin
	   sram_tcm_write_n_outen_reg <= 'b0;
	 end
	 else begin
	   sram_tcm_write_n_outen_reg <= 'b1;
	 end
     end		     
   
 
    reg [ 0 : 0 ] sram_tcm_write_n_out_reg;   

     always@(posedge clk) begin
	 sram_tcm_write_n_out_reg   <= tcs_sram_tcm_write_n_out[ 0 : 0 ];
      end
          
 
    assign 	sram_tcm_write_n_out[ 0 : 0 ] = sram_tcm_write_n_outen_reg ? sram_tcm_write_n_out_reg : 'z ;
        

endmodule


