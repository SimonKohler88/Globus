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

entity new_encoder_tb is
end entity new_encoder_tb;

architecture rtl of new_encoder_tb is

	component new_encoder is
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
		conduit_intern_col_fire    : out std_logic ;                                        --                        .fire

		conduit_debug_enc_enc_dbg_out   : out  std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_enc.enc_dbg_out
		conduit_debug_enc_enc_dbg_in    : in   std_logic_vector(31 downto 0)  := (others => '0') --                         .led_dbg_in
	);

	end component;

	signal s_avs_s0_address              :  std_logic_vector(7 downto 0) ;
	signal s_avs_s0_read                 :  std_logic                    ;
	signal s_avs_s0_readdata             :  std_logic_vector(31 downto 0);
	signal s_avs_s0_write                :  std_logic                    ;
	signal s_avs_s0_writedata            :  std_logic_vector(31 downto 0);
	signal s_avs_s0_waitrequest          :  std_logic                    ;
	signal clk                           :  std_logic                    ;
	signal reset                         :  std_logic                    ;
	signal s_conduit_encoder_index       :  std_logic                    ;
	signal s_conduit_encoder_sim_switch  :  std_logic                    ;
	signal s_conduit_encoder_sim_pulse   :  std_logic                    ;
	signal s_conduit_intern_col_nr       :  std_logic_vector(8 downto 0) ;
	signal s_conduit_intern_col_fire     :  std_logic                    ;
	signal s_conduit_debug_enc_out      : std_logic_vector(31 downto 0);
	signal s_conduit_debug_enc_in       : std_logic_vector(31 downto 0);


	constant c_cycle_time : time := 10 ns;
	constant c_enc_t_per_u :time := 100 us;
	constant c_enc_index_time : time := 195 ns; --c_enc_t_per_u/512;
	constant c_enc_S_time : time := 49 ns; --c_enc_index_time/4;
	signal enable         : boolean := true;
	signal enable_a_b         : std_ulogic := '1';
	signal A :std_ulogic_vector(3 downto 0) := "1100";
	signal B :std_ulogic_vector(3 downto 0) := "0110";
	signal indexCount : unsigned(8 downto 0);


begin

	dut:  new_encoder
	port map(
		avs_s0_address             => s_avs_s0_address              ,
		avs_s0_read                => s_avs_s0_read                 ,
		avs_s0_readdata            => s_avs_s0_readdata             ,
		avs_s0_write               => s_avs_s0_write                ,
		avs_s0_writedata           => s_avs_s0_writedata            ,
		avs_s0_waitrequest         => s_avs_s0_waitrequest          ,
		clock_clk                  => clk                           ,
		reset_reset                => reset                         ,
		conduit_encoder_A          => A(0)                          ,
		conduit_encoder_B          => B(0)                          ,
		conduit_encoder_index      => s_conduit_encoder_index       ,
		conduit_encoder_sim_switch => s_conduit_encoder_sim_switch  ,
		conduit_encoder_sim_pulse  => s_conduit_encoder_sim_pulse   ,
		conduit_intern_col_nr      => s_conduit_intern_col_nr       ,
		conduit_intern_col_fire    => s_conduit_intern_col_fire,

		conduit_debug_enc_enc_dbg_out   => s_conduit_debug_enc_out,
		conduit_debug_enc_enc_dbg_in    => s_conduit_debug_enc_in
	);

	reset <= transport '1', '0' after 5 ns;


	-- 100MHz
	p_system_clk : process
	begin
		while enable loop
		clk <= '0';
		wait for c_cycle_time/2;
		clk <= '1';
		wait for c_cycle_time/2;
		end loop;
		wait;  -- don't do it again
	end process p_system_clk;

	sim_pulse_clk_proc :process
	begin
		while enable loop
		s_conduit_encoder_sim_pulse <= '0';
		wait for c_enc_index_time;
		s_conduit_encoder_sim_pulse <= '1';
		wait for c_enc_index_time;
		end loop;
		wait;

	end process sim_pulse_clk_proc;

	stim_proc_encoder: process
	begin
		-- A before B
		-- From AMTS datasheet
		-- T = 1/(360/512 * 33.33) = 0.042s
		-- P = T/2 = 0.0213 s
		-- S = P/2 = 0.0106 s
		indexCount <= "000000000";
		--indexCount  <= indexCount + 1;
		while enable loop
			if enable_a_b='1' then
				wait for c_enc_S_time;
				A <= A(2 downto 0) & A(3);
				B <= B(2 downto 0) & B(3);
				wait for c_enc_S_time;
				A <= A(2 downto 0) & A(3);
				B <= B(2 downto 0) & B(3);
				wait for c_enc_S_time;
				A <= A(2 downto 0) & A(3);
				B <= B(2 downto 0) & B(3);
				wait for c_enc_S_time;
				A <= A(2 downto 0) & A(3);
				B <= B(2 downto 0) & B(3);
				indexCount   <= indexCount + 1;

			else
				wait for c_enc_S_time;
				A <= A(0) & A(3 downto 1);
				B <= B(0) & B(3 downto 1);
				wait for c_enc_S_time;
				A <= A(0) & A(3 downto 1);
				B <= B(0) & B(3 downto 1);
				wait for c_enc_S_time;
				A <= A(0) & A(3 downto 1);
				B <= B(0) & B(3 downto 1);
				wait for c_enc_S_time;
				A <= A(0) & A(3 downto 1);
				B <= B(0) & B(3 downto 1);
				indexCount   <= indexCount - 1;

			end if;

		end loop;
		report "encoder testbench finished";
        wait;
    end process stim_proc_encoder;

	s_conduit_encoder_index <= '1' when indexCount = 55 else '0';

    stim_main_proc: process
    begin
	enable_a_b<='1';
		s_conduit_encoder_sim_switch <= '0';
		-- normal counting up
		wait for 170 us;

		--normal counting down
		enable_a_b <= '0';
		wait for 170 us;

		--setup for sim
		enable_a_b <= '1';
		wait for 50 us;
		-- check if feeding in simulation signals works
		s_conduit_encoder_sim_switch <= '1';
		wait for 170 us;

		wait for 10 us;
		enable <= false;

    end process stim_main_proc;









end architecture rtl; -- of new_component
