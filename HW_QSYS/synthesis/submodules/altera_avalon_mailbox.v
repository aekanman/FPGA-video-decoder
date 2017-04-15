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


module altera_avalon_mailbox #(
  parameter DWIDTH = 32,
  parameter AWIDTH = 2
)(
  input               clk,
  input               rst_n,
  output              irq_msg,
  output              irq_space,

  input  [AWIDTH-1:0] avmm_snd_address,
  input  [DWIDTH-1:0] avmm_snd_writedata,
  input               avmm_snd_write,
  input               avmm_snd_read,
  output [DWIDTH-1:0] avmm_snd_readdata,
  output              avmm_snd_waitrequest,

  
  input  [AWIDTH-1:0] avmm_rcv_address,
  input  [DWIDTH-1:0] avmm_rcv_writedata,
  input               avmm_rcv_write,
  input               avmm_rcv_read,
  output [DWIDTH-1:0] avmm_rcv_readdata
);

// Status Register bit definition
  localparam          FULL_BIT = 1;
  localparam          MSG_PENDING_BIT = 0;
  localparam          SND_INT_BIT = 1;
  localparam          RCV_INT_BIT = 0;

// wires & registers declaration
  reg    [DWIDTH-1:0] command_reg;
  reg    [DWIDTH-1:0] pointer_reg;
  reg    [DWIDTH-1:0] status_reg;
  reg    [DWIDTH-1:0] mask_reg;
  wire   [DWIDTH-1:0] command_comb;
  wire   [DWIDTH-1:0] pointer_comb;
  wire   [DWIDTH-1:0] status_comb;
  wire   [DWIDTH-1:0] mask_comb;
  wire   [DWIDTH-1:0] snd_act_readdata;
  wire   [DWIDTH-1:0] snd_int_readdata;
  wire   [DWIDTH-1:0] snd_cmd_reg_rddata;
  wire   [DWIDTH-1:0] snd_ptr_reg_rddata; 
  wire   [DWIDTH-1:0] snd_sts_reg_rddata;
  wire   [DWIDTH-1:0] snd_msk_reg_rddata;
  wire   [DWIDTH-1:0] rcv_act_readdata;
  wire   [DWIDTH-1:0] rcv_int_readdata;
  wire   [DWIDTH-1:0] rcv_cmd_reg_rddata;
  wire   [DWIDTH-1:0] rcv_ptr_reg_rddata; 
  wire   [DWIDTH-1:0] rcv_sts_reg_rddata;
  wire   [DWIDTH-1:0] rcv_msk_reg_rddata;
  reg    [DWIDTH-1:0] readdata_with_waitstate;
  wire                cmd_reg_snd_access;
  wire                ptr_reg_snd_access;
  wire                sts_reg_snd_access;
  wire                msk_reg_snd_access;
  wire                cmd_reg_rcv_access;
  wire                ptr_reg_rcv_access;
  wire                sts_reg_rcv_access;
  wire                msk_reg_rcv_access;
  wire                snd_wr_valid;
  wire                snd_rd_valid;
  wire                rcv_rd_valid;
  wire                rcv_wr_valid;              
  wire                full;
  wire                pending;
  wire                snd_mask, rcv_mask;
  reg                 rst_for_bp;
  
