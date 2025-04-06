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

entity qspi_interface_tb is
end entity qspi_interface_tb;

architecture rtl of qspi_interface_tb is

	component qspi_interface is
	port (
		aso_out0_data          : out std_logic_vector(25 downto 0);                    --          aso_out0.data
		--aso_out0_endofpacket   : out std_logic;                                       --                  .endofpacket
		aso_out0_ready         : in  std_logic                    := '0';             --                  .ready
		--aso_out0_startofpacket : out std_logic;                                       --                  .startofpacket
		aso_out0_valid         : out std_logic;                                       --                  .valid
		clock_clk              : in  std_logic                    := '0';             --             clock.clk
		reset_reset            : in  std_logic                    := '0';             --             reset.reset
		conduit_qspi_data      : in  std_logic_vector(3 downto 0) := (others => '0'); --      conduit_qspi.qspi_data
		conduit_qspi_clk       : in  std_logic                    := '0';             --                  .qspi_clk
		conduit_qspi_cs        : in  std_logic                    := '0';             --                  .qspi_cs
		conduit_ping_pong      : out std_logic;                                        -- conduit_ping_pong.new_signal
		conduit_debug_qspi_out : out std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_qspi.qspi_out
		conduit_debug_qspi_out_2 : out std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_qspi.qspi_out
		conduit_debug_qspi_in  : in  std_logic_vector(31 downto 0)  := (others => '0') --                         .qspi_in
	);
	end component;


	-- component qspi_interface2 is
	-- port (
	-- 	aso_out0_data          : out std_logic_vector(23 downto 0);                    --          aso_out0.data
	-- 	aso_out0_endofpacket   : out std_logic;                                       --                  .endofpacket
	-- 	aso_out0_ready         : in  std_logic                    := '0';             --                  .ready
	-- 	aso_out0_startofpacket : out std_logic;                                       --                  .startofpacket
	-- 	aso_out0_valid         : out std_logic;                                       --                  .valid
	-- 	clock_clk              : in  std_logic                    := '0';             --             clock.clk
	-- 	reset_reset            : in  std_logic                    := '0';             --             reset.reset
	-- 	conduit_qspi_data      : in  std_logic_vector(3 downto 0) := (others => '0'); --      conduit_qspi.qspi_data
	-- 	conduit_qspi_clk       : in  std_logic                    := '0';             --                  .qspi_clk
	-- 	conduit_qspi_cs        : in  std_logic                    := '0';             --                  .qspi_cs
	-- 	conduit_ping_pong      : out std_logic;                                        -- conduit_ping_pong.new_signal
	-- 	conduit_debug_qspi_out : out std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_qspi.qspi_out
	-- 	conduit_debug_qspi_out_2 : out std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_qspi.qspi_out
	-- 	conduit_debug_qspi_in  : in  std_logic_vector(31 downto 0)  := (others => '0') --                         .qspi_in
	-- );
	-- end component;

	component qspi_interface_verify is
	port (
		aso_out0_data          : in   std_ulogic_vector(23 downto 0);
		aso_out0_endofpacket   : in   std_ulogic;
		aso_out0_ready         : out  std_ulogic                   ;
		aso_out0_startofpacket : in   std_ulogic;
		aso_out0_valid         : in   std_ulogic;
		clock_clk              : out  std_ulogic                   ;
		reset_reset            : out  std_ulogic                   ;
		conduit_qspi_data      : out  std_ulogic_vector(3 downto 0);
		conduit_qspi_clk       : out  std_ulogic                   ;
		conduit_qspi_cs        : out  std_ulogic                   ;
		conduit_ping_pong      : in   std_ulogic
	);
	end component;

	signal s_aso_out0_data          : std_ulogic_vector(25 downto 0);
	signal s_aso_out0_endofpacket   : std_ulogic                   ;
	signal s_aso_out0_ready         : std_ulogic                   ;
	signal s_aso_out0_startofpacket : std_ulogic                   ;
	signal s_aso_out0_valid         : std_ulogic                   ;
	signal s_clock_clk              : std_ulogic                   ;
	signal s_reset_reset            : std_ulogic                   ;
	signal s_conduit_qspi_data      : std_ulogic_vector(3 downto 0);
	signal s_conduit_qspi_clk       : std_ulogic                   ;
	signal s_conduit_qspi_cs        : std_ulogic                   ;
	signal s_conduit_ping_pong      : std_ulogic                   ;
	signal s_conduit_debug_qspi_out : std_logic_vector(31 downto 0);
	signal s_conduit_debug_qspi_out_2 : std_logic_vector(31 downto 0);
    signal s_conduit_debug_qspi_in  : std_logic_vector(31 downto 0);

	signal s_aso_out0_data_temp     : std_ulogic_vector(23 downto 0);

