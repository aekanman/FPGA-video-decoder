module sd_cont (
		output wire [31:0]	m_address,				// avm_m0.address
		output wire				m_read,					//       .read
		input  wire				m_waitrequest_n,		//       .waitrequest
		input  wire [31:0]	m_readdata,				//       .readdata
		output wire				m_write,					//       .write
		output wire [31:0]	m_writedata,			//       .writedata
		 
		input  wire				clk,						//  clock.clk
		input  wire        	reset,					//  reset.reset
        
		input  wire [7:0]		s_address,				// avs_s0.address
		input  wire				s_read,					//       .read
		output wire [31:0]	s_readdata,				//       .readdata
		input  wire				s_write,					//       .write
		input  wire [31:0]	s_writedata,			//       .writedata
		output wire				s_waitrequest_n,		//       .waitrequest
		input wire				s_chipselect,
		
		input wire				sd_pll_clk,
		output wire				sd_clk,
		inout wire				sd_cmd,
		inout wire [3:0]		sd_dat
    );

	wire			sd_cmd_i, sd_cmd_o;
	wire [3:0]	sd_dat_i, sd_dat_o;
	wire			sd_cmd_oe, sd_dat_oe;
	
	wire			s_stb, s_cyc, s_ack;
	wire			m_we, m_cyc, m_stb, m_ack; 
	
	assign sd_cmd = sd_cmd_oe? sd_cmd_o : 1'bz;
	assign sd_dat = sd_dat_oe? sd_dat_o : 4'bzzzz;
	
	assign sd_dat_i		=	sd_dat;
	assign sd_cmd_i		=	sd_cmd;
	
	assign s_waitrequest_n	= s_ack;	
	assign s_stb				= s_chipselect;
	assign s_cyc				= s_chipselect;
	
	assign m_ack				= m_waitrequest_n;	
	assign m_write				= m_we & m_cyc & m_stb;
	assign m_read				= ~m_we & m_cyc & m_stb;
	
sdc_controller u_sdc_controller(
	
  // WISHBONE common
  .wb_clk_i		(clk),
  .wb_rst_i		(reset), 
   
   // WISHBONE slave
  .wb_dat_i		(s_writedata), 
  .wb_dat_o		(s_readdata), 
  .wb_adr_i		(s_address), 
  .wb_sel_i		(4'b1111),
  .wb_we_i		(s_write), 
  .wb_cyc_i		(s_cyc), 
  .wb_stb_i		(s_stb), 
  .wb_ack_o		(s_ack), 

  // WISHBONE master
  .m_wb_dat_o		(m_writedata), 
  .m_wb_dat_i		(m_readdata), 
  .m_wb_adr_o		(m_address), 
  .m_wb_we_o		(m_we), 
  .m_wb_cyc_o		(m_cyc), 
  .m_wb_stb_o		(m_stb), 
  .m_wb_ack_i		(m_ack), 
  
  //SD BUS  
  .sd_cmd_dat_i		(sd_cmd_i),
  .sd_cmd_out_o		(sd_cmd_o),  
  .sd_cmd_oe_o			(sd_cmd_oe), 
  .sd_dat_dat_i		(sd_dat_i), 
  .sd_dat_out_o		(sd_dat_o), 
  .sd_dat_oe_o			(sd_dat_oe), 
  .sd_clk_o_pad		(sd_clk),
  .sd_clk_i_pad		(sd_pll_clk)

);
	
endmodule