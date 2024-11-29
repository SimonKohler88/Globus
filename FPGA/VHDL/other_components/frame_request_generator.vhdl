

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;


entity frame_request_generator is
	generic (
		counter_bits : integer := 22;
		on_clocks : integer := 10000
	);
	port (
		clock_clk                   : in  std_logic                     := '0';             --                clock.clk
		reset_reset                 : in  std_logic                     := '0';
        pulse_out                   : out std_logic
    );
end entity frame_request_generator;

architecture rtl of frame_request_generator is

    signal counter :unsigned(counter_bits -1 downto 0);
begin

    p_count: process(all)
	begin
		if reset_reset ='0' then
            counter <= (others=>'0');
		elsif rising_edge(clock_clk) then
            counter <= counter + 1;
		end if;
	end process;

    pulse_out <= '1' when counter < on_clocks else '0';

end architecture;