begin
	-- some psl stuff
    default clock is rising_edge (s_clock_clk);

	verify : qspi_interface_verify
	port map (
		aso_out0_data          =>  s_aso_out0_data_temp          ,
		aso_out0_endofpacket   =>  s_aso_out0_endofpacket   ,
		aso_out0_ready         =>  s_aso_out0_ready         ,
		aso_out0_startofpacket =>  s_aso_out0_startofpacket ,
		aso_out0_valid         =>  s_aso_out0_valid         ,
		clock_clk              =>  s_clock_clk              ,
		reset_reset            =>  s_reset_reset            ,
		conduit_qspi_data      =>  s_conduit_qspi_data      ,
		conduit_qspi_clk       =>  s_conduit_qspi_clk       ,
		conduit_qspi_cs        =>  s_conduit_qspi_cs        ,
		conduit_ping_pong      =>  s_conduit_ping_pong
	);


	dut: qspi_interface
	port map (
		aso_out0_data          =>  s_aso_out0_data          ,
		--aso_out0_endofpacket   =>  s_aso_out0_endofpacket   ,
		aso_out0_ready         =>  s_aso_out0_ready         ,
		--aso_out0_startofpacket =>  s_aso_out0_startofpacket ,
		aso_out0_valid         =>  s_aso_out0_valid         ,
		clock_clk              =>  s_clock_clk              ,
		reset_reset            =>  s_reset_reset            ,
		conduit_qspi_data      =>  s_conduit_qspi_data      ,
		conduit_qspi_clk       =>  s_conduit_qspi_clk       ,
		conduit_qspi_cs        =>  s_conduit_qspi_cs        ,
		conduit_ping_pong      =>  s_conduit_ping_pong,
		conduit_debug_qspi_out => s_conduit_debug_qspi_out ,
		conduit_debug_qspi_out_2 => s_conduit_debug_qspi_out_2 ,
		conduit_debug_qspi_in  => s_conduit_debug_qspi_in
	);

	-- dut2: qspi_interface2
	-- port map (
	-- 	aso_out0_data          =>  s_aso_out0_data          ,
	-- 	aso_out0_endofpacket   =>  s_aso_out0_endofpacket   ,
	-- 	aso_out0_ready         =>  s_aso_out0_ready         ,
	-- 	aso_out0_startofpacket =>  s_aso_out0_startofpacket ,
	-- 	aso_out0_valid         =>  s_aso_out0_valid         ,
	-- 	clock_clk              =>  s_clock_clk              ,
	-- 	reset_reset            =>  s_reset_reset            ,
	-- 	conduit_qspi_data      =>  s_conduit_qspi_data      ,
	-- 	conduit_qspi_clk       =>  s_conduit_qspi_clk       ,
	-- 	conduit_qspi_cs        =>  s_conduit_qspi_cs        ,
	-- 	conduit_ping_pong      =>  s_conduit_ping_pong,
	-- 	conduit_debug_qspi_out => s_conduit_debug_qspi_out ,
	-- 	conduit_debug_qspi_out_2 => s_conduit_debug_qspi_out_2 ,
	-- 	conduit_debug_qspi_in  => s_conduit_debug_qspi_in
	-- );

	-- avalon stream checking
	sop_eop: assert always s_aso_out0_startofpacket -> eventually! s_aso_out0_endofpacket;
	assert always s_aso_out0_startofpacket -> s_aso_out0_valid;
	assert always s_aso_out0_valid -> s_aso_out0_ready;

	-- qspi checking
	assert always s_conduit_qspi_clk -> not s_conduit_qspi_cs;

	s_conduit_debug_qspi_in <= (others=>'0');


	s_aso_out0_data_temp <= s_aso_out0_data(23 downto 0);
	s_aso_out0_startofpacket <= s_aso_out0_data(24);
	s_aso_out0_endofpacket <= s_aso_out0_data(25);



end architecture rtl; -- of qspi_interface
