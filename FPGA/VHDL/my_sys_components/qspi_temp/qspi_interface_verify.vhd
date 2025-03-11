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


entity qspi_interface_verify is
	port (
		aso_out0_data          : in   std_ulogic_vector(23 downto 0);
        aso_out0_endofpacket   : in   std_ulogic                   ;
        aso_out0_ready         : out  std_ulogic                   ;
        aso_out0_startofpacket : in   std_ulogic                   ;
        aso_out0_valid         : in   std_ulogic                   ;
        clock_clk              : out  std_ulogic                   ;
        reset_reset            : out  std_ulogic                   ;
        conduit_qspi_data      : out  std_ulogic_vector(3 downto 0);
        conduit_qspi_clk       : out  std_ulogic                   ;
        conduit_qspi_cs        : out  std_ulogic                   ;
        conduit_ping_pong      : in   std_ulogic
	);
end entity qspi_interface_verify;

architecture rtl of qspi_interface_verify is

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

    procedure qspi_write_many_pixel(
        constant c_num    : in integer;
        signal clk         : in std_ulogic;
        signal qspi_clk    : out std_ulogic;
        signal data        : out std_ulogic_vector(3 downto 0);
        signal cs        : out std_logic
    ) is
        file input_file : text open read_mode is "./Earth_relief_120x256_raw2.txt";
        variable v_input_data : std_ulogic_vector(23 downto 0);
        variable v_input_line : line;
    begin

       cs <= '0';
        wait for 10 ns;
        for i in 0 to c_num-1 loop
            readline(input_file, v_input_line);
            hread(v_input_line, v_input_data);
            qspi_write_pixel(v_input_data, clk, qspi_clk, data );
        end loop;
        cs <= '1';
    end procedure qspi_write_many_pixel;

    constant c_cycle_time_100M : time := 10 ns;
    constant c_cycle_time_qspi : time := 38 ns; --26M

    signal internal_qspi_clock: std_ulogic;

    signal enable         : boolean := true;

    file output_file : text open write_mode is "./stream_received.txt";
    constant c_pixel_to_send : integer := 256;



begin

    reset_reset <= transport '1', '0' after 5 ns;


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

	aso_out0_ready <= '1';



    p_stimuli: process

    begin
        conduit_qspi_cs <= '1';
        conduit_qspi_clk <= '0';
        conduit_qspi_data <= (others => '0');


        wait for 50 ns;

        for p in 0 to 4 loop
            qspi_write_many_pixel(256, internal_qspi_clock, conduit_qspi_clk, conduit_qspi_data, conduit_qspi_cs);

            wait for 5 us;
        end loop;


        wait;
    end process p_stimuli;

    p_store_stream: process(all) 
        variable v_output_line : line;
    begin

        if rising_edge(clock_clk) then
            if aso_out0_ready = '1' and aso_out0_valid ='1' then
                write(v_output_line, to_hstring(aso_out0_data));
                writeline(output_file, v_output_line);
            end if;
        end if;

    end process p_store_stream;

    p_monitor: process

    begin

		wait for 400 us;
		enable <= false;
		write(output, "all tested");
		wait;
    end process p_monitor;

















































end architecture rtl; -- of qspi_interface
