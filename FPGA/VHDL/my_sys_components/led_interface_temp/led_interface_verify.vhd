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

library std;
use std.textio.all;


entity led_interface_verify is
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
end entity led_interface_verify;

architecture rtl of led_interface_verify is


procedure avalon_stream_out_write_many_pixel(
        constant c_num     : in integer;
        constant c_time_intervall_ns : in integer range 5 to 100;
        signal clk         : in std_ulogic;
        signal data        : out std_ulogic_vector(23 downto 0);
        signal valid       : out std_ulogic;
        signal in_ready    : in std_ulogic;
        signal packet_start: out std_ulogic;
        signal packet_end  : out std_ulogic;
        constant delay_ready2valid_clocks : in integer;
        signal which_file :in std_ulogic;
        constant c_start_line: in integer

    ) is
        file input_file : text open read_mode is "./row_col_num.txt";
        -- file input_file_2 : text open read_mode is "./Earth_relief_120x256_raw2_inverted.txt";
        variable v_input_data : std_ulogic_vector(23 downto 0);
        variable v_input_line : line;
        variable pix_count : integer := 0;
    begin

		for i in 0 to c_start_line-1 loop
			readline(input_file, v_input_line);
		end loop;

        wait for 10 ns;
        for i in 0 to c_num-1 loop

			readline(input_file, v_input_line);

            hread(v_input_line, v_input_data);

            if in_ready = '0' then
                wait until rising_edge(in_ready);
            end if;

            if delay_ready2valid_clocks > 0 then
                for j in 0 to delay_ready2valid_clocks loop
                    wait until rising_edge(clock_clk);
                end loop;
			else
				wait until rising_edge(clock_clk);
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

            wait for 10 ns;
            --qspi_write_pixel(v_input_data, clk, qspi_clk, data );
        end loop;
        write(output, "Pixel Sent: " & to_string(pix_count) & lf);
    end procedure avalon_stream_out_write_many_pixel;

    procedure avs_stream_pixel(
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
    end procedure avs_stream_pixel;

	procedure pulse_out(signal out_signal: out std_ulogic; signal clk : in std_ulogic) is
    begin
        wait until rising_edge(clk);
        out_signal <= '1';
        wait until rising_edge(clk);
        out_signal <= '0';
    end procedure pulse_out;

	constant c_cycle_time_100M : time := 10 ns;
	constant c_cycle_time_26M : time := 33 ns;

	constant c_cycle_real_firepulse: time := 117 us; --1.1764705e-4 = 0.117... ms = 8.5 kHz

    signal enable :boolean:=true;


    file output_file_stream_A : text open write_mode is "./stream_out/stream_received_A.txt";
	file output_file_stream_B : text open write_mode is "./stream_out/stream_received_B.txt";
	file output_file_stream_C : text open write_mode is "./stream_out/stream_received_C.txt";
	file output_file_stream_D : text open write_mode is "./stream_out/stream_received_D.txt";

	signal rec_A: std_ulogic_vector(7 downto 0);
	signal rec_B: std_ulogic_vector(7 downto 0);
	signal rec_C: std_ulogic_vector(7 downto 0);
	signal rec_D: std_ulogic_vector(7 downto 0);

	signal bitcount: integer := 0;

	signal input_file : std_ulogic;

	signal test_case : integer;

	signal prohibit_hot_fire :std_logic;
	signal clear_led_pulse   :std_logic;


begin


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

	p_spi_clk : process
	begin
		while enable loop
            clock_led_spi_clk <= '0';
            wait for c_cycle_time_26M/2;
            clock_led_spi_clk <= '1';
            wait for c_cycle_time_26M/2;
		end loop;
		wait;  -- don't do it again
	end process p_spi_clk;

    reset_reset <= transport '1', '0' after 20 ns;

  	 p_store_stream: process
        variable v_output_line_A : line;
        variable v_output_line_B : line;
        variable v_output_line_C : line;
        variable v_output_line_D : line;
        variable byte_count: integer := 0;
        variable byte_written: std_ulogic;
    begin
		byte_written := '0';
		bitcount <= 0;
		while enable loop
			wait until rising_edge(conduit_LED_A_CLK);

			rec_A(7-bitcount) <= conduit_LED_A_DATA;
			rec_B(7-bitcount) <= conduit_LED_B_DATA;
			rec_C(7-bitcount) <= conduit_LED_C_DATA;
			rec_D(7-bitcount) <= conduit_LED_D_DATA;


			if bitcount = 7 then
				wait for 11 ns;


				bitcount <= 0;
				byte_count := byte_count + 1;
				byte_written := '0';
			-- elsif bitcount = 0 and byte_written = '0' then
				write(v_output_line_A, to_string(byte_count), left, 16);
				write(v_output_line_A, to_hstring(rec_A));
				writeline(output_file_stream_A, v_output_line_A);

				write(v_output_line_B, to_string(byte_count), left, 16);
				write(v_output_line_B, to_hstring(rec_B));
				writeline(output_file_stream_B, v_output_line_B);

				write(v_output_line_C, to_string(byte_count), left, 16);
				write(v_output_line_C, to_hstring(rec_C));
				writeline(output_file_stream_C, v_output_line_C);

				write(v_output_line_D, to_string(byte_count), left, 16);
				write(v_output_line_D, to_hstring(rec_D));
				writeline(output_file_stream_D, v_output_line_D);
				byte_written := '1';

			else
				bitcount <= bitcount + 1;
			end if;




		end loop;
		-- wait until conduit_LED_A_CLK ='0';
    end process p_store_stream;




	p_stim_stream_out: process
		variable v_write_data : std_ulogic_vector(23 downto 0);
        variable v_input_line : line;
    begin
		input_file <= '0';
		wait until falling_edge(reset_reset);
        wait for 20 ns;
        while enable loop
			wait until conduit_col_info_out_fire = '1';
			wait for 20 ns;
			wait until rising_edge(clock_clk);

			avalon_stream_out_write_many_pixel(60, 20, clock_clk, asi_in0_data, asi_in0_valid,
                asi_in0_ready, asi_in0_startofpacket, asi_in0_endofpacket, 0, input_file, 0 );

			wait until rising_edge(clock_clk);
			wait for 50 ns;

			avalon_stream_out_write_many_pixel(60, 20, clock_clk, asi_in1_data, asi_in1_valid,
                asi_in1_ready, asi_in1_startofpacket, asi_in1_endofpacket, 0, input_file, 100 );
		end loop;

        wait for 200 us;

        wait;
    end process p_stim_stream_out;

    p_encoder_stim: process
    begin
		test_case <= 0;
		conduit_debug_led_led_dbg_in <= (others=>'0');
		conduit_debug_led_led_dbg_in(0) <= '1';
		wait for 500 ns;
		conduit_debug_led_led_dbg_in(1) <= '1';
		wait for 100 ns;
		conduit_debug_led_led_dbg_in(1) <= '0';
		wait for 20 us;
		conduit_col_info <= "0" & X"16";
		pulse_out(conduit_fire, clock_clk);

		wait for 30 us;
		conduit_col_info <= "0" & X"16";
		pulse_out(conduit_fire, clock_clk);

		wait for 20 us;
		conduit_debug_led_led_dbg_in(0) <= '0';

		wait for 1 us;
		-- load first buffer
		conduit_col_info <= "0" & X"16";
		pulse_out(conduit_fire, clock_clk);

		wait for c_cycle_real_firepulse;
		-- load second buffer
		conduit_col_info <= "0" & X"17";
		pulse_out(conduit_fire, clock_clk);

		wait for 10 us;

		-- check consecutive firepulses while spi transfer --> no harm --> need locking mechanism?
		test_case <= 1;
  --
		-- conduit_col_info <= "0" & X"17";
		-- pulse_out(conduit_fire, clock_clk);
  --
		-- wait for 100 ns;
		-- pulse_out(conduit_fire, clock_clk);


		wait;

    end process p_encoder_stim;


    p_monitor: process

    begin

		wait for 500 us;
		enable <= false;

		write(output, "all tested " & to_string(now) & lf);
		wait;
    end process p_monitor;


end architecture rtl; -- of led_interface
