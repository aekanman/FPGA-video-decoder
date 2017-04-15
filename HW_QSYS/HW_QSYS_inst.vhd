	component HW_QSYS is
		port (
			clk_125_clk                                  : in    std_logic                     := 'X';             -- clk
			clk_50_clk                                   : in    std_logic                     := 'X';             -- clk
			i2c_scl_export                               : out   std_logic;                                        -- export
			i2c_sda_export                               : inout std_logic                     := 'X';             -- export
			key_export                                   : in    std_logic_vector(3 downto 0)  := (others => 'X'); -- export
			ledg_export                                  : out   std_logic_vector(7 downto 0);                     -- export
			ledr_export                                  : out   std_logic_vector(7 downto 0);                     -- export
			lpddr2_mem_ca                                : out   std_logic_vector(9 downto 0);                     -- mem_ca
			lpddr2_mem_ck                                : out   std_logic_vector(0 downto 0);                     -- mem_ck
			lpddr2_mem_ck_n                              : out   std_logic_vector(0 downto 0);                     -- mem_ck_n
			lpddr2_mem_cke                               : out   std_logic_vector(0 downto 0);                     -- mem_cke
			lpddr2_mem_cs_n                              : out   std_logic_vector(0 downto 0);                     -- mem_cs_n
			lpddr2_mem_dm                                : out   std_logic_vector(3 downto 0);                     -- mem_dm
			lpddr2_mem_dq                                : inout std_logic_vector(31 downto 0) := (others => 'X'); -- mem_dq
			lpddr2_mem_dqs                               : inout std_logic_vector(3 downto 0)  := (others => 'X'); -- mem_dqs
			lpddr2_mem_dqs_n                             : inout std_logic_vector(3 downto 0)  := (others => 'X'); -- mem_dqs_n
			lpddr2_global_reset_reset_n                  : in    std_logic                     := 'X';             -- reset_n
			lpddr2_oct_rzqin                             : in    std_logic                     := 'X';             -- rzqin
			lpddr2_pll_ref_clk_clk                       : in    std_logic                     := 'X';             -- clk
			lpddr2_pll_sharing_pll_mem_clk               : out   std_logic;                                        -- pll_mem_clk
			lpddr2_pll_sharing_pll_write_clk             : out   std_logic;                                        -- pll_write_clk
			lpddr2_pll_sharing_pll_locked                : out   std_logic;                                        -- pll_locked
			lpddr2_pll_sharing_pll_write_clk_pre_phy_clk : out   std_logic;                                        -- pll_write_clk_pre_phy_clk
			lpddr2_pll_sharing_pll_addr_cmd_clk          : out   std_logic;                                        -- pll_addr_cmd_clk
			lpddr2_pll_sharing_pll_avl_clk               : out   std_logic;                                        -- pll_avl_clk
			lpddr2_pll_sharing_pll_config_clk            : out   std_logic;                                        -- pll_config_clk
			lpddr2_pll_sharing_pll_mem_phy_clk           : out   std_logic;                                        -- pll_mem_phy_clk
			lpddr2_pll_sharing_afi_phy_clk               : out   std_logic;                                        -- afi_phy_clk
			lpddr2_pll_sharing_pll_avl_phy_clk           : out   std_logic;                                        -- pll_avl_phy_clk
			lpddr2_status_local_init_done                : out   std_logic;                                        -- local_init_done
			lpddr2_status_local_cal_success              : out   std_logic;                                        -- local_cal_success
			lpddr2_status_local_cal_fail                 : out   std_logic;                                        -- local_cal_fail
			reset_reset_n                                : in    std_logic                     := 'X';             -- reset_n
			sd_sd_clk                                    : out   std_logic;                                        -- sd_clk
			sd_sd_cmd                                    : inout std_logic                     := 'X';             -- sd_cmd
			sd_sd_dat                                    : inout std_logic_vector(3 downto 0)  := (others => 'X'); -- sd_dat
			sram_bridge_out_sram_tcm_data_out            : inout std_logic_vector(15 downto 0) := (others => 'X'); -- sram_tcm_data_out
			sram_bridge_out_sram_tcm_address_out         : out   std_logic_vector(18 downto 0);                    -- sram_tcm_address_out
			sram_bridge_out_sram_tcm_outputenable_n_out  : out   std_logic_vector(0 downto 0);                     -- sram_tcm_outputenable_n_out
			sram_bridge_out_sram_tcm_chipselect_n_out    : out   std_logic_vector(0 downto 0);                     -- sram_tcm_chipselect_n_out
			sram_bridge_out_sram_tcm_byteenable_n_out    : out   std_logic_vector(1 downto 0);                     -- sram_tcm_byteenable_n_out
			sram_bridge_out_sram_tcm_write_n_out         : out   std_logic_vector(0 downto 0);                     -- sram_tcm_write_n_out
			video_RGB_OUT                                : out   std_logic_vector(23 downto 0);                    -- RGB_OUT
			video_HD                                     : out   std_logic;                                        -- HD
			video_VD                                     : out   std_logic;                                        -- VD
			video_DEN                                    : out   std_logic;                                        -- DEN
			video_clk_clk                                : out   std_logic                                         -- clk
		);
	end component HW_QSYS;

	u0 : component HW_QSYS
		port map (
			clk_125_clk                                  => CONNECTED_TO_clk_125_clk,                                  --             clk_125.clk
			clk_50_clk                                   => CONNECTED_TO_clk_50_clk,                                   --              clk_50.clk
			i2c_scl_export                               => CONNECTED_TO_i2c_scl_export,                               --             i2c_scl.export
			i2c_sda_export                               => CONNECTED_TO_i2c_sda_export,                               --             i2c_sda.export
			key_export                                   => CONNECTED_TO_key_export,                                   --                 key.export
			ledg_export                                  => CONNECTED_TO_ledg_export,                                  --                ledg.export
			ledr_export                                  => CONNECTED_TO_ledr_export,                                  --                ledr.export
			lpddr2_mem_ca                                => CONNECTED_TO_lpddr2_mem_ca,                                --              lpddr2.mem_ca
			lpddr2_mem_ck                                => CONNECTED_TO_lpddr2_mem_ck,                                --                    .mem_ck
			lpddr2_mem_ck_n                              => CONNECTED_TO_lpddr2_mem_ck_n,                              --                    .mem_ck_n
			lpddr2_mem_cke                               => CONNECTED_TO_lpddr2_mem_cke,                               --                    .mem_cke
			lpddr2_mem_cs_n                              => CONNECTED_TO_lpddr2_mem_cs_n,                              --                    .mem_cs_n
			lpddr2_mem_dm                                => CONNECTED_TO_lpddr2_mem_dm,                                --                    .mem_dm
			lpddr2_mem_dq                                => CONNECTED_TO_lpddr2_mem_dq,                                --                    .mem_dq
			lpddr2_mem_dqs                               => CONNECTED_TO_lpddr2_mem_dqs,                               --                    .mem_dqs
			lpddr2_mem_dqs_n                             => CONNECTED_TO_lpddr2_mem_dqs_n,                             --                    .mem_dqs_n
			lpddr2_global_reset_reset_n                  => CONNECTED_TO_lpddr2_global_reset_reset_n,                  -- lpddr2_global_reset.reset_n
			lpddr2_oct_rzqin                             => CONNECTED_TO_lpddr2_oct_rzqin,                             --          lpddr2_oct.rzqin
			lpddr2_pll_ref_clk_clk                       => CONNECTED_TO_lpddr2_pll_ref_clk_clk,                       --  lpddr2_pll_ref_clk.clk
			lpddr2_pll_sharing_pll_mem_clk               => CONNECTED_TO_lpddr2_pll_sharing_pll_mem_clk,               --  lpddr2_pll_sharing.pll_mem_clk
			lpddr2_pll_sharing_pll_write_clk             => CONNECTED_TO_lpddr2_pll_sharing_pll_write_clk,             --                    .pll_write_clk
			lpddr2_pll_sharing_pll_locked                => CONNECTED_TO_lpddr2_pll_sharing_pll_locked,                --                    .pll_locked
			lpddr2_pll_sharing_pll_write_clk_pre_phy_clk => CONNECTED_TO_lpddr2_pll_sharing_pll_write_clk_pre_phy_clk, --                    .pll_write_clk_pre_phy_clk
			lpddr2_pll_sharing_pll_addr_cmd_clk          => CONNECTED_TO_lpddr2_pll_sharing_pll_addr_cmd_clk,          --                    .pll_addr_cmd_clk
			lpddr2_pll_sharing_pll_avl_clk               => CONNECTED_TO_lpddr2_pll_sharing_pll_avl_clk,               --                    .pll_avl_clk
			lpddr2_pll_sharing_pll_config_clk            => CONNECTED_TO_lpddr2_pll_sharing_pll_config_clk,            --                    .pll_config_clk
			lpddr2_pll_sharing_pll_mem_phy_clk           => CONNECTED_TO_lpddr2_pll_sharing_pll_mem_phy_clk,           --                    .pll_mem_phy_clk
			lpddr2_pll_sharing_afi_phy_clk               => CONNECTED_TO_lpddr2_pll_sharing_afi_phy_clk,               --                    .afi_phy_clk
			lpddr2_pll_sharing_pll_avl_phy_clk           => CONNECTED_TO_lpddr2_pll_sharing_pll_avl_phy_clk,           --                    .pll_avl_phy_clk
			lpddr2_status_local_init_done                => CONNECTED_TO_lpddr2_status_local_init_done,                --       lpddr2_status.local_init_done
			lpddr2_status_local_cal_success              => CONNECTED_TO_lpddr2_status_local_cal_success,              --                    .local_cal_success
			lpddr2_status_local_cal_fail                 => CONNECTED_TO_lpddr2_status_local_cal_fail,                 --                    .local_cal_fail
			reset_reset_n                                => CONNECTED_TO_reset_reset_n,                                --               reset.reset_n
			sd_sd_clk                                    => CONNECTED_TO_sd_sd_clk,                                    --                  sd.sd_clk
			sd_sd_cmd                                    => CONNECTED_TO_sd_sd_cmd,                                    --                    .sd_cmd
			sd_sd_dat                                    => CONNECTED_TO_sd_sd_dat,                                    --                    .sd_dat
			sram_bridge_out_sram_tcm_data_out            => CONNECTED_TO_sram_bridge_out_sram_tcm_data_out,            --     sram_bridge_out.sram_tcm_data_out
			sram_bridge_out_sram_tcm_address_out         => CONNECTED_TO_sram_bridge_out_sram_tcm_address_out,         --                    .sram_tcm_address_out
			sram_bridge_out_sram_tcm_outputenable_n_out  => CONNECTED_TO_sram_bridge_out_sram_tcm_outputenable_n_out,  --                    .sram_tcm_outputenable_n_out
			sram_bridge_out_sram_tcm_chipselect_n_out    => CONNECTED_TO_sram_bridge_out_sram_tcm_chipselect_n_out,    --                    .sram_tcm_chipselect_n_out
			sram_bridge_out_sram_tcm_byteenable_n_out    => CONNECTED_TO_sram_bridge_out_sram_tcm_byteenable_n_out,    --                    .sram_tcm_byteenable_n_out
			sram_bridge_out_sram_tcm_write_n_out         => CONNECTED_TO_sram_bridge_out_sram_tcm_write_n_out,         --                    .sram_tcm_write_n_out
			video_RGB_OUT                                => CONNECTED_TO_video_RGB_OUT,                                --               video.RGB_OUT
			video_HD                                     => CONNECTED_TO_video_HD,                                     --                    .HD
			video_VD                                     => CONNECTED_TO_video_VD,                                     --                    .VD
			video_DEN                                    => CONNECTED_TO_video_DEN,                                    --                    .DEN
			video_clk_clk                                => CONNECTED_TO_video_clk_clk                                 --           video_clk.clk
		);

