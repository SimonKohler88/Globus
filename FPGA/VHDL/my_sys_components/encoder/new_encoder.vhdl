-- new_component.vhd

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


--known maybe-issue: if direction changed, there is maybe a column-jump by one

entity new_encoder is
	generic (
		input_pulse_per_rev : integer := 512;
		output_pulse_div    : integer := 2;
		debounce_cycles     : integer := 1000
	);
	port (
		avs_s0_address             : in  std_logic_vector(7 downto 0)  := (others => '0'); --                  avs_s0.address
		avs_s0_read                : in  std_logic                     := '0';             --                        .read
		avs_s0_readdata            : out std_logic_vector(31 downto 0);                    --                        .readdata
		avs_s0_write               : in  std_logic                     := '0';             --                        .write
		avs_s0_writedata           : in  std_logic_vector(31 downto 0) := (others => '0'); --                        .writedata
		avs_s0_waitrequest         : out std_logic;                                        --                        .waitrequest
		clock_clk                  : in  std_logic                     := '0';             --                   clock.clk
		reset_reset                : in  std_logic                     := '0';             --                   reset.reset
		conduit_encoder_A          : in  std_logic                     := '0';             --         conduit_encoder.enc_a
		conduit_encoder_B          : in  std_logic                     := '0';             --                        .enc_b
		conduit_encoder_index      : in  std_logic                     := '0';             --                        .enc_index
		conduit_encoder_sim_switch : in  std_logic                     := '0';             --                        .sim_switch
		conduit_encoder_sim_pulse  : in  std_logic                     := '0';             --                        .sim_pulse
		conduit_intern_col_nr      : out std_logic_vector(8 downto 0);                     -- conduit_intern_col_info.col_nr
		conduit_intern_col_fire    : out std_logic                    ;                     --                        .fire

		conduit_debug_enc_enc_dbg_out    : out  std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_enc.enc_dbg_out
		conduit_debug_enc_enc_dbg_out_2    : out  std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_enc.enc_dbg_out_2
		conduit_debug_enc_enc_dbg_in     : in   std_logic_vector(31 downto 0)  := (others => '0') --                         .led_dbg_in
	);
end entity new_encoder;

architecture rtl of new_encoder is

	signal sync_a_reg : std_logic_vector(2 downto 0);
	signal sync_b_reg : std_logic_vector(2 downto 0);
	signal sync_i_reg : std_logic_vector(2 downto 0);
	signal sync_sim_pulse_reg : std_logic_vector(2 downto 0);
	signal sync_sim_sw_reg : std_logic_vector(2 downto 0);


	signal sync_a : std_logic;
	signal sync_b : std_logic;
	signal sync_i : std_logic;
	signal sync_sim_switch : std_logic;

	signal rising_edge_a_reg : std_logic_vector(1 downto 0);
	signal rising_edge_sim_pulse_reg : std_logic_vector(1 downto 0);
	signal rising_edge_i_reg : std_ulogic_vector(1 downto 0);

	signal enc_clk: std_logic :='0';
	signal enc_direction : std_logic := '0'; -- 1: upcounting 0:downcounting
	signal i_valid : std_logic;

	signal rising_edge_fire_reg : std_logic_vector(1 downto 0);
	signal column_counter :unsigned(8 downto 0);
	signal debounce_counter :integer range 0 to debounce_cycles;
	signal debounce_counter_index :integer range 0 to debounce_cycles;
	
	signal dbg_led_firepulse: std_logic;


	type state_test is (none, t1, t2);
	signal test_state         : state_test;


