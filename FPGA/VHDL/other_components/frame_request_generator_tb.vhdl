

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity frame_request_generator_tb is
end entity frame_request_generator_tb;

architecture rtl of frame_request_generator_tb is
    component frame_request_generator is
        generic (
            counter_bits  : integer := 22;
            on_clocks : integer := 2000
        );
        port (
            		clock_clk                   : in  std_logic                     := '0';             --                clock.clk
		reset_reset                 : in  std_logic                     := '0';
		enable_pin_input            : in  std_logic                     := '0';
		pin_input                   : in  std_logic                     := '0';
        pulse_out                   : out std_logic
        );
    end component;

    signal s_clock:std_logic;
    signal s_reset:std_logic;
    signal s_pulse:std_logic;
    signal s_pin_enable:std_logic;
    signal s_pin:std_logic;

    constant c_cycle_time_12M : time := 83 ns;
     signal enable :boolean:=true;

begin

    dut :frame_request_generator
    generic map (
            counter_bits  => 8,
            on_clocks => 20
        )
    port map(
        clock_clk => s_clock,
        reset_reset => s_reset,
        pulse_out => s_pulse,
        pin_input => s_pin,
        enable_pin_input => s_pin_enable
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
        s_pin_enable <= '0';
        s_pin <= '0';
        wait for 400 us;

        s_pin_enable <= '1';
        wait for 10 us;
        s_pin <= '1';
        wait for 10 us;
        s_pin <= '0';

        wait for 100 us;

        s_pin_enable <= '1';
        wait for 10 us;
        s_pin <= '1';
        wait for 10 us;
        s_pin <= '0';
        wait for 50 us;

        enable <= false;
		wait;  -- don't do it again
	end process;

end architecture;
