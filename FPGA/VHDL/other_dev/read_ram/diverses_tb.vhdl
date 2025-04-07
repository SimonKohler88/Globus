

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library std;
use std.textio.all;

entity diverses_tb is
end entity diverses_tb;

architecture rtl of diverses_tb is
    component diverses is
        generic (
            g_1 : integer := 22;
            g_2 : integer := 2000;
            g_3 : integer := 1024-- default 1024 clock-cycles
        );
        port (
                clk                    : in  std_logic                     := '0';
                reset                  : in  std_logic                     := '0';
                in_A                   : in  std_logic                     := '0';
                in_B                   : in  std_logic                     := '0';
                in_C                   : in std_logic;
                out_1                  : out std_logic;
                out_2                  : out std_logic
        );
    end component;

    procedure pulse_out(signal out_signal: out std_ulogic;
                        signal clk : in std_ulogic;
                        constant c_pulse_len_clocks : in integer range 1 to 100
                        ) is
    begin
        wait until rising_edge(clk);
        out_signal <= '1';

        for i in 0 to c_pulse_len_clocks-1 loop
            wait until rising_edge(clk);
        end loop;
        out_signal <= '0';
    end procedure pulse_out;

    signal s_clock:std_logic;
    signal s_reset:std_logic;

    signal s_in_A  :  std_logic;
    signal s_in_B  :  std_logic;
    signal s_in_C  :  std_logic;
    signal s_out_1 :  std_logic;
    signal s_out_2 :  std_logic;

    -- constant c_cycle_time : time := 83 ns; -- 12M
    constant c_cycle_time: time := 10 ns; -- 10M
    signal enable :boolean:=true;

begin

    dut :diverses
    generic map (
            g_1 => 8,
            g_2 => 20,
            g_3 => 30
        )
    port map(
        clk          => s_clock,
        reset        => s_reset,
        in_A         => s_in_A ,
        in_B         => s_in_B ,
        in_C         => s_in_C ,
        out_1        => s_out_1,
        out_2        => s_out_2

    );

    s_reset <= transport '1', '0' after 50 ns;

    p_system_clk : process
	begin
		while enable loop
            s_clock <= '0';
            wait for c_cycle_time/2;
            s_clock <= '1';
            wait for c_cycle_time/2;
		end loop;
		wait;  -- don't do it again
	end process p_system_clk;

    p_stim : process
	begin
        s_in_A <= '0';
        s_in_B <= '0';
        s_in_C <= '0';

        wait for 100 ns;
        pulse_out(s_in_A, s_clock, 1);
        wait for 20 ns;
        pulse_out(s_in_C, s_clock, 1);
        for i in 0 to 10 loop
        wait for 100 ns;
        pulse_out(s_in_B, s_clock, 2);
        end loop;

        wait for 1 us;


        enable <= false;
		wait;  -- don't do it again
	end process;

end architecture;
