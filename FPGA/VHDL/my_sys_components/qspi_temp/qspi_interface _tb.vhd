-- qspi_interface.vhd

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

entity qspi_interface is
	port (
		aso_out0_data          : out std_logic_vector(7 downto 0);                    --          aso_out0.data
		aso_out0_endofpacket   : out std_logic;                                       --                  .endofpacket
		aso_out0_ready         : in  std_logic                    := '0';             --                  .ready
		aso_out0_startofpacket : out std_logic;                                       --                  .startofpacket
		aso_out0_valid         : out std_logic;                                       --                  .valid
		clock_clk              : in  std_logic                    := '0';             --             clock.clk
		reset_reset            : in  std_logic                    := '0';             --             reset.reset
		conduit_qspi_data      : in  std_logic_vector(3 downto 0) := (others => '0'); --      conduit_qspi.qspi_data
		conduit_qspi_clk       : in  std_logic                    := '0';             --                  .qspi_clk
		conduit_qspi_cs        : in  std_logic                    := '0';             --                  .qspi_cs
		conduit_ping_pong      : out std_logic                                        -- conduit_ping_pong.new_signal
	);
end entity qspi_interface;

architecture rtl of qspi_interface is
begin

	-- TODO: Auto-generated HDL template

	aso_out0_valid <= '0';

	aso_out0_data <= "00000000";

	aso_out0_startofpacket <= '0';

	aso_out0_endofpacket <= '0';

	conduit_ping_pong <= '0';

end architecture rtl; -- of qspi_interface
