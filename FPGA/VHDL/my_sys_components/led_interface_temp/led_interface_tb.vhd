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

entity led_interface_tb is
end entity led_interface_tb;

architecture rtl of led_interface_tb is

component led_interface is
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
		asi_in0_endofpacket         : in  std_logic                     := '0';              --                     .endofpacket
		conduit_debug_led_led_dbg_out : out  std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_led.led_dbg_out
		conduit_debug_led_led_dbg_out_2 : out  std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_led.led_dbg_out
		conduit_debug_led_led_dbg_in  : in   std_logic_vector(31 downto 0)  := (others => '0') --                         .led_dbg_in
	);
end component;

component led_interface_verify is
	generic (
		num_led_A  : integer := 30;
		num_led_B  : integer := 30;
		num_led_C  : integer := 30;
		num_led_D  : integer := 30;
		image_rows : integer := 120
	);
	port (
		clock_clk                     : out  std_logic                     := '0';             --                clock.clk
		reset_reset                   : out  std_logic                     := '0';             --                reset.reset
		conduit_LED_A_CLK             : in   std_logic;                                        --        conduit_LED_A.led_a_clk
		conduit_LED_A_DATA            : in   std_logic;                                        --                     .led_a_data
		conduit_LED_B_CLK             : in   std_logic;                                        --        conduit_LED_B.led_b_clk
		conduit_LED_B_DATA            : in   std_logic;                                        --                     .led_b_data
		conduit_LED_C_CLK             : in   std_logic;                                        --        conduit_LED_C.led_c_clk
		conduit_LED_C_DATA            : in   std_logic;                                        --                     .led_c_data
		conduit_LED_D_CLK             : in   std_logic;                                        --        conduit_LED_D.led_d_clk
		conduit_LED_D_DATA            : in   std_logic;                                        --                     .new_signal_1
		clock_led_spi_clk             : out  std_logic                     := '0';             --        clock_led_spi.clk
		conduit_col_info              : out  std_logic_vector(8 downto 0)  := (others => '0'); --     conduit_col_info.col_nr
		conduit_fire                  : out  std_logic                     := '0';             --                     .fire
		conduit_col_info_out_col_nr   : in   std_logic_vector(8 downto 0);                     -- conduit_col_info_out.col_nr
		conduit_col_info_out_fire     : in   std_logic;                                        --                     .fire
		avs_s0_address                : out  std_logic_vector(7 downto 0)  := (others => '0'); --               avs_s0.address
		avs_s0_read                   : out  std_logic                     := '0';             --                     .read
		avs_s0_readdata               : in   std_logic_vector(31 downto 0);                    --                     .readdata
		avs_s0_write                  : out  std_logic                     := '0';             --                     .write
		avs_s0_writedata              : out  std_logic_vector(31 downto 0) := (others => '0'); --                     .writedata
		avs_s0_waitrequest            : in   std_logic;                                        --                     .waitrequest
		asi_in1_data                  : out  std_logic_vector(23 downto 0) := (others => '0'); --            asi_in1_B.data
		asi_in1_ready                 : in   std_logic;                                        --                     .ready
		asi_in1_valid                 : out  std_logic                     := '0';             --                     .valid
		asi_in1_startofpacket         : out  std_logic                     := '0';             --                     .startofpacket
		asi_in1_endofpacket           : out  std_logic                     := '0';             --                     .endofpacket
		asi_in0_data                  : out  std_logic_vector(23 downto 0) := (others => '0'); --            asi_in0_A.data
		asi_in0_ready                 : in   std_logic;                                        --                     .ready
		asi_in0_valid                 : out  std_logic                     := '0';             --                     .valid
		asi_in0_startofpacket         : out  std_logic                     := '0';             --                     .startofpacket
		asi_in0_endofpacket           : out  std_logic                     := '0';              --                     .endofpacket
		conduit_debug_led_led_dbg_in  : out  std_logic_vector(31 downto 0)
	);
