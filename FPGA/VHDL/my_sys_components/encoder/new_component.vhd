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
		input_pulse_per_rev : integer := 512;
		output_pulse_div    : integer := 2
	);
	port (
		avs_s0_address          : in  std_logic_vector(7 downto 0)  := (others => '0'); --                  avs_s0.address
		avs_s0_read             : in  std_logic                     := '0';             --                        .read
		avs_s0_readdata         : out std_logic_vector(31 downto 0);                    --                        .readdata
		avs_s0_write            : in  std_logic                     := '0';             --                        .write
		avs_s0_writedata        : in  std_logic_vector(31 downto 0) := (others => '0'); --                        .writedata
		avs_s0_waitrequest      : out std_logic;                                        --                        .waitrequest
		clock_clk               : in  std_logic                     := '0';             --                   clock.clk
		reset_reset             : in  std_logic                     := '0';             --                   reset.reset
		conduit_encoder_A       : in  std_logic                     := '0';             --         conduit_encoder.enc_a
		conduit_encoder_B       : in  std_logic                     := '0';             --                        .enc_b
		conduit_encoder_index   : in  std_logic                     := '0';             --                        .enc_index
		conduit_intern_col_nr   : out std_logic_vector(8 downto 0);                     -- conduit_intern_col_info.col_nr
		conduit_intern_col_fire : out std_logic                                         --                        .fire
	);
end entity new_component;

architecture rtl of new_component is
begin

	-- TODO: Auto-generated HDL template

	avs_s0_readdata <= "00000000000000000000000000000000";

	avs_s0_waitrequest <= '0';

	conduit_intern_col_fire <= '0';

	conduit_intern_col_nr <= "000000000";

end architecture rtl; -- of new_component
