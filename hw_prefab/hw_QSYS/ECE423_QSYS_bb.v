
module ECE423_QSYS (
	clk_125_clk,
	clk_50_clk,
	i2c_scl_export,
	i2c_sda_export,
	key_export,
	ledg_export,
	ledr_export,
	lpddr2_mem_ca,
	lpddr2_mem_ck,
	lpddr2_mem_ck_n,
	lpddr2_mem_cke,
	lpddr2_mem_cs_n,
	lpddr2_mem_dm,
	lpddr2_mem_dq,
	lpddr2_mem_dqs,
	lpddr2_mem_dqs_n,
	lpddr2_global_reset_reset_n,
	lpddr2_oct_rzqin,
	lpddr2_pll_ref_clk_clk,
	lpddr2_pll_sharing_pll_mem_clk,
	lpddr2_pll_sharing_pll_write_clk,
	lpddr2_pll_sharing_pll_locked,
	lpddr2_pll_sharing_pll_write_clk_pre_phy_clk,
	lpddr2_pll_sharing_pll_addr_cmd_clk,
	lpddr2_pll_sharing_pll_avl_clk,
	lpddr2_pll_sharing_pll_config_clk,
	lpddr2_pll_sharing_pll_mem_phy_clk,
	lpddr2_pll_sharing_afi_phy_clk,
	lpddr2_pll_sharing_pll_avl_phy_clk,
	lpddr2_status_local_init_done,
	lpddr2_status_local_cal_success,
	lpddr2_status_local_cal_fail,
	reset_reset_n,
	sd_sd_clk,
	sd_sd_cmd,
	sd_sd_dat,
	sram_bridge_out_sram_tcm_data_out,
	sram_bridge_out_sram_tcm_address_out,
	sram_bridge_out_sram_tcm_outputenable_n_out,
	sram_bridge_out_sram_tcm_chipselect_n_out,
	sram_bridge_out_sram_tcm_byteenable_n_out,
	sram_bridge_out_sram_tcm_write_n_out,
	video_RGB_OUT,
	video_HD,
	video_VD,
	video_DEN,
	video_clk_clk);	

	input		clk_125_clk;
	input		clk_50_clk;
	output		i2c_scl_export;
	inout		i2c_sda_export;
	input	[3:0]	key_export;
	output	[7:0]	ledg_export;
	output	[7:0]	ledr_export;
	output	[9:0]	lpddr2_mem_ca;
	output	[0:0]	lpddr2_mem_ck;
	output	[0:0]	lpddr2_mem_ck_n;
	output	[0:0]	lpddr2_mem_cke;
	output	[0:0]	lpddr2_mem_cs_n;
	output	[3:0]	lpddr2_mem_dm;
	inout	[31:0]	lpddr2_mem_dq;
	inout	[3:0]	lpddr2_mem_dqs;
	inout	[3:0]	lpddr2_mem_dqs_n;
	input		lpddr2_global_reset_reset_n;
	input		lpddr2_oct_rzqin;
	input		lpddr2_pll_ref_clk_clk;
	output		lpddr2_pll_sharing_pll_mem_clk;
	output		lpddr2_pll_sharing_pll_write_clk;
	output		lpddr2_pll_sharing_pll_locked;
	output		lpddr2_pll_sharing_pll_write_clk_pre_phy_clk;
	output		lpddr2_pll_sharing_pll_addr_cmd_clk;
	output		lpddr2_pll_sharing_pll_avl_clk;
	output		lpddr2_pll_sharing_pll_config_clk;
	output		lpddr2_pll_sharing_pll_mem_phy_clk;
	output		lpddr2_pll_sharing_afi_phy_clk;
	output		lpddr2_pll_sharing_pll_avl_phy_clk;
	output		lpddr2_status_local_init_done;
	output		lpddr2_status_local_cal_success;
	output		lpddr2_status_local_cal_fail;
	input		reset_reset_n;
	output		sd_sd_clk;
	inout		sd_sd_cmd;
	inout	[3:0]	sd_sd_dat;
	inout	[15:0]	sram_bridge_out_sram_tcm_data_out;
	output	[18:0]	sram_bridge_out_sram_tcm_address_out;
	output	[0:0]	sram_bridge_out_sram_tcm_outputenable_n_out;
	output	[0:0]	sram_bridge_out_sram_tcm_chipselect_n_out;
	output	[1:0]	sram_bridge_out_sram_tcm_byteenable_n_out;
	output	[0:0]	sram_bridge_out_sram_tcm_write_n_out;
	output	[23:0]	video_RGB_OUT;
	output		video_HD;
	output		video_VD;
	output		video_DEN;
	output		video_clk_clk;
endmodule
