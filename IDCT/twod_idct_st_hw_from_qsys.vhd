-- twod_idct_st_hw.vhd

-- This file was auto-generated as a prototype implementation of a module
-- created in component editor.  It ties off all outputs to ground and
-- ignores all inputs.  It needs to be edited to make it do something
-- useful.
-- 
-- This file will not be automatically regenerated.  You should check it in
-- to your version control system if you want to keep it.

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity twod_idct_st_hw is
	port (
		CLOCK     : in  std_logic                     := '0';             -- clock.clk
		DATAOUT   : out std_logic_vector(31 downto 0);                    --   dst.data
		dst_ready : in  std_logic                     := '0';             --      .ready
		dst_valid : out std_logic;                                        --      .valid
		reset     : in  std_logic                     := '0';             -- reset.reset
		DATAIN    : in  std_logic_vector(31 downto 0) := (others => '0'); --   src.data
		src_ready : out std_logic;                                        --      .ready
		src_valid : in  std_logic                     := '0'              --      .valid
	);
end entity twod_idct_st_hw;

architecture rtl of twod_idct_st_hw is
begin

	-- TODO: Auto-generated HDL template

	dst_valid <= '0';

	DATAOUT <= "00000000000000000000000000000000";

	src_ready <= '0';

end architecture rtl; -- of twod_idct_st_hw