end component;

	signal s_clock_clk                    : std_ulogic                     ;
    signal s_reset_reset                  : std_ulogic                     ;
    signal s_conduit_LED_A_CLK            : std_ulogic                     ;
    signal s_conduit_LED_A_DATA           : std_ulogic                     ;
    signal s_conduit_LED_B_CLK            : std_ulogic                     ;
    signal s_conduit_LED_B_DATA           : std_ulogic                     ;
    signal s_conduit_LED_C_CLK            : std_ulogic                     ;
    signal s_conduit_LED_C_DATA           : std_ulogic                     ;
    signal s_conduit_LED_D_CLK            : std_ulogic                     ;
    signal s_conduit_LED_D_DATA           : std_ulogic                     ;
    signal s_clock_led_spi_clk            : std_ulogic                     ;
    signal s_conduit_col_info             : std_logic_vector(8 downto 0)   ;
    signal s_conduit_fire                 : std_ulogic                     ;
    signal s_conduit_col_info_out_col_nr  : std_logic_vector(8 downto 0)   ;
    signal s_conduit_col_info_out_fire    : std_ulogic                     ;
    signal s_avs_s0_address               : std_logic_vector(7 downto 0)   ;
    signal s_avs_s0_read                  : std_ulogic                     ;
    signal s_avs_s0_readdata              : std_logic_vector(31 downto 0)  ;
    signal s_avs_s0_write                 : std_ulogic                     ;
    signal s_avs_s0_writedata             : std_logic_vector(31 downto 0)  ;
    signal s_avs_s0_waitrequest           : std_ulogic                     ;
    signal s_asi_in1_data                 : std_logic_vector(23 downto 0)  ;
    signal s_asi_in1_ready                : std_ulogic                     ;
    signal s_asi_in1_valid                : std_ulogic                     ;
    signal s_asi_in1_startofpacket        : std_ulogic                     ;
    signal s_asi_in1_endofpacket          : std_ulogic                     ;
    signal s_asi_in0_data                 : std_logic_vector(23 downto 0)  ;
    signal s_asi_in0_ready                : std_ulogic                     ;
    signal s_asi_in0_valid                : std_ulogic                     ;
    signal s_asi_in0_startofpacket        : std_ulogic                     ;
    signal s_asi_in0_endofpacket          : std_ulogic                     ;
	signal s_conduit_debug_led_out : std_logic_vector(31 downto 0);
	signal s_conduit_debug_led_out_2 : std_logic_vector(31 downto 0);
	signal s_conduit_debug_led_in  : std_logic_vector(31 downto 0);

