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

entity led_interface_verify is
	generic (
		num_led_A  : integer := 30;
		num_led_B  : integer := 30;
		num_led_C  : integer := 30;
		num_led_D  : integer := 30;
		image_rows : integer := 120
	);
	port (
		clock_clk                   : out  std_logic                     := '0';             --                clock.clk
		reset_reset                 : out  std_logic                     := '0';             --                reset.reset
		conduit_LED_A_CLK           : in   std_logic;                                        --        conduit_LED_A.led_a_clk
		conduit_LED_A_DATA          : in   std_logic;                                        --                     .led_a_data
		conduit_LED_B_CLK           : in   std_logic;                                        --        conduit_LED_B.led_b_clk
		conduit_LED_B_DATA          : in   std_logic;                                        --                     .led_b_data
		conduit_LED_C_CLK           : in   std_logic;                                        --        conduit_LED_C.led_c_clk
		conduit_LED_C_DATA          : in   std_logic;                                        --                     .led_c_data
		conduit_LED_D_CLK           : in   std_logic;                                        --        conduit_LED_D.led_d_clk
		conduit_LED_D_DATA          : in   std_logic;                                        --                     .new_signal_1
		clock_led_spi_clk           : out  std_logic                     := '0';             --        clock_led_spi.clk
		conduit_col_info            : out  std_logic_vector(8 downto 0)  := (others => '0'); --     conduit_col_info.col_nr
		conduit_fire                : out  std_logic                     := '0';             --                     .fire
		conduit_col_info_out_col_nr : in   std_logic_vector(8 downto 0);                     -- conduit_col_info_out.col_nr
		conduit_col_info_out_fire   : in   std_logic;                                        --                     .fire
		avs_s0_address              : out  std_logic_vector(7 downto 0)  := (others => '0'); --               avs_s0.address
		avs_s0_read                 : out  std_logic                     := '0';             --                     .read
		avs_s0_readdata             : in   std_logic_vector(31 downto 0);                    --                     .readdata
		avs_s0_write                : out  std_logic                     := '0';             --                     .write
		avs_s0_writedata            : out  std_logic_vector(31 downto 0) := (others => '0'); --                     .writedata
		avs_s0_waitrequest          : in   std_logic;                                        --                     .waitrequest
		asi_in1_data                : out  std_logic_vector(23 downto 0) := (others => '0'); --            asi_in1_B.data
		asi_in1_ready               : in   std_logic;                                        --                     .ready
		asi_in1_valid               : out  std_logic                     := '0';             --                     .valid
		asi_in1_startofpacket       : out  std_logic                     := '0';             --                     .startofpacket
		asi_in1_endofpacket         : out  std_logic                     := '0';             --                     .endofpacket
		asi_in0_data                : out  std_logic_vector(23 downto 0) := (others => '0'); --            asi_in0_A.data
		asi_in0_ready               : in   std_logic;                                        --                     .ready
		asi_in0_valid               : out  std_logic                     := '0';             --                     .valid
		asi_in0_startofpacket       : out  std_logic                     := '0';             --                     .startofpacket
		asi_in0_endofpacket         : out  std_logic                     := '0'              --                     .endofpacket
	);
end entity led_interface_verify;

architecture rtl of led_interface_verify is

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
    end procedure qspi_write_pixel;


	constant c_cycle_time_100M : time := 10 ns;
    signal enable :boolean:=true;

    file output_file_stream_A : text open write_mode is "./stream_out/stream_received_A.txt";
	file output_file_stream_B : text open write_mode is "./stream_out/stream_received_B.txt";

	file input_file : text open read_mode is "./Earth_relief_120x256_raw2.txt";
    constant c_pixel_to_send : integer := 4*20;

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

    reset_reset <= transport '1', '0' after 5 ns;

	 -- p_store_stream_A: process(all) --untested yet
  --       variable v_output_line : line;
  --   begin
  --       if rising_edge(clock_clk) then
  --           if aso_out0_A_ready = '1' and aso_out0_A_valid ='1' then
  --               write(v_output_line, to_string(now), left, 16);
  --               write(v_output_line, to_hstring(aso_out0_A_data));
  --               writeline(output_file_stream_A, v_output_line);
  --           end if;
  --       end if;
  --   end process p_store_stream_A;
  --
  --    p_store_stream_B: process(all) --untested yet
  --       variable v_output_line : line;
  --   begin
  --       if rising_edge(clock_clk) then
  --           if aso_out1_B_ready = '1' and aso_out1_B_valid ='1' then
  --               write(v_output_line, to_string(now), left, 16);
  --               write(v_output_line, to_hstring(aso_out1_B_data));
  --               writeline(output_file_stream_B, v_output_line);
  --           end if;
  --       end if;
  --   end process p_store_stream_B;

	p_stimuli: process
    begin


        wait for 2 ns;
        for i in 0 to c_pixel_to_send loop
            readline(input_file, v_input_line);
            hread(v_input_line, v_write_data);
            qspi_write_pixel(v_write_data, internal_qspi_clock, conduit_qspi_clk, conduit_qspi_data );
        end loop;

        wait for 200 us;

        wait;
    end process p_stimuli;

    p_monitor: process

    begin



		wait for 120 us;
		enable <= false;
		write(output, "all tested " & to_string(now) & lf);
		wait;
    end process p_monitor;


end architecture rtl; -- of led_interface
