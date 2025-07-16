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

entity led_interface is
	generic (
		num_led_A  : integer := 30;
		num_led_B  : integer := 30;
		num_led_C  : integer := 30;
		num_led_D  : integer := 30;
		image_rows : integer := 120
	);
	port (
		clock_clk                   : in  std_logic                     := '0';             --                clock.clk
		reset_reset                 : in  std_logic                     := '0';             --                reset.reset
		conduit_LED_A_CLK           : out std_logic;                                        --        conduit_LED_A.led_a_clk
		conduit_LED_A_DATA          : out std_logic;                                        --                     .led_a_data
		conduit_LED_B_CLK           : out std_logic;                                        --        conduit_LED_B.led_b_clk
		conduit_LED_B_DATA          : out std_logic;                                        --                     .led_b_data
		conduit_LED_C_CLK           : out std_logic;                                        --        conduit_LED_C.led_c_clk
		conduit_LED_C_DATA          : out std_logic;                                        --                     .led_c_data
		conduit_LED_D_CLK           : out std_logic;                                        --        conduit_LED_D.led_d_clk
		conduit_LED_D_DATA          : out std_logic;                                        --                     .new_signal_1
		clock_led_spi_clk           : in  std_logic                     := '0';             --        clock_led_spi.clk
		conduit_col_info            : in  std_logic_vector(8 downto 0)  := (others => '0'); --     conduit_col_info.col_nr
		conduit_fire                : in  std_logic                     := '0';             --                     .fire
		conduit_col_info_out_col_nr : out std_logic_vector(8 downto 0);                     -- conduit_col_info_out.col_nr
		conduit_col_info_out_fire   : out std_logic;                                        --                     .fire
		avs_s0_address              : in  std_logic_vector(7 downto 0)  := (others => '0'); --               avs_s0.address
		avs_s0_read                 : in  std_logic                     := '0';             --                     .read
		avs_s0_readdata             : out std_logic_vector(31 downto 0);                    --                     .readdata
		avs_s0_write                : in  std_logic                     := '0';             --                     .write
		avs_s0_writedata            : in  std_logic_vector(31 downto 0) := (others => '0'); --                     .writedata
		avs_s0_waitrequest          : out std_logic;                                        --                     .waitrequest
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

		conduit_debug_led_led_dbg_out    : out  std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_led.led_dbg_out
		conduit_debug_led_led_dbg_out_2    : out  std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_led.led_dbg_out_2
		conduit_debug_led_led_dbg_in     : in   std_logic_vector(31 downto 0)  := (others => '0') --                         .led_dbg_in
	);
end entity led_interface;

architecture rtl of led_interface is

	constant PIX_PER_STREAM_IN: integer:= 60;
	constant PIX_OUT_PER_SPI: integer:= 30;
	constant BIT_PER_LED_FRAME :integer := 32;
	constant BIT_PER_SPI_START_END_FRAME: integer:= 32;
	constant BRIGHTNESS : std_logic_vector(4 downto 0) := "01000";
	-- constant BRIGHTNESS : std_logic_vector(4 downto 0) := "01111";
	-- 01000: 8 out of 32 == 1/4
	-- 10000: 16 == 1/2
	-- 11111: 31 == full throttle
	-- 00100: 4 == 1/8
	
	-- 0100 0000 == 0x40
	-- 00001000 == 0x08
	-- from protocol: 1110 1000 = 0xE8

	signal use_bgr: std_logic:= '1';

	type t_pixel_buffer_in_array is array (0 to PIX_PER_STREAM_IN-1) of std_logic_vector(23 downto 0);
	type t_pixel_buffer_out_array is array (0 to PIX_OUT_PER_SPI-1) of std_logic_vector(31 downto 0);

	signal in_buffer_stream_A: t_pixel_buffer_in_array := (others =>(others => '0'));
	signal in_buffer_stream_B: t_pixel_buffer_in_array := (others =>(others => '0'));

	signal pix_out_A :t_pixel_buffer_out_array := (others =>(others => '0'));
	signal pix_out_B :t_pixel_buffer_out_array := (others =>(others => '0'));
	signal pix_out_C :t_pixel_buffer_out_array := (others =>(others => '0'));
	signal pix_out_D :t_pixel_buffer_out_array := (others =>(others => '0'));

	signal start_spi_pulse :std_logic;
	signal spi_pulse_stretch :std_logic_vector(12 downto 0);
	signal sync_start_spi_pulse_ff :std_logic_vector(1 downto 0);
	signal sync_start_spi_pulse  :std_logic;

	signal fire_delay_ff: std_logic_vector(2 downto 0);
	signal fire_out :std_logic;
	signal conduit_fire_signal : std_logic;

	signal pix_in_counter_A : natural range 0 to 70 := 0;
	signal pix_in_counter_B : natural range 0 to 70 := 0;

	signal spi_out_enable :std_logic;
	signal spi_bit_count_A: natural range 0 to 32 := 0;
	signal spi_pix_count_A: natural range 0 to PIX_OUT_PER_SPI := 0;

	type t_spi_state is (idle, send_start_frame, send_buffer, send_end_frame, end_send);
	signal spi_state : t_spi_state;
	signal next_spi_state : t_spi_state;
	signal spi_in_progress: std_logic;
	signal spi_in_progress_ff: std_logic_vector(12 downto 0);
	signal spi_fake_cs: std_logic;

	signal dbg_0_in_ff: std_logic_vector(1 downto 0);
	signal clear_led_pulse: std_logic;

	signal ram_to_buff_in_progress :std_logic;

	type state_test is (none, t1, t_data_in, t_fp, t_fifo_check);
	signal test_state         : state_test;


