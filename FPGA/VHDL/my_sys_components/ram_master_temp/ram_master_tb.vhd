

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity ram_master_tb is
end entity ram_master_tb;

architecture rtl of ram_master_tb is

component ram_master is
	generic (
		G_IMAGE_ROWS             : integer := 120;
		G_IMAGE_COLS_BITS        : integer:= 8;
		G_BASE_ADDR_2_OFFSET     : unsigned  :=  X"040000"
	);
	port (
		clock_clk                : in  std_logic                     := '0';             --                clock.clk
		reset_reset              : in  std_logic                     := '0';             --                reset.reset
		conduit_ping_or_pong     : in  std_logic                     := '0';             -- conduit_ping_or_pong.new_signal
		avm_m0_address           : out std_logic_vector(23 downto 0);                    --               avm_m0.address
		avm_m0_read_n            : out std_logic;                                        --                     .read
		avm_m0_waitrequest       : in  std_logic                     := '0';             --                     .waitrequest
		avm_m0_readdata          : in  std_logic_vector(15 downto 0) := (others => '0'); --                     .readdata
		avm_m0_readdatavalid     : in  std_logic                     := '0';             --                     .readdatavalid
		avm_m0_write_n           : out std_logic;                                        --                     .write
		avm_m0_writedata         : out std_logic_vector(15 downto 0);                    --                     .writedata
		avm_m0_byteenable        : out std_logic_vector(1 downto 0);                     --                     .byteenable
		avm_m0_chipselect        : out std_logic;                                        --                     .chipselect
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
		avs_s1_waitrequest       : out std_logic;                                         --                     .waitrequest
		conduit_debug_ram_out    : out  std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_ram.ram_out
		conduit_debug_ram_out_2    : out  std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_ram.ram_out
		conduit_debug_ram_in     : in   std_logic_vector(31 downto 0)  := (others => '0') --                         .ram_in
	);
	end component;


 component ram_master_verify is
 generic (
		--image_cols : integer := 256;
		image_rows : integer := 120;
		image_cols_bits : integer := 8;
		ram_address_bits :integer := 10
	);
	port (
        clock_clk              : out  std_ulogic                   ;
        reset_reset            : out  std_ulogic                   ;

		aso_out0_data          : out   std_ulogic_vector(23 downto 0);
        aso_out0_endofpacket   : out   std_ulogic                   ;
        aso_out0_ready         : in  std_ulogic                   ;
        aso_out0_startofpacket : out   std_ulogic                   ;
        aso_out0_valid         : out   std_ulogic                   ;

        avm_m0_address           : in std_logic_vector(23 downto 0);                    --               avm_m0.address
		avm_m0_read              : in std_logic;                                        --                     .read
		avm_m0_waitrequest       : out  std_logic                     := '0';             --                     .waitrequest
		avm_m0_readdata          : out  std_logic_vector(15 downto 0) := (others => '0'); --                     .readdata
		avm_m0_readdatavalid     : out  std_logic                     := '0';             --                     .readdatavalid
		avm_m0_write             : in std_logic;                                        --                     .write
		avm_m0_writedata         : in std_logic_vector(15 downto 0);                    --                     .writedata


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
		conduit_intern_col_nr      : out std_logic_vector(8 downto 0);                     -- conduit_intern_col_info.col_nr
		conduit_intern_col_fire    : out std_logic                                         --                        .fire

	);
end component;

	signal s_clock_clk                : std_ulogic;
	signal s_reset_reset              : std_ulogic;
	signal s_conduit_ping_or_pong     : std_ulogic;
	signal s_avm_m0_address           : std_logic_vector(23 downto 0);
	signal s_avm_m0_read              : std_ulogic;
	signal s_avm_m0_waitrequest       : std_ulogic;
	signal s_avm_m0_readdata          : std_logic_vector(15 downto 0);
	signal s_avm_m0_readdatavalid     : std_ulogic;
	signal s_avm_m0_write             : std_ulogic;
	signal s_avm_m0_writedata         : std_logic_vector(15 downto 0);
	signal s_avm_m0_byteenable        : std_logic_vector(1 downto 0);                     --                     .byteenable
	signal s_avm_m0_chipselect        : std_logic;                                        --                     .chipselect
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
	signal s_conduit_debug_ram_out    : std_logic_vector(31 downto 0);
	signal s_conduit_debug_ram_out_2    : std_logic_vector(31 downto 0);
	signal s_conduit_debug_ram_in     : std_logic_vector(31 downto 0);

	signal s_avm_m0_readdatavalid_latency :std_logic_vector(10 downto 0);
	signal s_avm_m0_readdatavalid_late : std_logic;

begin

    dut_ram_master: ram_master
    generic map(
        G_IMAGE_ROWS             => 120,
		G_IMAGE_COLS_BITS        => 3,
		G_BASE_ADDR_2_OFFSET     => X"001000"
    )
    port map (
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
		avm_m0_byteenable        => s_avm_m0_byteenable        ,           --                     .byteenable
		avm_m0_chipselect        => s_avm_m0_chipselect        ,                                --                     .chipselect
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
		avs_s1_waitrequest       => s_avs_s1_waitrequest       ,
		conduit_debug_ram_out    => s_conduit_debug_ram_out    ,
		conduit_debug_ram_out_2    => s_conduit_debug_ram_out_2    ,
		conduit_debug_ram_in     => s_conduit_debug_ram_in
	);

    verify_ram_master: ram_master_verify
    generic map(
		image_rows => 120,
		image_cols_bits => 3,
		ram_address_bits => 13
    )
    port map(

        clock_clk                => s_clock_clk                ,
		reset_reset              => s_reset_reset              ,

        aso_out0_data            =>  s_asi_in0_data             ,      -- avs pixels from qspi to ram
        aso_out0_endofpacket     =>  s_asi_in0_endofpacket      ,
        aso_out0_ready           =>  s_asi_in0_ready            ,
        aso_out0_startofpacket   =>  s_asi_in0_startofpacket    ,
        aso_out0_valid           =>  s_asi_in0_valid            ,

        avm_m0_address            => s_avm_m0_address           ,     -- ram emulation
		avm_m0_read               => s_avm_m0_read              ,
		avm_m0_waitrequest        => s_avm_m0_waitrequest       ,
		avm_m0_readdata           => s_avm_m0_readdata          ,
		avm_m0_readdatavalid      => s_avm_m0_readdatavalid     ,
		avm_m0_write              => s_avm_m0_write             ,
		avm_m0_writedata          => s_avm_m0_writedata         ,


        asi_in1_data             =>  s_aso_out1_B_data          , -- Stream B to LED Interface
		asi_in1_ready            =>  s_aso_out1_B_ready         ,
		asi_in1_valid            =>  s_aso_out1_B_valid         ,
		asi_in1_startofpacket    =>  s_aso_out1_B_startofpacket ,
		asi_in1_endofpacket      =>  s_aso_out1_B_endofpacket   ,

		asi_in0_data             =>  s_aso_out0_A_data          , -- Stream A to LED Interface
		asi_in0_ready            =>  s_aso_out0_A_ready         ,
		asi_in0_valid            =>  s_aso_out0_A_valid         ,
		asi_in0_startofpacket    =>  s_aso_out0_startofpacket_1 ,
		asi_in0_endofpacket      =>  s_aso_out0_endofpacket_1   ,

		conduit_intern_col_nr    =>  s_conduit_col_info_col_nr  ,
		conduit_intern_col_fire  =>  s_conduit_col_info_fire


    );

end architecture rtl; -- of ram_master






































