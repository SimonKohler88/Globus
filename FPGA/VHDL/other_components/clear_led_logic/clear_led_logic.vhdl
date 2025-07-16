

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;


entity clear_led_logic is
	generic (
		counter_bits : integer := 17
	);
	port (
		clock_clk                   : in  std_logic                     := '0';             --                clock.clk
		reset_reset                 : in  std_logic                     := '0';
		qspi_cs                     : in std_logic    := '0';
		clear_led_pulse             : out std_logic
    );
end entity clear_led_logic;

architecture rtl of clear_led_logic is

    signal counter :unsigned(counter_bits -1 downto 0);

    signal qspi_cs_ff :std_logic_vector(1 downto 0);
    signal qspi_cs_done :std_logic;


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
	
	-- double flopping in input signals
	p_in_flooping: process(all)
	begin
		if reset_reset ='0' then
            qspi_cs_ff <= (others=>'0');
		elsif rising_edge(clock_clk) then
            qspi_cs_ff <= qspi_cs_ff(0) & qspi_cs;
		end if;
	end process;

	p_logic: process(all)
	begin
		if reset_reset ='0' then
            qspi_cs_done <= '0';
		elsif rising_edge(clock_clk) then
            if qspi_cs_ff(1)='0' and qspi_cs_ff(0)='1' then
				qspi_cs_done <= '1';
            end if;
		end if;
	end process;

	p_output: process(all)
	begin
		if reset_reset ='0' then
            clear_led_pulse <= '0';
		elsif rising_edge(clock_clk) then
            if qspi_cs_done='0' and counter=0 then
				clear_led_pulse <= '1';
			else
				clear_led_pulse <= '0';
            end if;
		end if;
	end process;


end architecture;




