begin
	test_state <= t2;
	p_test: process(all)
	begin
		case test_state is
			when none =>
				conduit_debug_enc_enc_dbg_out(2 downto 0) <= conduit_intern_col_nr(4 downto 2);
				conduit_debug_enc_enc_dbg_out(3) <= conduit_intern_col_fire;
				conduit_debug_enc_enc_dbg_out(31 downto 4) <= (others => '0');
				conduit_debug_enc_enc_dbg_out_2(31 downto 0) <= (others => '0');

			when t1 =>
				conduit_debug_enc_enc_dbg_out <= (others => '0');
				conduit_debug_enc_enc_dbg_out(9 downto 1) <= conduit_intern_col_nr(8 downto 0);
				conduit_debug_enc_enc_dbg_out(0) <= conduit_intern_col_fire;
				--conduit_debug_enc_enc_dbg_out(10) <= dbg_led_firepulse;
			when t2 =>
				conduit_debug_enc_enc_dbg_out <= (others => '0');
				-- conduit_debug_enc_enc_dbg_out(0) <= conduit_intern_col_fire;
				conduit_debug_enc_enc_dbg_out(0) <= sync_i;
				conduit_debug_enc_enc_dbg_out(9 downto 1) <= conduit_intern_col_nr(8 downto 0);
				conduit_debug_enc_enc_dbg_out(10) <= enc_clk;
				conduit_debug_enc_enc_dbg_out(11) <= sync_a;
				conduit_debug_enc_enc_dbg_out(12) <= sync_a_reg(2);
				conduit_debug_enc_enc_dbg_out(13) <= sync_sim_sw_reg(2);
				conduit_debug_enc_enc_dbg_out(14) <= sync_sim_pulse_reg(2);
				--sync_a_reg(2) when sync_sim_sw_reg(2)='0' else sync_sim_pulse_reg(2);
				
				
			when others =>
				conduit_debug_enc_enc_dbg_out(31 downto 0) <= (others => '0');
				conduit_debug_enc_enc_dbg_out_2(31 downto 0) <= (others => '0');

		end case;
	end process;
	
	dbg_fp_proc: process(all)
	begin
		if reset_reset = '1' then
			dbg_led_firepulse <= '0';
		elsif rising_edge(clock_clk) then
			if debounce_counter > 0 and column_counter(0)='1' then
				dbg_led_firepulse <= '1';
			else
				dbg_led_firepulse <= '0';
			end if;
		end if;
		
	end process;

	-- sync in encoder inputs
	sync_proc: process(reset_reset, clock_clk)
	begin
		if reset_reset = '1' then
			sync_a_reg <= (others => '0');
			sync_b_reg <= (others => '0');
			sync_i_reg <= (others => '1'); -- TODO: must change if enc_index changed to positive
			sync_sim_pulse_reg <= (others => '0');
			sync_sim_sw_reg <= (others => '0');

        elsif rising_edge(clock_clk) then
			sync_a_reg <= sync_a_reg(1 downto 0) & conduit_encoder_A;
			sync_b_reg <= sync_b_reg(1 downto 0) & conduit_encoder_B;
			sync_i_reg <= sync_i_reg(1 downto 0) & conduit_encoder_index;
			sync_sim_pulse_reg <= sync_sim_pulse_reg(1 downto 0) & conduit_encoder_sim_pulse;
			sync_sim_sw_reg <= sync_sim_sw_reg(1 downto 0) & conduit_encoder_sim_switch;
        end if;
	end process sync_proc;
	-- feed in simulation signals
	sync_a <= sync_a_reg(2) when sync_sim_sw_reg(2)='0' else sync_sim_pulse_reg(2);
	sync_b <= sync_b_reg(2) when sync_sim_sw_reg(2)='0' else '0';
	sync_i <= not sync_i_reg(2); -- TODO: hall sensor pulls signal down when triggered
	sync_sim_switch <= sync_sim_sw_reg(2);

	-- determing direction and rising edge of a and i
	rising_edge_proc: process(all)
	begin
		if reset_reset = '1' then
			rising_edge_a_reg <= (others => '0');
			rising_edge_i_reg <= (others => '0');
			enc_direction <= '0';
			enc_clk <= '0';
		elsif rising_edge(clock_clk) then
			if debounce_counter = 0 then
				rising_edge_a_reg <= rising_edge_a_reg(0) & sync_a;
			else
				debounce_counter <= debounce_counter + 1;
				if debounce_counter = debounce_cycles-1 then
					debounce_counter <= 0;
				end if;
			end if;

			rising_edge_i_reg <= rising_edge_i_reg(0) & sync_i;

			if rising_edge_a_reg="01" then
				debounce_counter <= 1;
				rising_edge_a_reg <= "11";
				enc_clk <= '1';
				if sync_a='1' and sync_b ='0' then
					enc_direction <= '1';
				else
					enc_direction <= '0';
				end if;
			else
				enc_clk <= '0';
			end if;
		end if;
	end process rising_edge_proc;

	--counting only valid if index has been seen
	index_valid_proc: process(all)
	begin
		if reset_reset = '1' then
			i_valid <= '0';
		elsif rising_edge(clock_clk) then
			if (i_valid='0' and rising_edge_i_reg="01") or sync_sim_switch='1' then
				i_valid <= '1';
			end if;
		end if;
	end process index_valid_proc;

	-- count to 512 with wraparound. col nr is divided by 2 for output
	col_counter_proc: process(all)
	begin
		if reset_reset = '1' or i_valid='0' then
			column_counter <= (others => '0');
			conduit_intern_col_nr <= (others => '0');
		elsif rising_edge(clock_clk) then
			if enc_clk='1' then
				-- no need to reset counter, it wraps around
				if enc_direction='1' then
					column_counter <= column_counter + 1;
				else
					column_counter <= column_counter - 1;
				end if;
				conduit_intern_col_nr <= "0" & std_logic_vector(column_counter(8 downto 1));
			end if;
        end if;
	end process col_counter_proc;

	-- generate fire-pulse from counter

	fire_pulse_proc: process(all)
	begin
		if reset_reset = '1' or i_valid='0' then
			rising_edge_fire_reg <= (others => '0');
			conduit_intern_col_fire <= '0';

		elsif rising_edge(clock_clk) then
			rising_edge_fire_reg <= rising_edge_fire_reg(0) & column_counter(0);

			if rising_edge_fire_reg = "01" and i_valid='1' then
				conduit_intern_col_fire <= '1';
			else
				conduit_intern_col_fire <= '0';
			end if;

		end if;
	end process fire_pulse_proc;


	-- avalon slave not yet implemented
	avs_s0_readdata <= "00000000000000000000000000000000";
	avs_s0_waitrequest <= '0';
































end architecture rtl; -- of new_component
