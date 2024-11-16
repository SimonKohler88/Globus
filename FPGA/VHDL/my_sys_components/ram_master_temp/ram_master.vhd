-- new_component.vhd

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

entity new_component is
	generic (
		image_cols : integer := 256;
		image_rows : integer := 0
	);
	port (
		clock_clk                : in  std_logic                     := '0';             --                clock.clk
		reset_reset              : in  std_logic                     := '0';             --                reset.reset
		conduit_ping_or_pong     : in  std_logic                     := '0';             -- conduit_ping_or_pong.new_signal
		avm_m0_address           : out std_logic_vector(31 downto 0);                    --               avm_m0.address
		avm_m0_read              : out std_logic;                                        --                     .read
		avm_m0_waitrequest       : in  std_logic                     := '0';             --                     .waitrequest
		avm_m0_readdata          : in  std_logic_vector(31 downto 0) := (others => '0'); --                     .readdata
		avm_m0_readdatavalid     : in  std_logic                     := '0';             --                     .readdatavalid
		avm_m0_write             : out std_logic;                                        --                     .write
		avm_m0_writedata         : out std_logic_vector(31 downto 0);                    --                     .writedata
		asi_in0_data             : in  std_logic_vector(7 downto 0)  := (others => '0'); --              asi_in0.data
		asi_in0_ready            : out std_logic;                                        --                     .ready
		asi_in0_valid            : in  std_logic                     := '0';             --                     .valid
		asi_in0_endofpacket      : in  std_logic                     := '0';             --                     .endofpacket
		asi_in0_startofpacket    : in  std_logic                     := '0';             --                     .startofpacket
		conduit_col_info_col_nr  : in  std_logic_vector(8 downto 0)  := (others => '0'); --     conduit_col_info.col_nr
		conduit_col_info_fire    : in  std_logic                     := '0';             --                     .fire
		aso_out1_B_data          : out std_logic_vector(23 downto 0);                    --           aso_out1_B.data
		aso_out1_B_endofpacket   : out std_logic;                                        --                     .endofpacket
		aso_out1_B_ready         : in  std_logic                     := '0';             --                     .ready
		aso_out1_B_startofpacket : out std_logic;                                        --                     .startofpacket
		aso_out1_B_valid         : out std_logic;                                        --                     .valid
		aso_out0_startofpacket_1 : out std_logic;                                        --           aso_out0_A.startofpacket
		aso_out0_endofpacket_1   : out std_logic;                                        --                     .endofpacket
		aso_out0_A_data          : out std_logic_vector(23 downto 0);                    --                     .data
		aso_out0_A_ready         : in  std_logic                     := '0';             --                     .ready
		aso_out0_A_valid         : out std_logic;                                        --                     .valid
		avs_s1_address           : in  std_logic_vector(7 downto 0)  := (others => '0'); --               avs_s1.address
		avs_s1_read              : in  std_logic                     := '0';             --                     .read
		avs_s1_readdata          : out std_logic_vector(31 downto 0);                    --                     .readdata
		avs_s1_write             : in  std_logic                     := '0';             --                     .write
		avs_s1_writedata         : in  std_logic_vector(31 downto 0) := (others => '0'); --                     .writedata
		avs_s1_waitrequest       : out std_logic                                         --                     .waitrequest
	);
end entity new_component;

architecture rtl of new_component is
begin

	-- TODO: Auto-generated HDL template

	avm_m0_address <= "00000000000000000000000000000000";

	avm_m0_read <= '0';

	avm_m0_write <= '0';

	avm_m0_writedata <= "00000000000000000000000000000000";

	asi_in0_ready <= '0';

	aso_out1_B_valid <= '0';

	aso_out1_B_data <= "000000000000000000000000";

	aso_out1_B_startofpacket <= '0';

	aso_out1_B_endofpacket <= '0';

	aso_out0_A_valid <= '0';

	aso_out0_A_data <= "000000000000000000000000";

	aso_out0_startofpacket_1 <= '0';

	aso_out0_endofpacket_1 <= '0';

	avs_s1_readdata <= "00000000000000000000000000000000";

	avs_s1_waitrequest <= '0';

end architecture rtl; -- of new_component
