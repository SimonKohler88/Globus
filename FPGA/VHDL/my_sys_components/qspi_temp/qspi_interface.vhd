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

entity qspi_interface is
	port (
		aso_out0_data          : out std_logic_vector(25 downto 0);                    --          aso_out0.data
		--aso_out0_endofpacket   : out std_logic;                                       --                  .endofpacket
		aso_out0_ready         : in  std_logic                    := '0';             --                  .ready
		--aso_out0_startofpacket : out std_logic;                                       --                  .startofpacket
		aso_out0_valid         : out std_logic;                                       --                  .valid
		clock_clk              : in  std_logic                    := '0';             --             clock.clk
		reset_reset            : in  std_logic                    := '0';             --             reset.reset
		conduit_qspi_data      : in  std_logic_vector(3 downto 0) := (others => '0'); --      conduit_qspi.qspi_data
		conduit_qspi_clk       : in  std_logic                    := '0';             --                  .qspi_clk
		conduit_qspi_cs        : in  std_logic                    := '0';             --                  .qspi_cs
		conduit_ping_pong      : out std_logic                          ;              -- conduit_ping_pong.new_signal

		conduit_debug_qspi_out    : out  std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_qspi.qspi_out
		conduit_debug_qspi_out_2    : out  std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_qspi.qspi_out_2
		conduit_debug_qspi_in     : in   std_logic_vector(31 downto 0)  := (others => '0') --                         .qspi_in
	);
end entity qspi_interface;

architecture rtl of qspi_interface is
	signal sync_spi_clk_reg     : std_logic_vector(1 downto 0);
	signal sync_spi_cs_reg      : std_logic_vector(1 downto 0);
	signal sync_spi_d0_reg      : std_logic_vector(1 downto 0);
	signal sync_spi_d1_reg      : std_logic_vector(1 downto 0);
	signal sync_spi_d2_reg      : std_logic_vector(1 downto 0);
	signal sync_spi_d3_reg      : std_logic_vector(1 downto 0);

	--signal sync_spi_clk_r1      : std_logic;
	--signal sync_spi_clk_r2      : std_logic;
	signal sync_spi_cs          : std_logic;
	signal sync_spi_data        : std_logic_vector(3 downto 0);

	signal sync_ff_cs_reg       : std_logic_vector(1 downto 0);
	signal cs_edge_start        : std_logic;
	-- signal cs_edge_end          : std_logic;
	signal sync_ff_clk_reg      : std_logic_vector(1 downto 0);

	signal data_in_buffer       : std_logic_vector(23 downto 0);
	signal data_takeover_pulse  : std_logic;
	signal nibble_count			: natural range 0 to 5  :=0;
	signal pixel_count			: natural range 0 to 120*256 + 10 := 0;
	signal last_pix  			: natural range 0 to 120*256 + 10 := 0;

	signal valid_ff_pulse_reg   : std_logic_vector(1 downto 0);
	signal valid_intern         : std_logic;

	signal symbol_streamed         : std_logic;
	signal transfer_ongoing        : std_logic;
	signal transfer_ongoing_ff     : std_logic_vector(1 downto 0);

	type state_test is (none, t1, t_data2ram, t_fifo_check);
	signal test_state         : state_test;

	signal test_pack_sig_stretch   :std_logic_vector(5 downto 0);
	signal test_pack_sig_stretch_2 :std_logic_vector(2 downto 0);
	signal test_pack_sig           :std_logic;
	signal test_pack_sig_2         :std_logic;

	signal aso_out0_startofpacket :std_logic;
	signal aso_out0_endofpacket :std_logic;
	signal intern_aso_out0_data :std_logic_vector(23 downto 0);


