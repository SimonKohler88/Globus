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

library std;
use std.textio.all;


entity qspi_simulate is
	port (
        conduit_qspi_data      : out  std_ulogic_vector(3 downto 0);
        conduit_qspi_clk       : out  std_ulogic                   ;
        conduit_qspi_cs        : out  std_ulogic
	);
end entity qspi_simulate;

architecture rtl of qspi_simulate is

    procedure qspi_write_pixel(
        constant c_data    : in std_ulogic_vector(23 downto 0);
        signal clk         : in std_ulogic;
        signal qspi_clk    : out std_ulogic;
        signal data        : out std_ulogic_vector(3 downto 0)
    ) is
    begin
        qspi_clk <= '0';
        data <= (others => '0');

        wait for 2 ns;

        FOR i IN 5 downto 0 LOOP
            data <= c_data(i*4 + 3 downto i*4 );
            wait until rising_edge(clk);
            qspi_clk <= '1';
            wait until falling_edge(clk);
            qspi_clk <= '0';
        END LOOP;
    end procedure qspi_write_pixel;

    constant c_cycle_time_100M : time := 10 ns;
    constant c_cycle_time_qspi : time := 38 ns; --26M

    signal internal_qspi_clock: std_ulogic;
    signal enable :boolean:=true;

    file input_file : text open read_mode is "./Earth_relief_120x256_raw2.txt";
    constant c_pixel_to_send : integer := 4*20;


begin
	-- qspi_clk
	p_qspi_clk : process
	begin
		while enable loop
            internal_qspi_clock <= '0';
            wait for c_cycle_time_qspi/2;
            internal_qspi_clock<= '1';
            wait for c_cycle_time_qspi/2;
		end loop;
		wait;  -- don't do it again
	end process p_qspi_clk;


    p_stimuli: process
        variable v_write_data : std_ulogic_vector(23 downto 0);
        variable v_input_line : line;
    begin
        conduit_qspi_cs <= '1';
        conduit_qspi_clk <= '0';
        conduit_qspi_data <= (others => '0');


        wait for 50 ns;

        conduit_qspi_cs <= '0';
        wait for 2 ns;
        for i in 0 to c_pixel_to_send loop
            readline(input_file, v_input_line);
            hread(v_input_line, v_write_data);
            qspi_write_pixel(v_write_data, internal_qspi_clock, conduit_qspi_clk, conduit_qspi_data );
        end loop;
        conduit_qspi_cs <= '1';

        wait for 50 us;

        conduit_qspi_cs <= '0';
        wait for 2 ns;
        for i in 0 to c_pixel_to_send loop
            readline(input_file, v_input_line);
            hread(v_input_line, v_write_data);
            qspi_write_pixel(v_write_data, internal_qspi_clock, conduit_qspi_clk, conduit_qspi_data );
        end loop;
        conduit_qspi_cs <= '1';


        wait;
    end process p_stimuli;

    
















































end architecture rtl; -- of qspi_interface
