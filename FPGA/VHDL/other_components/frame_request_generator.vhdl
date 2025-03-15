

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;


entity frame_request_generator is
	generic (
		counter_bits : integer := 19;
		on_clocks : integer := 10000
	);
	port (
		clock_clk                   : in  std_logic                     := '0';             --                clock.clk
		reset_reset                 : in  std_logic                     := '0';
		enable_pin_input            : in  std_logic                     := '0';
		pin_input                   : in  std_logic                     := '0';
      pulse_out                   : out std_logic
    );
end entity frame_request_generator;

architecture rtl of frame_request_generator is

    signal counter :unsigned(counter_bits -1 downto 0);
    signal pin_counter :unsigned(counter_bits -1 downto 0);
	 signal pin_counter_is_counting :std_logic;
	 
	 signal enable_pin_input_ff : std_logic_vector(1 downto 0);
	 signal pin_input_ff : std_logic_vector(1 downto 0);
	 
	 signal enable_pin_input_intern : std_logic;
	 signal pin_input_intern_re : std_logic;
	 
	 signal pulse_out_counter_intern : std_logic;
	 signal pulse_out_pin_in_intern : std_logic;
	 
	 
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
		elsif rising_edge(clock_clk) then
            enable_pin_input_ff <= enable_pin_input_ff(0) & enable_pin_input;
            pin_input_ff <= pin_input_ff(0) & pin_input;
		end if;
	end process;

	enable_pin_input_intern <= enable_pin_input_ff(1);
	pin_input_intern_re <= pin_input_ff(1) and not pin_input_ff(0) ;
	
	-- generate long pulse when input comes
   p_count_pin: process(all)
	begin
		if reset_reset ='0' then
            pin_counter <= (others=>'0');
				pin_counter_is_counting <= '0';
				
		elsif rising_edge(clock_clk) then
            if pin_counter_is_counting ='0' and pin_input_intern_re = '1' then
					pin_counter_is_counting <= '1';
				end if;
				
				if pin_counter_is_counting = '1' then
					
					if pin_counter < on_clocks then
						pin_counter <= pin_counter + 1;
					else
						pin_counter <= (others=>'0');
						pin_counter_is_counting <= '0';
					end if;
				end if;
		end if;
	end process;
	pulse_out_pin_in_intern <= '1' when pin_counter_is_counting else '0';

	pulse_out <= pulse_out_pin_in_intern when enable_pin_input_intern else pulse_out_counter_intern;
	
end architecture;
