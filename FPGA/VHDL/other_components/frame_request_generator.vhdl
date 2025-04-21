

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;


entity frame_request_generator is
	generic (
		counter_bits : integer := 19;
		on_clocks : integer := 10000;
		dbg_enc_after_req0_clocks : integer := 1024-- default 1024 clock-cycles
	);
	port (
		clock_clk                   : in  std_logic                     := '0';             --                clock.clk
		reset_reset                 : in  std_logic                     := '0';
		enable_pin_input            : in  std_logic                     := '0';
		pin_input                   : in  std_logic                     := '0';
		pulse_out                   : out std_logic;
		enable_dbg_enc              : in std_logic;
		dbg_enc_out                 : out std_logic
    );
end entity frame_request_generator;

architecture rtl of frame_request_generator is

    signal counter :unsigned(counter_bits -1 downto 0);
    signal pin_counter :unsigned(counter_bits -1 downto 0);
	 signal pin_counter_is_counting :std_logic;
	 
	 signal enable_pin_input_ff : std_logic_vector(1 downto 0);
	 signal pin_input_ff : std_logic_vector(1 downto 0);
	 signal enable_dbg_enc_ff : std_logic_vector(1 downto 0);

	 signal enable_pin_input_intern : std_logic;
	 signal pin_input_intern_re : std_logic;

	 signal pulse_out_counter_intern : std_logic;
	 signal pulse_out_pin_in_intern : std_logic;

	 signal dbg_enc_counter : integer range 0 to dbg_enc_after_req0_clocks;
	 signal dbg_enc_counter_is_counting :std_logic;
	 signal dbg_enc_pulse_out_intern:std_logic;
	 signal dbg_enc_enable_intern:std_logic;
	 constant dbg_enc_counter_first_pulse :integer:=dbg_enc_after_req0_clocks-100;
begin
	
	-- generate internal pulse
   p_count: process(all)
	begin
		if reset_reset ='0' then
            counter <= (others=>'0');
		elsif rising_edge(clock_clk) then
            counter <= counter + 1;
		end if;
	end process;

   pulse_out_counter_intern <= '1' when counter < on_clocks else '0';
	
	-- double flopping in input signals
	p_in_flooping: process(all)
	begin
		if reset_reset ='0' then
            enable_pin_input_ff <= (others=>'0');
            pin_input_ff <= (others=>'0');
            enable_dbg_enc_ff <= (others=>'0');

		elsif rising_edge(clock_clk) then
            enable_pin_input_ff <= enable_pin_input_ff(0) & enable_pin_input;
            pin_input_ff <= pin_input_ff(0) & pin_input;
			enable_dbg_enc_ff <= enable_dbg_enc_ff (0) & enable_dbg_enc;
		end if;
	end process;

	enable_pin_input_intern <= enable_pin_input_ff(1);
	pin_input_intern_re <= pin_input_ff(0) and not pin_input_ff(1);
	dbg_enc_enable_intern <= enable_dbg_enc_ff(1);
	
	pulse_out <= '0';
	dbg_enc_pulse_out_intern <=  '0'; 
	pulse_out_pin_in_intern <=  '0'; 
	
	
	p_count_enc_counter: process(all)
	begin
		if reset_reset ='0' then
			dbg_enc_counter_is_counting <= '0';
			dbg_enc_counter <= 0;
		elsif rising_edge(clock_clk) then
			dbg_enc_out <= '0';
			if enable_pin_input_intern then

				if pin_input_intern_re='1' then
					dbg_enc_counter <= 0;
					dbg_enc_counter_is_counting <= '1';
				end if;

				if dbg_enc_counter_is_counting = '1' then

					if dbg_enc_counter < dbg_enc_after_req0_clocks then
						dbg_enc_counter <= dbg_enc_counter  + 1;
					else
						dbg_enc_counter_is_counting <= '0';
						dbg_enc_counter <= 0;
					end if;
					
					if dbg_enc_counter=dbg_enc_after_req0_clocks then
						dbg_enc_out <= '1';
					end if;
					
				end if;
			else
				dbg_enc_counter_is_counting <= '0';
				dbg_enc_counter <= 0;
			end if;
		end if;
	end process;

	-- dbg_enc_pulse_out_intern <=  '1' when dbg_enc_counter=dbg_enc_after_req0_clocks or dbg_enc_counter=dbg_enc_counter_first_pulse else '0';
	-- dbg_enc_out <= '1' when dbg_enc_enable_intern and dbg_enc_counter= else '0';
end architecture;




























