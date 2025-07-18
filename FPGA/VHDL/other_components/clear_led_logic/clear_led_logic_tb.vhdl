

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library std;
use std.textio.all;

entity clear_led_logic_tb is
end entity clear_led_logic_tb;

architecture rtl of clear_led_logic_tb is
    component clear_led_logic is
        generic (
		G_COUNTER_BITS : integer := 17;
		G_TIMEOUT_COUNTER_BITS: integer := 15
	);
	port (
		clock_clk                   : in  std_logic                     := '0';             --                clock.clk
		reset_reset                 : in  std_logic                     := '0';
		qspi_cs                     : in std_logic := '0';
		clear_led_pulse             : out std_logic;
		clear_led_prohibit_fire     : out std_logic
        );
    end component;

    signal s_clock                     : std_logic;
    signal s_reset                     : std_logic;
    signal s_qspi_cs                   : std_logic;
    signal s_clear_led_pulse           : std_logic;
    signal s_clear_led_prohibit_fire   : std_logic;

    constant c_cycle_time_12M : time := 83 ns;
    signal enable :boolean:=true;

begin

    dut :clear_led_logic
    generic map (
            G_COUNTER_BITS  => 8,
            G_TIMEOUT_COUNTER_BITS  => 6
        )
    port map(
        clock_clk                  => s_clock,
        reset_reset                => s_reset,
        qspi_cs                    => s_qspi_cs,
        clear_led_pulse            => s_clear_led_pulse,
        clear_led_prohibit_fire    => s_clear_led_prohibit_fire
    );

    s_reset <= transport '0', '1' after 50 ns;

    p_system_clk : process
	begin
		while enable loop
            s_clock <= '0';
            wait for c_cycle_time_12M/2;
            s_clock <= '1';
            wait for c_cycle_time_12M/2;
		end loop;
		wait;  -- don't do it again
	end process p_system_clk;

    p_stim : process
	begin
        s_qspi_cs <= '0';
        
        wait for 100 us;
        s_qspi_cs <= '1';
        wait for 100 us;

        s_qspi_cs <= '0';
        wait for 50 us;
        s_qspi_cs <= '1';
        wait for 500 ns;
        s_qspi_cs <= '0';
        wait for 1 us;
        s_qspi_cs <= '1';
        wait for 20 us;

        s_qspi_cs <= '0';
        wait for 50 us;
        s_qspi_cs <= '1';
        wait for 100 us;

        s_qspi_cs <= '0';
        wait for 50 us;
        s_qspi_cs <= '1';
        wait for 100 us;

        s_qspi_cs <= '0';
        wait for 50 us;
        s_qspi_cs <= '1';
        wait for 100 us;

        s_qspi_cs <= '0';
        wait for 50 us;
        s_qspi_cs <= '1';
        wait for 100 us;

        s_qspi_cs <= '0';
        wait for 50 us;
        s_qspi_cs <= '1';
        wait for 100 us;

        s_qspi_cs <= '0';
        wait for 50 us;
        s_qspi_cs <= '1';
        wait for 100 us;

        s_qspi_cs <= '0';
        wait for 50 us;
        s_qspi_cs <= '1';
        wait for 100 us;

        enable <= false;
		wait;  -- don't do it again
	end process;

end architecture;