// decoding of address for register target
  assign cmd_reg_snd_access = (avmm_snd_address == 2'b00);
  assign ptr_reg_snd_access = (avmm_snd_address == 2'b01);
  assign sts_reg_snd_access = (avmm_snd_address == 2'b10);
  assign msk_reg_snd_access = (avmm_snd_address == 2'b11);

  assign cmd_reg_rcv_access = (avmm_rcv_address == 2'b00);
  assign ptr_reg_rcv_access = (avmm_rcv_address == 2'b01);
  assign sts_reg_rcv_access = (avmm_rcv_address == 2'b10);
  assign msk_reg_rcv_access = (avmm_rcv_address == 2'b11);

// registers assignment
  always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      command_reg <= {(DWIDTH){1'b0}};
      pointer_reg <= {(DWIDTH){1'b0}};
      status_reg  <= {(DWIDTH){1'b0}};
      mask_reg    <= {(DWIDTH){1'b0}};  
      end
    else begin
      command_reg <= command_comb;
      pointer_reg <= pointer_comb;
      status_reg  <= status_comb;
      mask_reg    <= mask_comb;
      end
    end  
  
  assign command_comb = (snd_wr_valid & cmd_reg_snd_access) ? 
                        avmm_snd_writedata :
                        command_reg;
  assign pointer_comb = (snd_wr_valid & ptr_reg_snd_access) ? 
                        avmm_snd_writedata :
                        pointer_reg;

  assign mask_comb    = {{30{1'b0}},snd_mask, rcv_mask};
  assign snd_mask     = (snd_wr_valid & msk_reg_snd_access) ? 
                        avmm_snd_writedata[SND_INT_BIT]:
                        mask_reg[SND_INT_BIT];
  assign rcv_mask     = (rcv_wr_valid & msk_reg_rcv_access) ? 
                        avmm_rcv_writedata[RCV_INT_BIT]:
                        mask_reg[RCV_INT_BIT];

  assign status_comb  = {30'b0, full, pending};
  assign full = status_reg[FULL_BIT] ? !(cmd_reg_rcv_access & rcv_rd_valid): /*(read from rcv will set 0)*/ 
                                        (cmd_reg_snd_access & snd_wr_valid); /*(write of cmd_reg will set 1)*/
  assign pending = status_reg[MSG_PENDING_BIT] ? !(cmd_reg_rcv_access & rcv_rd_valid): /*(read from rcv will set 0)*/ 
                                                  (cmd_reg_snd_access & snd_wr_valid); /*(write of cmd_reg will set 1)*/
  // for time being, with 1 message mailbox availability, full and pending logic is the same. 
  // However, if a message queue >1 is implemented, full and pending condition will be different. 
  // The pending logic will then honor the empty status of the queue.

// Avalon MM interface handling
// Avalon MM Slave interfaces of the sender has backpressure flow control with waitrequest signal 
// Avalon MM Slave interfaces of the receiver has readWaitTime = 1, and has write capability only to the mask_reg

// assert waitreq during reset to avoid lockup
  assign snd_wr_valid = avmm_snd_write & !avmm_snd_waitrequest;
  assign snd_rd_valid = avmm_snd_read  & !avmm_snd_waitrequest;
  assign avmm_snd_waitrequest = (avmm_snd_write & cmd_reg_snd_access & status_reg[FULL_BIT]) | 
                                (avmm_snd_write & ptr_reg_snd_access & status_reg[FULL_BIT]) |  
                                // add condition for read transaction if location being read is empty, no need for now, but maybe for future
                                rst_for_bp;
  assign avmm_snd_readdata = snd_act_readdata;

  assign snd_act_readdata = snd_rd_valid ? snd_int_readdata : {(DWIDTH){1'b0}};
  assign snd_int_readdata = snd_cmd_reg_rddata |
                            snd_ptr_reg_rddata |
                            snd_sts_reg_rddata |
                            snd_msk_reg_rddata ;
  assign snd_cmd_reg_rddata = command_reg & {(DWIDTH){cmd_reg_snd_access}};
  assign snd_ptr_reg_rddata = pointer_reg & {(DWIDTH){ptr_reg_snd_access}};
  assign snd_sts_reg_rddata = status_reg  & {(DWIDTH){sts_reg_snd_access}};
  assign snd_msk_reg_rddata = mask_reg    & {(DWIDTH){msk_reg_snd_access}};


  assign rcv_wr_valid = avmm_rcv_write;
  
  assign rcv_rd_valid = avmm_rcv_read;
  assign avmm_rcv_readdata = readdata_with_waitstate;
  
  always @(posedge clk or negedge rst_n) begin
    if (!rst_n) rst_for_bp <= 1'b1;
    else        rst_for_bp <= 1'b0;
    end
  
  always @(posedge clk or negedge rst_n) begin
    if (!rst_n) readdata_with_waitstate <= {(DWIDTH){1'b0}};
    else        readdata_with_waitstate <= rcv_act_readdata;
    end
    
  assign rcv_act_readdata = rcv_rd_valid ? rcv_int_readdata : {(DWIDTH){1'b0}};
  assign rcv_int_readdata = rcv_cmd_reg_rddata |
                            rcv_ptr_reg_rddata |
                            rcv_sts_reg_rddata |
                            rcv_msk_reg_rddata ;
  assign rcv_cmd_reg_rddata = command_reg & {(DWIDTH){cmd_reg_rcv_access}};
  assign rcv_ptr_reg_rddata = pointer_reg & {(DWIDTH){ptr_reg_rcv_access}};
  assign rcv_sts_reg_rddata = status_reg  & {(DWIDTH){sts_reg_rcv_access}};
  assign rcv_msk_reg_rddata = mask_reg    & {(DWIDTH){msk_reg_rcv_access}};
         
  assign irq_space = ~status_reg[FULL_BIT] & mask_reg[SND_INT_BIT];  
  assign irq_msg   = status_reg[MSG_PENDING_BIT] & mask_reg[RCV_INT_BIT];  
  
endmodule
