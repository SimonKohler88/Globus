

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library std;
use std.textio.all;

entity diverses_tb is
end entity diverses_tb;


--**********************************************************************************
--**********************************************************************************
--
--                  RAM WRITE TB
--
--**********************************************************************************
--**********************************************************************************

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
                in_D                   : in std_logic_vector(23 downto 0);
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


procedure avalon_stream_out_write_many_pixel(
        constant c_num     : in integer;
        constant c_time_intervall_ns : in time;
        signal clk         : in std_ulogic;
        signal data        : out std_ulogic_vector(23 downto 0);
        signal valid       : out std_ulogic;
        signal in_ready    : in std_ulogic;
        signal packet_start: out std_ulogic;
        signal packet_end  : out std_ulogic;
        constant delay_ready2valid_clocks : in integer

    ) is
        file input_file : text open read_mode is "./Earth_relief_120x256_raw2.txt";
        variable v_input_data : std_ulogic_vector(23 downto 0);
        variable v_input_line : line;
        variable pix_count : integer := 0;
    begin

        wait for 10 ns;
        for i in 0 to c_num-1 loop
            readline(input_file, v_input_line);
            hread(v_input_line, v_input_data);

            wait until rising_edge(clk);
            if in_ready = '0' then
                wait on in_ready;
            end if;


            if delay_ready2valid_clocks > 0 then
                for j in 0 to delay_ready2valid_clocks loop
                    wait until rising_edge(clk);
                end loop;
            end if;

            if i=0 then
                packet_start <= '1';
            end if;

            if i=c_num-1 then
                packet_end <= '1';
            end if;

            data <= v_input_data;
            valid <= '1';
            pix_count := pix_count +1;

            wait until rising_edge(clk);
            valid <= '0';
            packet_start <= '0';
            packet_end <= '0';



            wait for c_time_intervall_ns;
            wait until rising_edge(clk);
            --qspi_write_pixel(v_input_data, clk, qspi_clk, data );
        end loop;
        write(output, "Pixel Sent: " & to_string(pix_count) & lf);
    end procedure avalon_stream_out_write_many_pixel;

    signal s_clock:std_logic;
    signal s_reset:std_logic;

    signal s_in_A  :  std_logic;
    signal s_in_B  :  std_logic;
    signal s_in_C  :  std_logic;
    signal s_in_D  :  std_logic_vector(23 downto 0);
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
        in_D         => s_in_D ,
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

        avalon_stream_out_write_many_pixel(50, 200 ns, s_clock, s_in_D, s_in_A, s_out_1, s_in_C, s_in_B, 0 );
        wait for 1 us;

        enable <= false;
		wait;  -- don't do it again
	end process;

end architecture;