begin
	test_state <= t_data_in;
	p_test: process(all)
	begin
		case test_state is
			when none =>
				conduit_debug_led_led_dbg_out <= (others=>'0');
				conduit_debug_led_led_dbg_out_2 <= (others=>'0');

			when t1 => -- measure spi time
				conduit_debug_led_led_dbg_out(0) <= spi_in_progress;
				conduit_debug_led_led_dbg_out(31 downto 1) <= (others=>'0');
				conduit_debug_led_led_dbg_out_2 <= (others=>'0');
				
            when t_data_in =>
				conduit_debug_led_led_dbg_out <= (others => '0');
				conduit_debug_led_led_dbg_out(7 downto 0) <= conduit_col_info(7 downto 0);
				conduit_debug_led_led_dbg_out(31 downto 8) <= pix_out_A(spi_pix_count_A)(23 downto 0);
				
				
				-- conduit_debug_led_led_dbg_out_2 <= (
    --                 --0 => conduit_fire,
    --                 1 => spi_in_progress,
    --                 27 => asi_in0_valid,
    --                 others=>'0'
    --             );
                conduit_debug_led_led_dbg_out_2(24 downto 1) <= asi_in0_data;
                
				if conduit_col_info="000000000" then
					conduit_debug_led_led_dbg_out_2(0) <= '1';
				else
					conduit_debug_led_led_dbg_out_2(0) <= '0';
				end if;

			when t_fp =>
				conduit_debug_led_led_dbg_out <= (others => '0');

				conduit_debug_led_led_dbg_out_2 <= (
                    0 => conduit_fire,
                    1 => spi_in_progress,
                    2 => conduit_LED_A_CLK,
                    3 => spi_fake_cs,
                    others=>'0'
                );

			when t_fifo_check=>
				-- Must enable redirection of fp-input. Check commented line below (ca line 182)
				conduit_debug_led_led_dbg_out <= (others => '0');
				conduit_debug_led_led_dbg_out_2 <= (
                    0 => conduit_fire,
                    1 => ram_to_buff_in_progress,
                    2 => asi_in0_valid or asi_in1_valid,
                    3 => spi_fake_cs,
                    others=>'0'
                );


			when others =>
				conduit_debug_led_led_dbg_out <= (others=>'0');
				conduit_debug_led_led_dbg_out_2 <= (others=>'0');

		end case;
	end process;


	avs_s0_readdata <= "00000000000000000000000000000000";
	avs_s0_waitrequest <= '0';
    
    use_bgr <= '1';
    
	conduit_col_info_out_fire <= fire_out;
	
	-- dangerous line: discards real fp, takes debug fp instead when in t_fifo_check
	-- conduit_fire_signal <= conduit_debug_led_led_dbg_in(1) when test_state=t_fifo_check else conduit_fire;
	conduit_fire_signal <= conduit_fire; -- good line

	-- delay fp to RAM Master a few clk for no real reason.
	p_fire_delay : process(all)
	begin
		if reset_reset ='1' then
			fire_delay_ff <= (others => '0');
			conduit_col_info_out_col_nr <= (others => '0');
		elsif rising_edge(clock_clk) then
			fire_delay_ff <= fire_delay_ff(1 downto 0) & conduit_fire_signal;
			
			if fire_delay_ff(2 downto 1) = "01" then
				fire_out <= '1';
				conduit_col_info_out_col_nr <= conduit_col_info;
			else
				fire_out <= '0';
			end if;
		end if;
	end process p_fire_delay;

	-- debug purpose
	p_ram_to_buff_in_progress: process(all)
	begin

	if reset_reset='1' then
		ram_to_buff_in_progress <= '0';
	elsif rising_edge(clock_clk) then

		if asi_in0_startofpacket = '1' then
			ram_to_buff_in_progress <= '1';
		elsif asi_in1_endofpacket = '1' then
			ram_to_buff_in_progress<= '0';
		end if;
	end if;
	end process;

	-- verify purpose: make a chip select for SPI- transfer:
	-- Will be read by second ESP over QSPI and sent to server
	p_dbg_fake_cs: process(all)
	begin
	if reset_reset='1' then
		spi_fake_cs <= '1';
		spi_in_progress_ff <= (others=>'0');
	elsif rising_edge(clock_clk) then
		spi_in_progress_ff <= spi_in_progress_ff(spi_in_progress_ff'length-2 downto 0) & spi_in_progress;

		if fire_delay_ff(1 downto 0) = "01" then -- rising edge of firepulse
			spi_fake_cs <= '0';
		elsif spi_in_progress_ff(spi_in_progress_ff'length-1 downto spi_in_progress_ff'length-2) = "10" then -- falling edge, delayed
			spi_fake_cs <= '1';
		end if;
	end if;
	end process;

	-- read in dbg_in(0). on rising edge, give info to p_data_to_buffer for a LED turning off SPI-Burst
	p_empty_shot: process(all)
	begin
	if reset_reset ='1' then
		dbg_0_in_ff <= (others=>'0');

	elsif rising_edge(clock_clk) then
		dbg_0_in_ff <= dbg_0_in_ff(0) & conduit_debug_led_led_dbg_in(0);
		if dbg_0_in_ff(1) = '0' and dbg_0_in_ff(0) = '1' then
			clear_led_pulse <= '1';
		else
			clear_led_pulse <= '0';
		end if;
	end if;
	end process;

	--receiving streams to buffer
	p_receive_stream_A: process(all)
	begin
		if reset_reset ='1' then
			in_buffer_stream_A  <= (others =>(others => '0'));
			pix_in_counter_A <= 0;

		elsif rising_edge(clock_clk) then
			if asi_in0_valid = '1' and asi_in0_ready ='1' and pix_in_counter_A < PIX_PER_STREAM_IN then
				in_buffer_stream_A( pix_in_counter_A ) <= asi_in0_data;
				-- in_buffer_stream_A( pix_in_counter_A ) <= X"FFFFFF";
				pix_in_counter_A <= pix_in_counter_A + 1;
			end if;

			if asi_in0_endofpacket = '1' then
				pix_in_counter_A <= 0;
			end if;
		end if;
	end process p_receive_stream_A;

	p_receive_stream_B: process(all)
	begin
		if reset_reset ='1' then
			in_buffer_stream_B  <= (others =>(others => '0'));
			pix_in_counter_B <= 0;

		elsif rising_edge(clock_clk) then
			if asi_in1_valid = '1' and asi_in1_ready ='1' and pix_in_counter_B < PIX_PER_STREAM_IN then
				in_buffer_stream_B( pix_in_counter_B ) <= asi_in1_data;
				-- in_buffer_stream_B( pix_in_counter_B ) <= X"FFFFFF";
				pix_in_counter_B <= pix_in_counter_B + 1;
			end if;

			if asi_in1_endofpacket = '1' then
				pix_in_counter_B <= 0;
			end if;
		end if;
	end process p_receive_stream_B;

	-- buffer to spi out buffers, always when fire pulse is on
	p_data_to_buffer: process(all)
	begin
		if reset_reset ='1' then
			pix_out_A           <= (others =>(others => '0'));
			pix_out_B           <= (others =>(others => '0'));
			pix_out_C           <= (others =>(others => '0'));
			pix_out_D           <= (others =>(others => '0'));
			spi_pulse_stretch <= (others => '0');

		elsif rising_edge(clock_clk) then

			if conduit_fire = '1' or clear_led_pulse='1' then -- todo: gamma

				for a in 0 to (pix_out_A'length - 1) loop

					if clear_led_pulse='1' then
						pix_out_A(a) <= (others=>'0');
					else
						if use_bgr='1' then
							--BGR
							pix_out_A(a)(7 downto 0)   <= in_buffer_stream_A(pix_out_A'length - 1 - a)(23 downto 16);
							pix_out_A(a)(15 downto 8)  <= in_buffer_stream_A(pix_out_A'length - 1 - a)(15 downto 8) ;
							pix_out_A(a)(23 downto 16) <= in_buffer_stream_A(pix_out_A'length - 1 - a)(7 downto 0)  ;
						else
							--RGB
							pix_out_A(a)(23 downto 0) <= in_buffer_stream_A(a);
						end if;
					end if;

					pix_out_A(a)(31 downto 29) <= "111";
					pix_out_A(a)(28 downto 24)  <= BRIGHTNESS;
				end loop;

				for b in 0 to (pix_out_B'length - 1) loop -- change direction
					if clear_led_pulse='1' then
						pix_out_B(b) <= (others=>'0');
					else
						if use_bgr='1' then
							--BGR
							pix_out_B(b)(7 downto 0)   <= in_buffer_stream_A(in_buffer_stream_A'length - 1 - b)(23 downto 16);
							pix_out_B(b)(15 downto 8)  <= in_buffer_stream_A(in_buffer_stream_A'length - 1 - b)(15 downto 8) ;
							pix_out_B(b)(23 downto 16) <= in_buffer_stream_A(in_buffer_stream_A'length - 1 - b)(7 downto 0)  ;
						else
							--RGB
							pix_out_B(b)(23 downto 0) <= in_buffer_stream_A(in_buffer_stream_A'length - 1 - b);
						end if;
					end if;

					pix_out_B(b)(31 downto 29)  <= "111";
					pix_out_B(b)(28 downto 24) <= BRIGHTNESS;
				end loop;

				for c in 0 to (pix_out_C'length - 1) loop
					if clear_led_pulse='1' then
						pix_out_C(c) <= (others=>'0');
					else
						if use_bgr='1' then
							--BGR
							pix_out_C(c)(7 downto 0)   <= in_buffer_stream_B(pix_out_C'length - 1 - c)(23 downto 16);
							pix_out_C(c)(15 downto 8)  <= in_buffer_stream_B(pix_out_C'length - 1 - c)(15 downto 8) ;
							pix_out_C(c)(23 downto 16) <= in_buffer_stream_B(pix_out_C'length - 1 - c)(7 downto 0)  ;
						else
							--RGB
							pix_out_C(c)(23 downto 0)<= in_buffer_stream_B(c);
						end if;
					end if;

					pix_out_C(c)(31 downto 29) <= "111";
					pix_out_C(c)(28 downto 24)  <= BRIGHTNESS;
				end loop;

				for d in 0 to (pix_out_D'length - 1) loop -- change direction
					if clear_led_pulse='1' then
						pix_out_D(d) <= (others=>'0');
					else
						if use_bgr='1' then
							--BGR
							pix_out_D(d)(7 downto 0)   <= in_buffer_stream_B(in_buffer_stream_B'length - 1 - d)(23 downto 16);
							pix_out_D(d)(15 downto 8)  <= in_buffer_stream_B(in_buffer_stream_B'length - 1 - d)(15 downto 8) ;
							pix_out_D(d)(23 downto 16) <= in_buffer_stream_B(in_buffer_stream_B'length - 1 - d)(7 downto 0)  ;
						else
							--RGB
							pix_out_D(d)(23 downto 0) <= in_buffer_stream_B(in_buffer_stream_B'length - 1 - d);
						end if;
					end if;
					pix_out_D(d)(31 downto 29)  <= "111";
					pix_out_D(d)(28 downto 24)  <= BRIGHTNESS;
				end loop;

				spi_pulse_stretch <= spi_pulse_stretch(11 downto 0) & "1";
			else
				spi_pulse_stretch <= spi_pulse_stretch(11 downto 0) & "0";
			end if;
		end if;
	end process;

	-- stretch spi start pulse for clock domain crossing
	start_spi_pulse <= spi_pulse_stretch(11) or spi_pulse_stretch(10) or spi_pulse_stretch(9) or spi_pulse_stretch(8) 
					or spi_pulse_stretch(7)  or spi_pulse_stretch(6) or spi_pulse_stretch(5) or spi_pulse_stretch(4) 
					or spi_pulse_stretch(3) or spi_pulse_stretch(2) or spi_pulse_stretch(1) or spi_pulse_stretch(0);

	-- generate start pulse in spi clock domain
	p_spi_sync: process(all)
	begin
		if reset_reset ='1' then
			sync_start_spi_pulse_ff <= (others => '0');
		elsif rising_edge(clock_led_spi_clk) then
			sync_start_spi_pulse_ff <= sync_start_spi_pulse_ff(0) & start_spi_pulse;
		end if;
	end process;
	sync_start_spi_pulse <= sync_start_spi_pulse_ff(0) and  not sync_start_spi_pulse_ff(1);

	-- state machine for spi led
	p_spi_state_clocked: process(all)
	begin
		if reset_reset	= '1' then
			spi_state  <= idle;

			spi_bit_count_A <= BIT_PER_LED_FRAME-1;
			spi_pix_count_A <= 0;

		elsif falling_edge(clock_led_spi_clk) then
			spi_state <= next_spi_state;

			if spi_state /= next_spi_state then

				spi_bit_count_A  <= BIT_PER_LED_FRAME-1;
				if sync_start_spi_pulse='1' then
					spi_pix_count_A <= 0;
				end if;

			elsif spi_state /= idle then
				if spi_state=send_start_frame or spi_state=send_end_frame then
					spi_bit_count_A <= spi_bit_count_A - 1;
				else
					if spi_bit_count_A = 0 then
						spi_bit_count_A  <= BIT_PER_LED_FRAME-1;
						spi_pix_count_A <= spi_pix_count_A + 1;
					else
						spi_bit_count_A <= spi_bit_count_A - 1;
					end if;
				end if;
			end if;
		end if;
	end process;

	p_spi_state_comb: process(all)
	begin
	--idle, send_start_frame, send_buffer, send_end_frame, end_send
		case spi_state is
			when idle =>
				if sync_start_spi_pulse='1' then
					next_spi_state <= send_start_frame;
				else
					next_spi_state <= idle;
				end if;

			when send_start_frame =>
				if spi_bit_count_A = 0 then
					next_spi_state <= send_buffer;
				else
					next_spi_state <= send_start_frame;
				end if;

			when send_buffer =>
				if spi_pix_count_A = PIX_OUT_PER_SPI-1 and spi_bit_count_A = 0 then
					next_spi_state <= send_end_frame;
				else
					next_spi_state <= send_buffer;
				end if;

			when send_end_frame =>
				if spi_bit_count_A = 0 then
					next_spi_state <= end_send;
				else
					next_spi_state <= send_end_frame;
				end if;

			when end_send =>
				next_spi_state <= idle;
			when others =>
				next_spi_state <= idle;
		end case;

		case spi_state is
			when idle =>
				spi_out_enable   <= '0';
				conduit_LED_A_DATA <= '0';
				conduit_LED_B_DATA <= '0';
				conduit_LED_C_DATA <= '0';
				conduit_LED_D_DATA <= '0';

			when send_start_frame =>
				conduit_LED_A_DATA <= '0';
				conduit_LED_B_DATA <= '0';
				conduit_LED_C_DATA <= '0';
				conduit_LED_D_DATA <= '0';
				spi_out_enable   <= '1';

			when send_buffer =>

				conduit_LED_A_DATA <= pix_out_A(spi_pix_count_A)(spi_bit_count_A); --todo: change when pixel count differ in A,B,C,D
				conduit_LED_B_DATA <= pix_out_B(spi_pix_count_A)(spi_bit_count_A);
				conduit_LED_C_DATA <= pix_out_C(spi_pix_count_A)(spi_bit_count_A);
				conduit_LED_D_DATA <= pix_out_D(spi_pix_count_A)(spi_bit_count_A);

				spi_out_enable   <= '1';

			when send_end_frame =>
				conduit_LED_A_DATA <= '1';
				conduit_LED_B_DATA <= '1';
				conduit_LED_C_DATA <= '1';
				conduit_LED_D_DATA <= '1';
				spi_out_enable   <= '1';

			when end_send =>
				conduit_LED_A_DATA <= '0';
				conduit_LED_B_DATA <= '0';
				conduit_LED_C_DATA <= '0';
				conduit_LED_D_DATA <= '0';
				spi_out_enable   <= '0';

			when others =>
				conduit_LED_A_DATA <= '0';
				conduit_LED_B_DATA <= '0';
				conduit_LED_C_DATA <= '0';
				conduit_LED_D_DATA <= '0';
				spi_out_enable   <= '0';
		end case;

	end process;
	conduit_LED_A_CLK <= clock_led_spi_clk when spi_out_enable else '0';
	conduit_LED_B_CLK <= clock_led_spi_clk when spi_out_enable else '0';
	conduit_LED_C_CLK <= clock_led_spi_clk when spi_out_enable else '0';
	conduit_LED_D_CLK <= clock_led_spi_clk when spi_out_enable else '0';

	spi_in_progress <= '0' when spi_state=idle else '1';

	asi_in1_ready <= '1';

	asi_in0_ready <= '1';

end architecture rtl; -- of led_interface
