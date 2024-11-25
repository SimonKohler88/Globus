library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity integration_tb is
end entity integration_tb;

architecture rtl of integration_tb is


	component ram_master is
	generic (
		image_cols : integer := 256;
		image_rows : integer := 120
	);
	port (
		clock_clk                : in  std_logic                     := '0';             --                clock.clk
		reset_reset              : in  std_logic                     := '0';             --                reset.reset
		conduit_ping_or_pong     : in  std_logic                     := '0';             -- conduit_ping_or_pong.new_signal
		avm_m0_address           : out std_logic_vector(24 downto 0);                    --               avm_m0.address
		avm_m0_read_n            : out std_logic;                                        --                     .read
		avm_m0_waitrequest       : in  std_logic                     := '0';             --                     .waitrequest
		avm_m0_readdata          : in  std_logic_vector(15 downto 0) := (others => '0'); --                     .readdata
		avm_m0_readdatavalid     : in  std_logic                     := '0';             --                     .readdatavalid
		avm_m0_write_n           : out std_logic;                                        --                     .write
		avm_m0_writedata         : out std_logic_vector(15 downto 0);                    --                     .writedata
		asi_in0_data             : in  std_logic_vector(23 downto 0)  := (others => '0'); --              asi_in0.data
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
	end component;

    component qspi_interface is
	port (
		aso_out0_data          : out std_logic_vector(23 downto 0);                    --          aso_out0.data
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
    end component;

    component qspi_simulate is
	port (
        conduit_qspi_data      : out  std_ulogic_vector(3 downto 0);
        conduit_qspi_clk       : out  std_ulogic                   ;
        conduit_qspi_cs        : out  std_ulogic
	);
    end component;

    component integration_verify_ram_qspi is
	generic (
		image_cols : integer := 256;
		image_rows : integer := 120
	);
	port (
		clock_clk                : out  std_logic                     := '0';             --                clock.clk
		reset_reset              : out  std_logic                     := '0';             --                reset.reset
		avm_m0_address           : in std_logic_vector(24 downto 0);                    --               avm_m0.address
		avm_m0_read              : in std_logic;                                        --                     .read
		avm_m0_waitrequest       : out  std_logic                     := '0';             --                     .waitrequest
		avm_m0_readdata          : out  std_logic_vector(15 downto 0) := (others => '0'); --                     .readdata
		avm_m0_readdatavalid     : out  std_logic                     := '0';             --                     .readdatavalid
		avm_m0_write             : in std_logic;                                        --                     .write
		avm_m0_writedata         : in std_logic_vector(15 downto 0);                    --                     .writedata

		aso_out1_B_data          : in std_logic_vector(23 downto 0);                    --           aso_out1_B.data
		aso_out1_B_endofpacket   : in std_logic;                                        --                     .endofpacket
		aso_out1_B_ready         : out  std_logic                     := '0';             --                     .ready
		aso_out1_B_startofpacket : in std_logic;                                        --                     .startofpacket
		aso_out1_B_valid         : in std_logic;                                        --                     .valid
		aso_out0_startofpacket_1 : in std_logic;                                        --           aso_out0_A.startofpacket
		aso_out0_endofpacket_1   : in std_logic;                                        --                     .endofpacket
		aso_out0_A_data          : in std_logic_vector(23 downto 0);                    --                     .data
		aso_out0_A_ready         : out  std_logic                     := '0';             --                     .ready
		aso_out0_A_valid         : in std_logic;                                        --                     .valid
		avs_s1_address           : out  std_logic_vector(7 downto 0)  := (others => '0'); --               avs_s1.address
		avs_s1_read              : out  std_logic                     := '0';             --                     .read
		avs_s1_readdata          : in std_logic_vector(31 downto 0);                    --                     .readdata
		avs_s1_write             : out  std_logic                     := '0';             --                     .write
		avs_s1_writedata         : out  std_logic_vector(31 downto 0) := (others => '0'); --                     .writedata
		avs_s1_waitrequest       : in std_logic ;                                        --                     .waitrequest

		conduit_encoder_A          : out  std_logic                     := '0';             --         conduit_encoder.enc_a
		conduit_encoder_B          : out  std_logic                     := '0';             --                        .enc_b
		conduit_encoder_index      : out  std_logic                     := '0';             --                        .enc_index
		conduit_encoder_sim_switch : out  std_logic                     := '0';             --                        .sim_switch
		conduit_encoder_sim_pulse  : out  std_logic                     := '0';            --                        .sim_pulse

		col_fire : in std_ulogic
	);
    end component integration_verify_ram_qspi;

    component new_encoder is
	generic (
		input_pulse_per_rev : integer := 512;
		output_pulse_div    : integer := 2
	);
	port (
		avs_s0_address             : in  std_logic_vector(7 downto 0)  := (others => '0'); --                  avs_s0.address
		avs_s0_read                : in  std_logic                     := '0';             --                        .read
		avs_s0_readdata            : out std_logic_vector(31 downto 0);                    --                        .readdata
		avs_s0_write               : in  std_logic                     := '0';             --                        .write
		avs_s0_writedata           : in  std_logic_vector(31 downto 0) := (others => '0'); --                        .writedata
		avs_s0_waitrequest         : out std_logic;                                        --                        .waitrequest
		clock_clk                  : in  std_logic                     := '0';             --                   clock.clk
		reset_reset                : in  std_logic                     := '0';             --                   reset.reset
		conduit_encoder_A          : in  std_logic                     := '0';             --         conduit_encoder.enc_a
		conduit_encoder_B          : in  std_logic                     := '0';             --                        .enc_b
		conduit_encoder_index      : in  std_logic                     := '0';             --                        .enc_index
		conduit_encoder_sim_switch : in  std_logic                     := '0';             --                        .sim_switch
		conduit_encoder_sim_pulse  : in  std_logic                     := '0';             --                        .sim_pulse
		conduit_intern_col_nr      : out std_logic_vector(8 downto 0);                     -- conduit_intern_col_info.col_nr
		conduit_intern_col_fire    : out std_logic                                         --                        .fire
	);
	end component;
--

	signal s_clock_clk                : std_ulogic;
	signal s_reset_reset              : std_ulogic;
	signal s_conduit_ping_or_pong     : std_ulogic;
	signal s_avm_m0_address           : std_logic_vector(24 downto 0);
	signal s_avm_m0_read              : std_ulogic;
	signal s_avm_m0_waitrequest       : std_ulogic;
	signal s_avm_m0_readdata          : std_logic_vector(15 downto 0);
	signal s_avm_m0_readdatavalid     : std_ulogic;
	signal s_avm_m0_write             : std_ulogic;
	signal s_avm_m0_writedata         : std_logic_vector(15 downto 0);
	signal s_asi_in0_data             : std_logic_vector(23 downto 0);
	signal s_asi_in0_ready            : std_ulogic;
	signal s_asi_in0_valid            : std_ulogic;
	signal s_asi_in0_endofpacket      : std_ulogic;
	signal s_asi_in0_startofpacket    : std_ulogic;
	signal s_conduit_col_info_col_nr  : std_logic_vector(8 downto 0);
	signal s_conduit_col_info_fire    : std_ulogic;
	signal s_aso_out1_B_data          : std_logic_vector(23 downto 0);
	signal s_aso_out1_B_endofpacket   : std_ulogic;
	signal s_aso_out1_B_ready         : std_ulogic;
	signal s_aso_out1_B_startofpacket : std_ulogic;
	signal s_aso_out1_B_valid         : std_ulogic;
	signal s_aso_out0_startofpacket_1 : std_ulogic;
	signal s_aso_out0_endofpacket_1   : std_ulogic;
	signal s_aso_out0_A_data          : std_logic_vector(23 downto 0);
	signal s_aso_out0_A_ready         : std_ulogic;
	signal s_aso_out0_A_valid         : std_ulogic;
	signal s_avs_s1_address           : std_logic_vector(7 downto 0);
	signal s_avs_s1_read              : std_ulogic;
	signal s_avs_s1_readdata          : std_logic_vector(31 downto 0);
	signal s_avs_s1_write             : std_ulogic;
	signal s_avs_s1_writedata         : std_logic_vector(31 downto 0);
	signal s_avs_s1_waitrequest       : std_ulogic;

	signal s_conduit_encoder_A            : std_ulogic;
	signal s_conduit_encoder_B            : std_ulogic;
	signal s_conduit_encoder_index        : std_ulogic;
	signal s_conduit_encoder_sim_switch   : std_ulogic;
	signal s_conduit_encoder_sim_pulse    : std_ulogic;


	signal s_conduit_qspi_data      : std_ulogic_vector(3 downto 0);
    signal s_conduit_qspi_clk       : std_ulogic                   ;
    signal s_conduit_qspi_cs        : std_ulogic                   ;

begin

    verify_ram_qspi: integration_verify_ram_qspi port map (
		clock_clk                => s_clock_clk               ,
		reset_reset              => s_reset_reset             ,
		avm_m0_address           => s_avm_m0_address          ,
		avm_m0_read              => s_avm_m0_read             ,
		avm_m0_waitrequest       => s_avm_m0_waitrequest      ,
		avm_m0_readdata          => s_avm_m0_readdata         ,
		avm_m0_readdatavalid     => s_avm_m0_readdatavalid    ,
		avm_m0_write             => s_avm_m0_write            ,
		avm_m0_writedata         => s_avm_m0_writedata        ,

		aso_out1_B_data          => s_aso_out1_B_data         ,
		aso_out1_B_endofpacket   => s_aso_out1_B_endofpacket  ,
		aso_out1_B_ready         => s_aso_out1_B_ready        ,
		aso_out1_B_startofpacket => s_aso_out1_B_startofpacket,
		aso_out1_B_valid         => s_aso_out1_B_valid        ,

		aso_out0_startofpacket_1 => s_aso_out0_startofpacket_1,
		aso_out0_endofpacket_1   => s_aso_out0_endofpacket_1  ,
		aso_out0_A_data          => s_aso_out0_A_data         ,
		aso_out0_A_ready         => s_aso_out0_A_ready        ,
		aso_out0_A_valid         => s_aso_out0_A_valid        ,

		avs_s1_address           => s_avs_s1_address          ,
		avs_s1_read              => s_avs_s1_read             ,
		avs_s1_readdata          => s_avs_s1_readdata         ,
		avs_s1_write             => s_avs_s1_write            ,
		avs_s1_writedata         => s_avs_s1_writedata        ,
		avs_s1_waitrequest       => s_avs_s1_waitrequest,

		conduit_encoder_A             =>  s_conduit_encoder_A             ,
		conduit_encoder_B             =>  s_conduit_encoder_B             ,
		conduit_encoder_index         =>  s_conduit_encoder_index         ,
		conduit_encoder_sim_switch    =>  s_conduit_encoder_sim_switch    ,
		conduit_encoder_sim_pulse     =>  s_conduit_encoder_sim_pulse,
		col_fire  => s_conduit_col_info_fire
	);

    sim_qspi: qspi_simulate port map (
        conduit_qspi_data     => s_conduit_qspi_data  ,
        conduit_qspi_clk      => s_conduit_qspi_clk   ,
        conduit_qspi_cs       => s_conduit_qspi_cs
    );

    dut_qspi: qspi_interface  port map (
        aso_out0_data          => s_asi_in0_data              ,
        aso_out0_endofpacket   => s_asi_in0_endofpacket       ,
        aso_out0_ready         => s_asi_in0_ready             ,
        aso_out0_startofpacket => s_asi_in0_startofpacket     ,
        aso_out0_valid         => s_asi_in0_valid             ,
        clock_clk              => s_clock_clk                ,
        reset_reset            => s_reset_reset              ,
        conduit_qspi_data      => s_conduit_qspi_data  ,
        conduit_qspi_clk       => s_conduit_qspi_clk   ,
        conduit_qspi_cs        => s_conduit_qspi_cs    ,
        conduit_ping_pong      => s_conduit_ping_or_pong
    );


	dut_encoder:  new_encoder
	port map(
		avs_s0_address             => open        ,
		avs_s0_read                => open        ,
		avs_s0_readdata            => open        ,
		avs_s0_write               => open        ,
		avs_s0_writedata           => open        ,
		avs_s0_waitrequest         => open,
		clock_clk                  => s_clock_clk                           ,
		reset_reset                => s_reset_reset                         ,
		conduit_encoder_A          => s_conduit_encoder_A                          ,
		conduit_encoder_B          => s_conduit_encoder_B                         ,
		conduit_encoder_index      => s_conduit_encoder_index       ,
		conduit_encoder_sim_switch => s_conduit_encoder_sim_switch  ,
		conduit_encoder_sim_pulse  => s_conduit_encoder_sim_pulse   ,
		conduit_intern_col_nr      => s_conduit_col_info_col_nr       ,
		conduit_intern_col_fire    => s_conduit_col_info_fire
	);--s_avs_s1_waitrequest

    dut_ram_master: ram_master port map (
		clock_clk                => s_clock_clk                ,
		reset_reset              => s_reset_reset              ,
		conduit_ping_or_pong     => s_conduit_ping_or_pong     ,
		avm_m0_address           => s_avm_m0_address           ,
		avm_m0_read_n            => s_avm_m0_read              ,
		avm_m0_waitrequest       => s_avm_m0_waitrequest       ,
		avm_m0_readdata          => s_avm_m0_readdata          ,
		avm_m0_readdatavalid     => s_avm_m0_readdatavalid     ,
		avm_m0_write_n           => s_avm_m0_write             ,
		avm_m0_writedata         => s_avm_m0_writedata         ,
		asi_in0_data             => s_asi_in0_data             ,
		asi_in0_ready            => s_asi_in0_ready            ,
		asi_in0_valid            => s_asi_in0_valid            ,
		asi_in0_endofpacket      => s_asi_in0_endofpacket      ,
		asi_in0_startofpacket    => s_asi_in0_startofpacket    ,
		conduit_col_info_col_nr  => s_conduit_col_info_col_nr  ,
		conduit_col_info_fire    => s_conduit_col_info_fire    ,
		aso_out1_B_data          => s_aso_out1_B_data          ,
		aso_out1_B_endofpacket   => s_aso_out1_B_endofpacket   ,
		aso_out1_B_ready         => s_aso_out1_B_ready         ,
		aso_out1_B_startofpacket => s_aso_out1_B_startofpacket ,
		aso_out1_B_valid         => s_aso_out1_B_valid         ,
		aso_out0_startofpacket_1 => s_aso_out0_startofpacket_1 ,
		aso_out0_endofpacket_1   => s_aso_out0_endofpacket_1   ,
		aso_out0_A_data          => s_aso_out0_A_data          ,
		aso_out0_A_ready         => s_aso_out0_A_ready         ,
		aso_out0_A_valid         => s_aso_out0_A_valid         ,
		avs_s1_address           => s_avs_s1_address           ,
		avs_s1_read              => s_avs_s1_read              ,
		avs_s1_readdata          => s_avs_s1_readdata          ,
		avs_s1_write             => s_avs_s1_write             ,
		avs_s1_writedata         => s_avs_s1_writedata         ,
		avs_s1_waitrequest       => s_avs_s1_waitrequest
	);









end architecture rtl; -- of ram_master