begin

   default clock is rising_edge (s_clock_clk);

	verify_led_if: led_interface_verify port map (
        clock_clk                      => s_clock_clk                   ,
        reset_reset                    => s_reset_reset                 ,
        conduit_LED_A_CLK              => s_conduit_LED_A_CLK           ,
        conduit_LED_A_DATA             => s_conduit_LED_A_DATA          ,
        conduit_LED_B_CLK              => s_conduit_LED_B_CLK           ,
        conduit_LED_B_DATA             => s_conduit_LED_B_DATA          ,
        conduit_LED_C_CLK              => s_conduit_LED_C_CLK           ,
        conduit_LED_C_DATA             => s_conduit_LED_C_DATA          ,
        conduit_LED_D_CLK              => s_conduit_LED_D_CLK           ,
        conduit_LED_D_DATA             => s_conduit_LED_D_DATA          ,
        clock_led_spi_clk              => s_clock_led_spi_clk           ,
        conduit_col_info               => s_conduit_col_info            ,
        conduit_fire                   => s_conduit_fire                ,
        conduit_col_info_out_col_nr    => s_conduit_col_info_out_col_nr ,
        conduit_col_info_out_fire      => s_conduit_col_info_out_fire   ,
        avs_s0_address                 => s_avs_s0_address              ,
        avs_s0_read                    => s_avs_s0_read                 ,
        avs_s0_readdata                => s_avs_s0_readdata             ,
        avs_s0_write                   => s_avs_s0_write                ,
        avs_s0_writedata               => s_avs_s0_writedata            ,
	    avs_s0_waitrequest             => s_avs_s0_waitrequest          ,
        asi_in1_data                   => s_asi_in1_data                ,
        asi_in1_ready                  => s_asi_in1_ready               ,
        asi_in1_valid                  => s_asi_in1_valid               ,
        asi_in1_startofpacket          => s_asi_in1_startofpacket       ,
        asi_in1_endofpacket            => s_asi_in1_endofpacket         ,
        asi_in0_data                   => s_asi_in0_data                ,
        asi_in0_ready                  => s_asi_in0_ready               ,
        asi_in0_valid                  => s_asi_in0_valid               ,
        asi_in0_startofpacket          => s_asi_in0_startofpacket       ,
        asi_in0_endofpacket            => s_asi_in0_endofpacket         ,
        conduit_debug_led_led_dbg_in   => s_conduit_debug_led_in
     );

     dut : led_interface port map(
        clock_clk                        => s_clock_clk                   ,
        reset_reset                      => s_reset_reset                 ,
        conduit_LED_A_CLK                => s_conduit_LED_A_CLK           ,
        conduit_LED_A_DATA               => s_conduit_LED_A_DATA          ,
        conduit_LED_B_CLK                => s_conduit_LED_B_CLK           ,
        conduit_LED_B_DATA               => s_conduit_LED_B_DATA          ,
        conduit_LED_C_CLK                => s_conduit_LED_C_CLK           ,
        conduit_LED_C_DATA               => s_conduit_LED_C_DATA          ,
        conduit_LED_D_CLK                => s_conduit_LED_D_CLK           ,
        conduit_LED_D_DATA               => s_conduit_LED_D_DATA          ,
        clock_led_spi_clk                => s_clock_led_spi_clk           ,
        conduit_col_info                 => s_conduit_col_info            ,
        conduit_fire                     => s_conduit_fire                ,
        conduit_col_info_out_col_nr      => s_conduit_col_info_out_col_nr ,
        conduit_col_info_out_fire        => s_conduit_col_info_out_fire   ,
        avs_s0_address                   => s_avs_s0_address              ,
        avs_s0_read                      => s_avs_s0_read                 ,
        avs_s0_readdata                  => s_avs_s0_readdata             ,
        avs_s0_write                     => s_avs_s0_write                ,
        avs_s0_writedata                 => s_avs_s0_writedata            ,
	    avs_s0_waitrequest               => s_avs_s0_waitrequest          ,
        asi_in1_data                     => s_asi_in1_data                ,
        asi_in1_ready                    => s_asi_in1_ready               ,
        asi_in1_valid                    => s_asi_in1_valid               ,
        asi_in1_startofpacket            => s_asi_in1_startofpacket       ,
        asi_in1_endofpacket              => s_asi_in1_endofpacket         ,
        asi_in0_data                     => s_asi_in0_data                ,
        asi_in0_ready                    => s_asi_in0_ready               ,
        asi_in0_valid                    => s_asi_in0_valid               ,
        asi_in0_startofpacket            => s_asi_in0_startofpacket       ,
        asi_in0_endofpacket              => s_asi_in0_endofpacket         ,
        conduit_debug_led_led_dbg_out    => s_conduit_debug_led_out       ,
        conduit_debug_led_led_dbg_out_2  => s_conduit_debug_led_out_2     ,
        conduit_debug_led_led_dbg_in     => s_conduit_debug_led_in
     );

     -- avalon stream checking
	in0_sop_eop:     assert always s_asi_in0_startofpacket -> eventually! s_asi_in0_endofpacket;
	in0_sop_valid:   assert always s_asi_in0_startofpacket -> s_asi_in0_valid;
	in0_eop_valid:   assert always s_asi_in0_startofpacket -> s_asi_in0_valid;
	in0_ready_valid: assert always s_asi_in0_valid -> s_asi_in0_ready;

    in1_sop_eop:     assert always s_asi_in1_startofpacket -> eventually! s_asi_in1_endofpacket;
	in1_sop_valid:   assert always s_asi_in1_startofpacket -> s_asi_in1_valid;
	in1_eop_valid:   assert always s_asi_in1_startofpacket -> s_asi_in1_valid;
	in1_ready_valid: assert always s_asi_in1_valid -> s_asi_in1_ready;























end architecture rtl; -- of led_interface
