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

entity new_encoder is
	generic (
		input_pulse_per_rev : integer := 512;
		output_pulse_div    : integer := 2
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
		conduit_intern_col_fire    : out std_logic                                         --                        .fire
	);
end entity new_encoder;

architecture rtl of new_encoder is

	signal sync_a_reg : std_ulogic_vector(2 downto 0);
	signal sync_b_reg : std_ulogic_vector(2 downto 0);
	signal sync_i_reg : std_ulogic_vector(2 downto 0);
	signal sync_sim_pulse_reg : std_ulogic_vector(2 downto 0);
	signal sync_sim_sw_reg : std_ulogic_vector(2 downto 0);


	signal sync_a : std_ulogic;
	signal sync_b : std_ulogic;
	signal sync_i : std_ulogic;

	signal rising_edge_a_reg : std_ulogic_vector(1 downto 0);
	signal rising_edge_sim_pulse_reg : std_ulogic_vector(1 downto 0);
	--signal rising_edge_i_reg : std_ulogic_vector(1 downto 0);

	signal enc_clk: std_ulogic :='0';
	signal enc_direction : std_ulogic := '0'; -- 1: upcounting 0:downcounting
	signal i_valid : std_ulogic;

	signal rising_edge_fire_reg : std_logic_vector(1 downto 0);
	signal column_counter :unsigned(8 downto 0);


begin

	-- sync in encoder inputs
	sync_proc: process(reset_reset, clock_clk)
	begin
		if reset_reset = '1' then
			sync_a_reg <= (others => '0');
			sync_b_reg <= (others => '0');
			sync_i_reg <= (others => '0');
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
	sync_i <= sync_i_reg(2);

	-- determing direction and rising edge of a
	rising_edge_proc: process(all)
	begin
		if reset_reset = '1' then
			rising_edge_a_reg <= (others => '0');
			enc_direction <= '0';
			enc_clk <= '0';
		elsif rising_edge(clock_clk) then
			rising_edge_a_reg <= rising_edge_a_reg(0) & sync_a;

			if rising_edge_a_reg="01" then
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
			if i_valid='0' and sync_i='1' then
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
				conduit_intern_col_nr(7 downto 0) <= std_ulogic_vector(column_counter(8 downto 1));
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
