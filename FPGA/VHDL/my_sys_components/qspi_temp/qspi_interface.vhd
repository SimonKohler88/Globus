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
		aso_out0_data          : out std_logic_vector(23 downto 0);                    --          aso_out0.data
		aso_out0_endofpacket   : out std_logic;                                       --                  .endofpacket
		aso_out0_ready         : in  std_logic                    := '0';             --                  .ready
		aso_out0_startofpacket : out std_logic;                                       --                  .startofpacket
		aso_out0_valid         : out std_logic;                                       --                  .valid
		clock_clk              : in  std_logic                    := '0';             --             clock.clk
		reset_reset            : in  std_logic                    := '0';             --             reset.reset
		conduit_qspi_data      : in  std_logic_vector(3 downto 0) := (others => '0'); --      conduit_qspi.qspi_data
		conduit_qspi_clk       : in  std_logic                    := '0';             --                  .qspi_clk
		conduit_qspi_cs        : in  std_logic                    := '0';             --                  .qspi_cs
		conduit_ping_pong      : out std_logic                          ;              -- conduit_ping_pong.new_signal

		conduit_debug_qspi_out    : out  std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_qspi.qspi_out
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

	signal sync_spi_clk_r1      : std_logic;
	signal sync_spi_clk_r2      : std_logic;
	signal sync_spi_cs          : std_logic;
	signal sync_spi_data        : std_logic_vector(3 downto 0);

	signal sync_ff_cs_reg       : std_logic_vector(1 downto 0);
	signal cs_edge_start        : std_logic;
	signal cs_edge_end          : std_logic;
	signal sync_ff_clk_reg      : std_logic_vector(1 downto 0);

	signal data_in_buffer       : std_logic_vector(23 downto 0);
	signal data_takeover_pulse  : std_logic;
	signal nibble_count			: natural range 0 to 5  :=0;
	signal pixel_count			: natural range 0 to 120*256 + 10 := 0;
	signal last_pix  			: natural range 0 to 120*256 + 10 := 0;

	signal valid_ff_pulse_reg   : std_logic_vector(1 downto 0);
	signal valid_intern         : std_logic;

	signal symbol_streamed      : std_logic;

begin

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

			data_takeover_pulse <= '0';

		elsif rising_edge(clock_clk) then
			sync_spi_clk_reg <=  sync_spi_clk_reg(0) & conduit_qspi_clk;
			sync_spi_cs_reg  <=  sync_spi_cs_reg (0) & conduit_qspi_cs     ;
			sync_spi_d0_reg  <=  sync_spi_d0_reg (0) & conduit_qspi_data(0);
			sync_spi_d1_reg  <=  sync_spi_d1_reg (0) & conduit_qspi_data(1);
			sync_spi_d2_reg  <=  sync_spi_d2_reg (0) & conduit_qspi_data(2);
			sync_spi_d3_reg  <=  sync_spi_d3_reg (0) & conduit_qspi_data(3);

			if sync_spi_clk_reg = "01" then --pos edge
				data_takeover_pulse <= '1';
			else
				data_takeover_pulse <= '0';
			end if;
		end if;
	end process p_sync;

	conduit_ping_pong <= '0'; --not used

	sync_spi_cs      <= sync_spi_cs_reg(1);
	sync_spi_data    <= sync_spi_d3_reg(1) & sync_spi_d2_reg(1) & sync_spi_d1_reg(1) & sync_spi_d0_reg(1);

	-- generate some edges for packet start, packet end
	p_generate_edges: process(all)
	begin
		if reset_reset = '1' then
			sync_ff_cs_reg <= (others => '0');
			cs_edge_start <= '0';
			cs_edge_end   <= '0';

		elsif rising_edge(clock_clk) then
			sync_ff_cs_reg <= sync_ff_cs_reg(0) & sync_spi_cs;

			-- generate edges
			if sync_ff_cs_reg = "10" then
				cs_edge_start <= '1';
			elsif sync_ff_cs_reg = "01" then
				cs_edge_end <= '1';
			else
				cs_edge_start <= '0';
				cs_edge_end <= '0';
			end if;
		end if;
	end process p_generate_edges;


	p_data_collector: process(clock_clk, reset_reset)
	begin
		if reset_reset = '1' then
			data_in_buffer <= (others => '0');
			nibble_count <= 0;
			valid_intern <= '0';
			pixel_count <= 0;
			aso_out0_data <= (others => '0');

		elsif rising_edge(clock_clk) then

			if cs_edge_start = '1' then
				pixel_count <= 0;

			elsif data_takeover_pulse='1' then
				data_in_buffer <= data_in_buffer(19 downto 0) & sync_spi_data;

				if nibble_count=5 then
					nibble_count <= 0;
					pixel_count <= pixel_count + 1;
				else
					nibble_count <= nibble_count + 1;
				end if;

				if nibble_count=0 and pixel_count > 0 then
					aso_out0_data <= data_in_buffer ;
				end if;

				if pixel_count > 0 and nibble_count >= 0 and nibble_count < 5 then
					valid_intern <= '1';
				else
					valid_intern <= '0';
				end if;
			end if;
		end if;
	end process p_data_collector;
	-- aso_out0_data <= data_in_buffer when (nibble_count=0 and pixel_count > 0) else (others=>'0');


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
			aso_out0_startofpacket <= '0';
			last_pix <= 0;
			aso_out0_endofpacket <= '0';

		elsif rising_edge(clock_clk) then
			aso_out0_endofpacket <= '0';
			aso_out0_startofpacket <= '0';
			aso_out0_valid <= '0';

			if cs_edge_end = '1' then
				last_pix <= pixel_count;
			end if;

			if valid_ff_pulse_reg="01" then
				symbol_streamed <= '0';

			elsif valid_ff_pulse_reg = "11" and symbol_streamed = '0' and aso_out0_ready = '1' then
				aso_out0_endofpacket <= '0';
				symbol_streamed <= '1';
				aso_out0_valid <= '1';

				-- send start of packet at pix 1 (its actually pix 0, but currently receiving pix 1)
				if pixel_count = 1 then
					aso_out0_startofpacket <= '1';
				end if;

			-- since we dont know
			elsif pixel_count = last_pix and pixel_count > 0 then
				aso_out0_endofpacket <= '1';
				aso_out0_valid <= '1';
				last_pix <= 0;
			end if;
		end if;
	end process p_data_streamer;

	conduit_debug_qspi_out <= (others=>'0');
	-- if reset_reset = '1' then
	-- elsif rising_edge(clock_clk) then
	-- end if;
































end architecture rtl; -- of qspi_interface