begin
	test_state <= t_fifo_check;
	p_test: process(all)
	begin
		case test_state is
			when none =>
				conduit_debug_qspi_out(31 downto 0) <= (others => '0');
				conduit_debug_qspi_out_2(31 downto 0) <= (others => '0');

			when t1 =>
				-- Test 1: check first pixel after transfer initiated
				conduit_debug_qspi_out(23 downto 0) <= intern_aso_out0_data;
				conduit_debug_qspi_out(31 downto 24) <= (others=>'0');

				if pixel_count=1 then
					conduit_debug_qspi_out_2(0) <= '1';
				else
					conduit_debug_qspi_out_2(0) <= '0';
				end if;
				conduit_debug_qspi_out_2(1) <= sync_spi_cs;
				conduit_debug_qspi_out_2(31 downto 2) <= (others=>'0');

			when t_data2ram =>
				conduit_debug_qspi_out(23 downto 0) <= intern_aso_out0_data;
				conduit_debug_qspi_out(31 downto 24) <= (others => '0');


				if pixel_count=1 then
					conduit_debug_qspi_out_2(0) <= '1';
				else
					conduit_debug_qspi_out_2(0) <= '0';
				end if;

				conduit_debug_qspi_out_2(1) <= aso_out0_valid;
				conduit_debug_qspi_out_2(2) <= aso_out0_ready;
				conduit_debug_qspi_out_2(3) <= test_pack_sig; -- startofpacket
				conduit_debug_qspi_out_2(4) <= test_pack_sig_2; --endofpacket
				conduit_debug_qspi_out_2(31 downto 5) <= (others => '0');

			when t_fifo_check =>
				conduit_debug_qspi_out(31 downto 0) <= (others => '0');

				conduit_debug_qspi_out_2 <=(
					25 => test_pack_sig, --sop
					26 => test_pack_sig_2, --eop
					27 => aso_out0_valid,
					28 => aso_out0_ready,
					others => '0'
				);
				if pixel_count=1 then
					conduit_debug_qspi_out_2(0) <= '1';
				else
					conduit_debug_qspi_out_2(0) <= '0';
				end if;
				
				conduit_debug_qspi_out_2(24 downto 1) <= intern_aso_out0_data;

			when others =>
				conduit_debug_qspi_out(31 downto 0) <= (others => '0');
				conduit_debug_qspi_out_2(31 downto 0) <= (others => '0');
		end case;
	end process;
	--debug process
	p_debug: process(all)
	begin
	 if reset_reset = '1' then
            test_pack_sig_stretch     <= (others => '0');
            test_pack_sig_stretch_2   <= (others => '0');
        elsif rising_edge(clock_clk) then
           test_pack_sig_stretch   <= test_pack_sig_stretch(test_pack_sig_stretch'length -2 downto 0)   & aso_out0_startofpacket;
           test_pack_sig_stretch_2 <= test_pack_sig_stretch_2(1 downto 0) & aso_out0_endofpacket;
        end if;
	end process;

	test_pack_sig <=   test_pack_sig_stretch(5) or test_pack_sig_stretch(4) or test_pack_sig_stretch(3) or test_pack_sig_stretch(2) or test_pack_sig_stretch(1) or test_pack_sig_stretch(0);
	test_pack_sig_2 <= test_pack_sig_stretch_2(2) or test_pack_sig_stretch_2(1) or test_pack_sig_stretch_2(0);



	aso_out0_data(24) <= aso_out0_startofpacket;
	aso_out0_data(25) <= aso_out0_endofpacket;
	aso_out0_data(23 downto 0) <= intern_aso_out0_data;


	-- sync in signals
	p_sync: process(all)
	begin
		if reset_reset = '1' then
			sync_spi_clk_reg <= (others =>'0');
			sync_spi_cs_reg  <= (others =>'0');
			sync_spi_d0_reg  <= (others =>'0');
			sync_spi_d1_reg  <= (others =>'0');
			sync_spi_d2_reg  <= (others =>'0');
			sync_spi_d3_reg  <= (others =>'0');

		elsif rising_edge(clock_clk) then
			sync_spi_clk_reg <=  sync_spi_clk_reg(0) & conduit_qspi_clk;
			sync_spi_cs_reg  <=  sync_spi_cs_reg (0) & conduit_qspi_cs     ;
			sync_spi_d0_reg  <=  sync_spi_d0_reg (0) & conduit_qspi_data(0);
			sync_spi_d1_reg  <=  sync_spi_d1_reg (0) & conduit_qspi_data(1);
			sync_spi_d2_reg  <=  sync_spi_d2_reg (0) & conduit_qspi_data(2);
			sync_spi_d3_reg  <=  sync_spi_d3_reg (0) & conduit_qspi_data(3);

		end if;
	end process p_sync;

	data_takeover_pulse <= '1' when sync_spi_clk_reg = "01" else '0';

	conduit_ping_pong <= '0'; --not used

	sync_spi_cs      <= sync_spi_cs_reg(1);
	sync_spi_data    <= sync_spi_d3_reg(1) & sync_spi_d2_reg(1) & sync_spi_d1_reg(1) & sync_spi_d0_reg(1);

	-- generate some edges for packet start, packet end
	p_generate_edges: process(all)
	begin
		if reset_reset = '1' then
			sync_ff_cs_reg <= (others => '0');
			cs_edge_start <= '0';
			-- cs_edge_end   <= '0';

		elsif rising_edge(clock_clk) then
			sync_ff_cs_reg <= sync_ff_cs_reg(0) & sync_spi_cs;

			-- generate edges
			if sync_ff_cs_reg = "10" then
				cs_edge_start <= '1';
			elsif sync_ff_cs_reg = "01" then
				-- cs_edge_end <= '1';
			else
				cs_edge_start <= '0';
				-- cs_edge_end <= '0';
			end if;
		end if;
	end process p_generate_edges;


	p_data_collector: process(all)
	begin
		if reset_reset = '1' then
			data_in_buffer <= (others => '0');
			nibble_count <= 0;
			valid_intern <= '0';
			pixel_count <= 0;
			intern_aso_out0_data <= (others => '0');

		elsif rising_edge(clock_clk) then

			--if sync_spi_cs = '1' then
			--end if;

			if cs_edge_start = '1' then
				--nibble_count <= 0;
				--data_in_buffer <= (others => '0');
				--valid_intern <= '0';
				pixel_count <= 0;

			elsif data_takeover_pulse='1' then
				data_in_buffer <= data_in_buffer(19 downto 0) & sync_spi_data;

				-- track nibbles
				if nibble_count=5 then
					nibble_count <= 0;
					pixel_count <= pixel_count + 1;
				else
					nibble_count <= nibble_count + 1;
				end if;

			end if;

			if nibble_count=0 and pixel_count > 0 then
				intern_aso_out0_data <= data_in_buffer ;
			end if;

			if pixel_count > 0 and nibble_count >= 0 and nibble_count < 5 then
				valid_intern <= '1';
			else
				valid_intern <= '0';
			end if;

			if transfer_ongoing_ff = "10" then
				nibble_count <= 0;
				data_in_buffer <= (others => '0');
				valid_intern <= '0';
				pixel_count <= 0;
				intern_aso_out0_data <= (others => '0');

			end if;
		end if;
	end process p_data_collector;

	p_valid_pos_edge:process(all)
	begin
		if reset_reset = '1' then
			valid_ff_pulse_reg <= (others => '0');
		elsif rising_edge(clock_clk) then
			valid_ff_pulse_reg <= valid_ff_pulse_reg(0) & valid_intern;
		end if;
	end process p_valid_pos_edge;


	-- generate startofpacket, valid and endofpacket. data already setup previously
	p_data_streamer: process(all)
	begin
		if reset_reset = '1' then
			symbol_streamed <= '0';
			aso_out0_valid <= '0';
			aso_out0_endofpacket <= '0';
			-- end_of_packet_streamed <= '1';
			transfer_ongoing <= '0';

		elsif rising_edge(clock_clk) then
			aso_out0_endofpacket <= '0';
			aso_out0_valid <= '0';

			-- check for intern valid edge to restart symbol-straming
			if valid_ff_pulse_reg="01" then
				symbol_streamed <= '0';

			elsif valid_ff_pulse_reg = "11" and sync_spi_cs='0' and
					symbol_streamed = '0' and aso_out0_ready = '1' and pixel_count > 0 then
				aso_out0_endofpacket <= '0';
				symbol_streamed <= '1';
				aso_out0_valid <= '1';
			end if;

			-- send start of packet at pix 1 (its actually pix 0, but currently receiving pix 1)
			if pixel_count = 1 and aso_out0_valid = '1' then
				transfer_ongoing <= '1';
			end if;

			-- last packet
			if transfer_ongoing = '1' and sync_spi_cs='1' and aso_out0_ready = '1' then
				aso_out0_endofpacket <= '1';
				aso_out0_valid <= '1';
				transfer_ongoing <= '0';
			end if;

			if transfer_ongoing_ff = "10" then


			end if;
		end if;
	end process p_data_streamer;

	aso_out0_startofpacket <= '1' when (pixel_count = 1 and aso_out0_valid = '1') else '0';

	p_transfer_end_edge:process(all)
	begin
		if reset_reset = '1' then
			transfer_ongoing_ff <= (others => '0');
		elsif rising_edge(clock_clk) then
			transfer_ongoing_ff <= transfer_ongoing_ff(0) & aso_out0_endofpacket ;
		end if;
	end process p_transfer_end_edge;
































end architecture rtl; -- of qspi_interface
