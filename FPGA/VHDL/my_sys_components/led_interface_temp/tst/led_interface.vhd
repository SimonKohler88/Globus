-- led_interface.vhd

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

entity led_interface is
	generic (
		num_led_A  : integer := 30;
		num_led_B  : integer := 30;
		num_led_C  : integer := 30;
		num_led_D  : integer := 30;
		image_rows : integer := 120
	);
	port (
		clock_clk                   : in  std_logic                     := '0';             --                clock.clk
		reset_reset                 : in  std_logic                     := '0';             --                reset.reset
		conduit_LED_A_CLK           : out std_logic;                                        --        conduit_LED_A.led_a_clk
		conduit_LED_A_DATA          : out std_logic;                                        --                     .led_a_data
		conduit_LED_B_CLK           : out std_logic;                                        --        conduit_LED_B.led_b_clk
		conduit_LED_B_DATA          : out std_logic;                                        --                     .led_b_data
		conduit_LED_C_CLK           : out std_logic;                                        --        conduit_LED_C.led_c_clk
		conduit_LED_C_DATA          : out std_logic;                                        --                     .led_c_data
		conduit_LED_D_CLK           : out std_logic;                                        --        conduit_LED_D.led_d_clk
		conduit_LED_D_DATA          : out std_logic;                                        --                     .new_signal_1
		clock_led_spi_clk           : in  std_logic                     := '0';             --        clock_led_spi.clk
		conduit_col_info            : in  std_logic_vector(8 downto 0)  := (others => '0'); --     conduit_col_info.col_nr
		conduit_fire                : in  std_logic                     := '0';             --                     .fire
		conduit_col_info_out_col_nr : out std_logic_vector(8 downto 0);                     -- conduit_col_info_out.col_nr
		conduit_col_info_out_fire   : out std_logic;                                        --                     .fire
		avs_s0_address              : in  std_logic_vector(7 downto 0)  := (others => '0'); --               avs_s0.address
		avs_s0_read                 : in  std_logic                     := '0';             --                     .read
		avs_s0_readdata             : out std_logic_vector(31 downto 0);                    --                     .readdata
		avs_s0_write                : in  std_logic                     := '0';             --                     .write
		avs_s0_writedata            : in  std_logic_vector(31 downto 0) := (others => '0'); --                     .writedata
		avs_s0_waitrequest          : out std_logic;                                        --                     .waitrequest
		asi_in1_data                : in  std_logic_vector(23 downto 0) := (others => '0'); --            asi_in1_B.data
		asi_in1_ready               : out std_logic;                                        --                     .ready
		asi_in1_valid               : in  std_logic                     := '0';             --                     .valid
		asi_in1_startofpacket       : in  std_logic                     := '0';             --                     .startofpacket
		asi_in1_endofpacket         : in  std_logic                     := '0';             --                     .endofpacket
		asi_in0_data                : in  std_logic_vector(23 downto 0) := (others => '0'); --            asi_in0_A.data
		asi_in0_ready               : out std_logic;                                        --                     .ready
		asi_in0_valid               : in  std_logic                     := '0';             --                     .valid
		asi_in0_startofpacket       : in  std_logic                     := '0';             --                     .startofpacket
		asi_in0_endofpacket         : in  std_logic                     := '0'              --                     .endofpacket
	);
end entity led_interface;

architecture rtl of led_interface is
begin

	-- TODO: Auto-generated HDL template

	conduit_LED_A_CLK <= '0';

	conduit_LED_A_DATA <= '0';

	conduit_LED_B_CLK <= '0';

	conduit_LED_B_DATA <= '0';

	conduit_LED_C_DATA <= '0';

	conduit_LED_C_CLK <= '0';

	conduit_LED_D_DATA <= '0';

	conduit_LED_D_CLK <= '0';

	conduit_col_info_out_fire <= '0';

	conduit_col_info_out_col_nr <= "000000000";

	avs_s0_readdata <= "00000000000000000000000000000000";

	avs_s0_waitrequest <= '0';

	asi_in1_ready <= '0';

	asi_in0_ready <= '0';

end architecture rtl; -- of led_interface
