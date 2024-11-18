
library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library std;
use std.textio.all;


entity integration_verify_ram_qspi is
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

		conduit_col_info_col_nr  : out  std_logic_vector(8 downto 0)  := (others => '0'); --     conduit_col_info.col_nr
		conduit_col_info_fire    : out  std_logic                     := '0';             --                     .fire
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
		avs_s1_waitrequest       : in std_logic                                         --                     .waitrequest
	);
end entity integration_verify_ram_qspi;


architecture rtl of integration_verify_ram_qspi is

    component avalon_slave_ram_emulator is
	port (
		rst : in std_ulogic;
		clk : in std_ulogic;
        address       : in  std_ulogic_vector(24 downto 0);
        read          : in  std_ulogic;
        waitrequest   : out std_ulogic;
        readdata      : out std_ulogic_vector(15 downto 0);
        readdatavalid : out std_ulogic;
        write         : in  std_ulogic;
        writedata     : in  std_ulogic_vector(15 downto 0)
	);
    end component avalon_slave_ram_emulator;

    signal s_avm_m0_address           : std_logic_vector(24 downto 0);
	signal s_avm_m0_read              : std_ulogic;
	signal s_avm_m0_waitrequest       : std_ulogic;
	signal s_avm_m0_readdata          : std_logic_vector(15 downto 0);
	signal s_avm_m0_readdatavalid     : std_ulogic;
	signal s_avm_m0_write             : std_ulogic;
	signal s_avm_m0_writedata         : std_logic_vector(15 downto 0);

    constant c_cycle_time_100M : time := 10 ns;
    signal enable :boolean:=true;



begin

-- 100MHz
	p_system_clk : process
	begin
		while enable loop
            clock_clk <= '0';
            wait for c_cycle_time_100M/2;
            clock_clk <= '1';
            wait for c_cycle_time_100M/2;
		end loop;
		wait;  -- don't do it again
	end process p_system_clk;

    reset_reset <= transport '1', '0' after 5 ns;

    p_stimuli: process
    begin
        wait for 200 us;

        wait;
    end process p_stimuli;




    p_monitor: process

    begin

		wait for 400 us;
		enable <= false;
		write(output, "all tested");
		wait;
    end process p_monitor;



    helper_ram_emulator: avalon_slave_ram_emulator port map (
		rst           => clock_clk                ,
		clk           => reset_reset              ,
        address       => s_avm_m0_address           ,
        read          => s_avm_m0_read              ,
        waitrequest   => s_avm_m0_waitrequest       ,
        readdata      => s_avm_m0_readdata          ,
        readdatavalid => s_avm_m0_readdatavalid     ,
        write         => s_avm_m0_write             ,
        writedata     => s_avm_m0_writedata
	);



















































end architecture;
