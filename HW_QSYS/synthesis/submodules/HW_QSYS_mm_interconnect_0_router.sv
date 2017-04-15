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


// $Id: //acds/rel/15.1/ip/merlin/altera_merlin_router/altera_merlin_router.sv.terp#1 $
// $Revision: #1 $
// $Date: 2015/08/09 $
// $Author: swbranch $

// -------------------------------------------------------
// Merlin Router
//
// Asserts the appropriate one-hot encoded channel based on 
// either (a) the address or (b) the dest id. The DECODER_TYPE
// parameter controls this behaviour. 0 means address decoder,
// 1 means dest id decoder.
//
// In the case of (a), it also sets the destination id.
// -------------------------------------------------------

`timescale 1 ns / 1 ns

module HW_QSYS_mm_interconnect_0_router_default_decode
  #(
     parameter DEFAULT_CHANNEL = 1,
               DEFAULT_WR_CHANNEL = -1,
               DEFAULT_RD_CHANNEL = -1,
               DEFAULT_DESTID = 11 
   )
  (output [105 - 101 : 0] default_destination_id,
   output [31-1 : 0] default_wr_channel,
   output [31-1 : 0] default_rd_channel,
   output [31-1 : 0] default_src_channel
  );

  assign default_destination_id = 
    DEFAULT_DESTID[105 - 101 : 0];

  generate
    if (DEFAULT_CHANNEL == -1) begin : no_default_channel_assignment
      assign default_src_channel = '0;
    end
    else begin : default_channel_assignment
      assign default_src_channel = 31'b1 << DEFAULT_CHANNEL;
    end
  endgenerate

  generate
    if (DEFAULT_RD_CHANNEL == -1) begin : no_default_rw_channel_assignment
      assign default_wr_channel = '0;
      assign default_rd_channel = '0;
    end
    else begin : default_rw_channel_assignment
      assign default_wr_channel = 31'b1 << DEFAULT_WR_CHANNEL;
      assign default_rd_channel = 31'b1 << DEFAULT_RD_CHANNEL;
    end
  endgenerate

endmodule


module HW_QSYS_mm_interconnect_0_router
(
    // -------------------
    // Clock & Reset
    // -------------------
    input clk,
    input reset,

    // -------------------
    // Command Sink (Input)
    // -------------------
    input                       sink_valid,
    input  [119-1 : 0]    sink_data,
    input                       sink_startofpacket,
    input                       sink_endofpacket,
    output                      sink_ready,

    // -------------------
    // Command Source (Output)
    // -------------------
    output                          src_valid,
    output reg [119-1    : 0] src_data,
    output reg [31-1 : 0] src_channel,
    output                          src_startofpacket,
    output                          src_endofpacket,
    input                           src_ready
);

    // -------------------------------------------------------
    // Local parameters and variables
    // -------------------------------------------------------
    localparam PKT_ADDR_H = 67;
    localparam PKT_ADDR_L = 36;
    localparam PKT_DEST_ID_H = 105;
    localparam PKT_DEST_ID_L = 101;
    localparam PKT_PROTECTION_H = 109;
    localparam PKT_PROTECTION_L = 107;
    localparam ST_DATA_W = 119;
    localparam ST_CHANNEL_W = 31;
    localparam DECODER_TYPE = 0;

    localparam PKT_TRANS_WRITE = 70;
    localparam PKT_TRANS_READ  = 71;

    localparam PKT_ADDR_W = PKT_ADDR_H-PKT_ADDR_L + 1;
    localparam PKT_DEST_ID_W = PKT_DEST_ID_H-PKT_DEST_ID_L + 1;



    // -------------------------------------------------------
    // Figure out the number of bits to mask off for each slave span
    // during address decoding
    // -------------------------------------------------------
    localparam PAD0 = log2ceil(64'h20000000 - 64'h0); 
    localparam PAD1 = log2ceil(64'h20100000 - 64'h20080000); 
    localparam PAD2 = log2ceil(64'h20101000 - 64'h20100800); 
    localparam PAD3 = log2ceil(64'h20101400 - 64'h20101000); 
    localparam PAD4 = log2ceil(64'h20101440 - 64'h20101400); 
    localparam PAD5 = log2ceil(64'h20101460 - 64'h20101440); 
    localparam PAD6 = log2ceil(64'h20101480 - 64'h20101460); 
    localparam PAD7 = log2ceil(64'h201014a0 - 64'h20101480); 
    localparam PAD8 = log2ceil(64'h201014c0 - 64'h201014a0); 
    localparam PAD9 = log2ceil(64'h201014e0 - 64'h201014c0); 
    localparam PAD10 = log2ceil(64'h20101500 - 64'h201014e0); 
    localparam PAD11 = log2ceil(64'h20101510 - 64'h20101500); 
    localparam PAD12 = log2ceil(64'h20101520 - 64'h20101510); 
    localparam PAD13 = log2ceil(64'h20101530 - 64'h20101520); 
    localparam PAD14 = log2ceil(64'h20101540 - 64'h20101530); 
    localparam PAD15 = log2ceil(64'h20101550 - 64'h20101540); 
    localparam PAD16 = log2ceil(64'h20101560 - 64'h20101550); 
    localparam PAD17 = log2ceil(64'h20101570 - 64'h20101560); 
    localparam PAD18 = log2ceil(64'h20101580 - 64'h20101570); 
    localparam PAD19 = log2ceil(64'h20101590 - 64'h20101580); 
    localparam PAD20 = log2ceil(64'h201015a0 - 64'h20101590); 
    localparam PAD21 = log2ceil(64'h201015a8 - 64'h201015a0); 
    localparam PAD22 = log2ceil(64'h201015b0 - 64'h201015a8); 
    // -------------------------------------------------------
    // Work out which address bits are significant based on the
    // address range of the slaves. If the required width is too
    // large or too small, we use the address field width instead.
    // -------------------------------------------------------
    localparam ADDR_RANGE = 64'h201015b0;
    localparam RANGE_ADDR_WIDTH = log2ceil(ADDR_RANGE);
    localparam OPTIMIZED_ADDR_H = (RANGE_ADDR_WIDTH > PKT_ADDR_W) ||
                                  (RANGE_ADDR_WIDTH == 0) ?
                                        PKT_ADDR_H :
                                        PKT_ADDR_L + RANGE_ADDR_WIDTH - 1;

    localparam RG = RANGE_ADDR_WIDTH-1;
    localparam REAL_ADDRESS_RANGE = OPTIMIZED_ADDR_H - PKT_ADDR_L;

      reg [PKT_ADDR_W-1 : 0] address;
      always @* begin
        address = {PKT_ADDR_W{1'b0}};
        address [REAL_ADDRESS_RANGE:0] = sink_data[OPTIMIZED_ADDR_H : PKT_ADDR_L];
      end   

    // -------------------------------------------------------
    // Pass almost everything through, untouched
    // -------------------------------------------------------
    assign sink_ready        = src_ready;
    assign src_valid         = sink_valid;
    assign src_startofpacket = sink_startofpacket;
    assign src_endofpacket   = sink_endofpacket;
    wire [PKT_DEST_ID_W-1:0] default_destid;
    wire [31-1 : 0] default_src_channel;




    // -------------------------------------------------------
    // Write and read transaction signals
    // -------------------------------------------------------
    wire write_transaction;
    assign write_transaction = sink_data[PKT_TRANS_WRITE];
    wire read_transaction;
    assign read_transaction  = sink_data[PKT_TRANS_READ];


    HW_QSYS_mm_interconnect_0_router_default_decode the_default_decode(
      .default_destination_id (default_destid),
      .default_wr_channel   (),
      .default_rd_channel   (),
      .default_src_channel  (default_src_channel)
    );

    always @* begin
        src_data    = sink_data;
        src_channel = default_src_channel;
        src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = default_destid;

        // --------------------------------------------------
        // Address Decoder
        // Sets the channel and destination ID based on the address
        // --------------------------------------------------

    // ( 0x0 .. 0x20000000 )
    if ( {address[RG:PAD0],{PAD0{1'b0}}} == 30'h0   ) begin
            src_channel = 31'b00000000000000000000010;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 11;
    end

    // ( 0x20080000 .. 0x20100000 )
    if ( {address[RG:PAD1],{PAD1{1'b0}}} == 30'h20080000   ) begin
            src_channel = 31'b10000000000000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 23;
    end

    // ( 0x20100800 .. 0x20101000 )
    if ( {address[RG:PAD2],{PAD2{1'b0}}} == 30'h20100800   ) begin
            src_channel = 31'b00000000000010000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 0;
    end

    // ( 0x20101000 .. 0x20101400 )
    if ( {address[RG:PAD3],{PAD3{1'b0}}} == 30'h20101000   ) begin
            src_channel = 31'b01000000000000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 22;
    end

    // ( 0x20101400 .. 0x20101440 )
    if ( {address[RG:PAD4],{PAD4{1'b0}}} == 30'h20101400   ) begin
            src_channel = 31'b00000010000000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 26;
    end

    // ( 0x20101440 .. 0x20101460 )
    if ( {address[RG:PAD5],{PAD5{1'b0}}} == 30'h20101440   ) begin
            src_channel = 31'b00001000000000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 10;
    end

    // ( 0x20101460 .. 0x20101480 )
    if ( {address[RG:PAD6],{PAD6{1'b0}}} == 30'h20101460   ) begin
            src_channel = 31'b00000100000000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 9;
    end

    // ( 0x20101480 .. 0x201014a0 )
    if ( {address[RG:PAD7],{PAD7{1'b0}}} == 30'h20101480   ) begin
            src_channel = 31'b00000000100000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 25;
    end

    // ( 0x201014a0 .. 0x201014c0 )
    if ( {address[RG:PAD8],{PAD8{1'b0}}} == 30'h201014a0   ) begin
            src_channel = 31'b00000000000001000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 20;
    end

    // ( 0x201014c0 .. 0x201014e0 )
    if ( {address[RG:PAD9],{PAD9{1'b0}}} == 30'h201014c0   ) begin
            src_channel = 31'b00000000000000100000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 29;
    end

    // ( 0x201014e0 .. 0x20101500 )
    if ( {address[RG:PAD10],{PAD10{1'b0}}} == 30'h201014e0   ) begin
            src_channel = 31'b00000000000000010000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 27;
    end

    // ( 0x20101500 .. 0x20101510 )
    if ( {address[RG:PAD11],{PAD11{1'b0}}} == 30'h20101500   ) begin
            src_channel = 31'b00000000000000000100000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 19;
    end

    // ( 0x20101510 .. 0x20101520 )
    if ( {address[RG:PAD12],{PAD12{1'b0}}} == 30'h20101510   ) begin
            src_channel = 31'b00000000000000000001000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 16;
    end

    // ( 0x20101520 .. 0x20101530 )
    if ( {address[RG:PAD13],{PAD13{1'b0}}} == 30'h20101520   ) begin
            src_channel = 31'b00100000000000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 4;
    end

    // ( 0x20101530 .. 0x20101540 )
    if ( {address[RG:PAD14],{PAD14{1'b0}}} == 30'h20101530   ) begin
            src_channel = 31'b00010000000000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 3;
    end

    // ( 0x20101540 .. 0x20101550 )
    if ( {address[RG:PAD15],{PAD15{1'b0}}} == 30'h20101540   ) begin
            src_channel = 31'b00000001000000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 8;
    end

    // ( 0x20101550 .. 0x20101560 )
    if ( {address[RG:PAD16],{PAD16{1'b0}}} == 30'h20101550  && write_transaction  ) begin
            src_channel = 31'b00000000010000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 21;
    end

    // ( 0x20101560 .. 0x20101570 )
    if ( {address[RG:PAD17],{PAD17{1'b0}}} == 30'h20101560  && write_transaction  ) begin
            src_channel = 31'b00000000001000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 30;
    end

    // ( 0x20101570 .. 0x20101580 )
    if ( {address[RG:PAD18],{PAD18{1'b0}}} == 30'h20101570  && write_transaction  ) begin
            src_channel = 31'b00000000000100000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 28;
    end

    // ( 0x20101580 .. 0x20101590 )
    if ( {address[RG:PAD19],{PAD19{1'b0}}} == 30'h20101580   ) begin
            src_channel = 31'b00000000000000000010000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 13;
    end

    // ( 0x20101590 .. 0x201015a0 )
    if ( {address[RG:PAD20],{PAD20{1'b0}}} == 30'h20101590   ) begin
            src_channel = 31'b00000000000000000000100;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 14;
    end

    // ( 0x201015a0 .. 0x201015a8 )
    if ( {address[RG:PAD21],{PAD21{1'b0}}} == 30'h201015a0  && read_transaction  ) begin
            src_channel = 31'b00000000000000001000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 24;
    end

    // ( 0x201015a8 .. 0x201015b0 )
    if ( {address[RG:PAD22],{PAD22{1'b0}}} == 30'h201015a8   ) begin
            src_channel = 31'b00000000000000000000001;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 5;
    end

end


    // --------------------------------------------------
    // Ceil(log2()) function
    // --------------------------------------------------
    function integer log2ceil;
        input reg[65:0] val;
        reg [65:0] i;

        begin
            i = 1;
            log2ceil = 0;

            while (i < val) begin
                log2ceil = log2ceil + 1;
                i = i << 1;
            end
        end
    endfunction

endmodule


