

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;


entity diverses is
	generic (
		g_1 : integer := 19;
		g_2 : integer := 10000;
		g_3 : integer := 1024
	);
	port (
		   clk                    : in  std_logic                     := '0';
           reset                  : in  std_logic                     := '0';
           in_A                   : in  std_logic                     := '0';
           in_B                   : in  std_logic                     := '0';
           in_C                   : in std_logic;
           out_1                  : out std_logic;
           out_2                  : out std_logic
    );
end entity diverses;

architecture rtl of diverses is


	type state_main is (main_write, main_read);
	signal main_state         : state_main;

	type state_AVM_read is (idle, addr_delay, set_addr, wait_valid, wait_end, end_read);
	signal read_state         : state_AVM_read := idle;
	signal next_read_state    : state_AVM_read := idle;

	signal read_address       : unsigned(23 downto 0);
	signal read_n             : std_logic;

	signal read_data_count           : unsigned(8 downto 0):= (others=>'0');

	signal valid_ff       : std_logic_vector(1 downto 0);
	signal clock_counter_set_addr : unsigned(8 downto 0);
	signal clock_counter_wait_end : unsigned(8 downto 0);

	signal valid :std_logic;
	signal wait_request :std_logic;
	signal addr_ready :std_logic;
	signal reset_reset :std_logic;
	signal clock_clk :std_logic;


begin
	main_state<=main_read;
	valid <= in_B;
	addr_ready    <= in_A;
	clock_clk <= clk;
	reset_reset<= reset;
	wait_request <= in_C;


p_counter: process(all)
begin
	if reset_reset = '1' then
		clock_counter_set_addr <= (others => '0');
		clock_counter_wait_end <= (others => '0');

	elsif rising_edge(clock_clk) then
		if (read_state=set_addr or read_state=addr_delay) and wait_request='0'  then
			clock_counter_set_addr <= clock_counter_set_addr + 1;
		else
			clock_counter_set_addr <= (others => '0');
		end if;

		if read_state=wait_end then
			clock_counter_wait_end <= clock_counter_wait_end + 1;
		else
			clock_counter_wait_end <= (others => '0');
		end if;

	end if;
end process;

p_valid_ff: process(all)
begin
	if reset_reset = '1' then
		valid_ff <= (others=>'0');
	elsif rising_edge(clock_clk) then
		if read_state /= idle then
			valid_ff <= valid_ff(0) & valid;
		else
			valid_ff <= (others=>'0');
		end if;
	end if;
end process;

p_addr_inc: process(all)
begin
	if reset_reset = '1' then
		read_address <= (others=>'0');

	elsif rising_edge(clock_clk) then
		-- if read_state = set_addr and clock_counter_set_addr(0)='1' then
		if valid_ff="11" and valid='0' and read_state/=idle and read_state/=addr_delay then
			read_address <= read_address + 1;

		elsif addr_ready = '1' then
			read_address  <= (others=>'0');
		end if;

	end if;
end process;

p_count_data_access: process(all)
begin
	if reset_reset = '1' then
		read_data_count <= (others=>'0');

	elsif rising_edge(clock_clk) then
		-- if read_state=read then
			if valid_ff="11" then
				read_data_count <= read_data_count + 1;
			end if;
		-- else
			-- read_data_count <= (others=>'0');
		-- end if;

		if addr_ready = '1' then
			read_data_count <= (others=>'0');
		end if;
	end if;
end process;

p_AVM_Read_statemachine_clocked :process(all)
begin
	if reset_reset = '1' then
		read_state <= idle;
	elsif rising_edge(clock_clk) then
		read_state <= next_read_state;
	end if;
end process ;

p_state_machine_comb: process(all)
begin
	if main_state = main_read then
		case read_state is
		when idle =>
			if addr_ready <= '1' then
				-- next_read_state <= addr_delay;
				next_read_state <= set_addr;
			else
				next_read_state <= idle;
			end if;
		when addr_delay=>
			if valid_ff="11" then
				next_read_state <= set_addr;
			else
				next_read_state <= addr_delay;
			end if;
		when set_addr =>
			-- if clock_counter_set_addr=4 then
			if clock_counter_set_addr=1 then
				next_read_state <= wait_valid;
			else
				next_read_state <= set_addr;
			end if;
		when wait_valid =>
			if valid='1' then
				next_read_state <= wait_end;
			else
				next_read_state <= wait_valid;
			end if;

		when wait_end =>
			if read_address = 100 then
				next_read_state <= end_read;
			elsif clock_counter_wait_end=2 then
				next_read_state<= set_addr;
			else
				next_read_state<= wait_end;
			end if;

		when end_read =>
			next_read_state <= idle;

		end case;
	else
		next_read_state <= idle;
	end if;
	case read_state is
		when idle =>
			read_n <= '1';

		when addr_delay =>
			read_n <= '0';

		when set_addr =>
			read_n <= '0';

		when wait_valid =>
			read_n <= '0';

		when wait_end =>
			read_n <= '1';

		when end_read =>
			read_n <= '1';

	end case;

end process;


end architecture;


-- p_calculate_new_read_addr: process(all)
-- 	begin
-- 		if reset = '1' then
--
-- 		elsif rising_edge(clk) then
--
-- 		end if;
-- 	end process;

























