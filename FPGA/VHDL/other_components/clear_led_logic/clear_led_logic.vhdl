

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;


entity clear_led_logic is
	generic (
		G_COUNTER_BITS: integer := 17;
		G_TIMEOUT_COUNTER_BITS: integer := 15

	);
	port (
		clock_clk                   : in  std_logic                     := '0';             --                clock.clk
		reset_reset                 : in  std_logic                     := '0';
		qspi_cs                     : in std_logic    := '0';
		clear_led_pulse             : out std_logic;
		clear_led_prohibit_fire     : out std_logic
    );
end entity clear_led_logic;

architecture rtl of clear_led_logic is

    signal counter :unsigned(G_COUNTER_BITS -1 downto 0);
    signal deadtime_counter :unsigned(G_TIMEOUT_COUNTER_BITS -1 downto 0);

    signal qspi_cs_ff :std_logic_vector(1 downto 0);
    signal qspi_cs_done_ff :std_logic_vector(5 downto 0);
    signal qspi_cs_done :std_logic;


begin
	clear_led_prohibit_fire <= not qspi_cs_done;
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

	p_edge_deadtime_counter: process(all)
	begin
		if reset_reset ='0' then
            deadtime_counter <= (others=>'0');
		elsif rising_edge(clock_clk) then
			if qspi_cs_ff(1)='0' and qspi_cs_ff(0)='1' then
				deadtime_counter <= (0=>'1', others=>'0');
			end if;

			if deadtime_counter > 0 then
				deadtime_counter <= deadtime_counter + 1;
			end if;

		end if;
	end process;

	p_logic: process(all)
	begin
		if reset_reset ='0' then
            qspi_cs_done <= '0';
            qspi_cs_done_ff <= (others=>'0');
		elsif rising_edge(clock_clk) then
            if qspi_cs_ff(1)='0' and qspi_cs_ff(0)='1' and deadtime_counter=0 then
				qspi_cs_done_ff <= qspi_cs_done_ff(4 downto 0) & '1';
            end if;

			if qspi_cs_done_ff(5) = '1' then
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




























