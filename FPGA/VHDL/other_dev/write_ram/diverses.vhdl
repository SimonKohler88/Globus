

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;



--**********************************************************************************
--**********************************************************************************
--
--                  RAM WRITE
--
--**********************************************************************************
--**********************************************************************************

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
           in_D                   : in std_logic_vector(23 downto 0);
           out_1                  : out std_logic;
           out_2                  : out std_logic
    );
end entity diverses;

architecture rtl of diverses is

	constant BASE_ADDR_1      : unsigned(23 downto 0):= (others => '0');
	constant BASE_ADDR_2: unsigned(23 downto 0):=   X"020000";
	constant image_rows : integer := 120;
	constant image_cols_bits: integer:= 8;

	type state_main is (main_write, main_read);
	signal main_state         : state_main;

	signal clock_clk: std_ulogic;
	signal reset_reset : std_ulogic;
	signal asi_in0_data             : std_logic_vector(23 downto 0);
	signal asi_in0_ready            : std_ulogic;
	signal asi_in0_valid            : std_ulogic;
	signal asi_in0_endofpacket      : std_ulogic;
	signal asi_in0_startofpacket    : std_ulogic;

	signal avm_m0_address           : std_logic_vector(23 downto 0);
    signal avm_m0_read_n              : std_logic;
    signal avm_m0_waitrequest       : std_logic;
    signal avm_m0_readdata          : std_logic_vector(15 downto 0);
    signal avm_m0_readdatavalid     : std_logic;
    signal avm_m0_write_n             : std_logic;
    signal avm_m0_writedata         : std_logic_vector(15 downto 0);


	type state_AVM_write is (idle, wait_valid, write_1, write_2,write_3, end_write);
	signal write_state        : state_AVM_write := idle;
	signal next_write_state   : state_AVM_write := idle;

	signal end_packet_ff             : std_logic_vector(1 downto 0);
	signal start_packet_ff           : std_logic_vector(1 downto 0);
	signal addr_switch_pending       : std_logic;
	signal current_address_write     : unsigned(23 downto 0) := (others => '0');
	signal last_address_write     : unsigned(23 downto 0) := (others => '0');
	signal current_byteenable_write     : std_logic_vector(1 downto 0) := (others => '0');
	signal current_byteenable_write_b      : std_logic;
	signal current_chipselect_read   : std_logic;
	signal current_chipselect_write  : std_logic;

	signal incoming_pix_count     : unsigned(15 downto 0);
	constant image_cols :integer := 2**image_cols_bits;
	constant addr_row_to_row_offset  : integer := 4* 2**image_cols_bits;
	constant c_accept_eop		: integer:= image_rows*image_cols-100;
	signal incoming_transfer_ongoing :std_logic;
	signal SystemEnable                  : std_logic;

	signal active_base_addr   : std_logic;
	signal data_in_buffer     : std_logic_vector(23 downto 0) := (others => '0');

	signal write_address      : unsigned(23 downto 0) := (others => '0');

	signal write_1_ff :std_logic_vector(1 downto 0);
	signal write_2_ff :std_logic_vector(1 downto 0);
	signal write_3_ff :std_logic_vector(1 downto 0);

begin
	reset_reset<= reset;
	clock_clk <= clk;
	main_state<=main_write;
	avm_m0_waitrequest <= '0';

	asi_in0_data   <= in_D;
	out_1 <= asi_in0_ready;
	asi_in0_valid    <=       in_A;
	asi_in0_endofpacket <=    in_B;
	asi_in0_startofpacket<=   in_C;

	SystemEnable<='1';

	asi_in0_ready <= '1' when main_state=main_write and ( write_state = wait_valid ) and SystemEnable='1' else '0';

	--control input (but not state) of "write to ram" statemachine
	p_write_ram_clocked : PROCESS(all)
	BEGIN
		IF reset_reset = '1' THEN
			write_state <= idle; -- Zustand nach reset
			end_packet_ff <= (others=>'0');
			start_packet_ff <= (others=>'0');
			active_base_addr <= '0';
			data_in_buffer <= (others =>'0');
			addr_switch_pending <= '0';
			current_address_write <= BASE_ADDR_1;
			incoming_pix_count <= (others=>'0');
			incoming_transfer_ongoing <= '0';
			write_1_ff <= (others=>'0');
			write_2_ff <= (others=>'0');
			write_3_ff <= (others=>'0');

		ELSIF rising_edge(clock_clk)THEN
			write_state <= next_write_state;

			-- buffering data in. we dont know if we can write it already.
			if asi_in0_valid = '1' then
				data_in_buffer <= asi_in0_data;
				incoming_pix_count <= incoming_pix_count + 1;
			end if;

			-- check start->reset counter
			start_packet_ff <= start_packet_ff(0) & asi_in0_startofpacket;
			if start_packet_ff = "01" and incoming_transfer_ongoing='0' then
				incoming_pix_count <= (0 => '1', others =>'0'); -- set to 1
				incoming_transfer_ongoing <= '1';
			end if;

			-- remember end of packet -> switch to other buffer
			end_packet_ff <= end_packet_ff(0) & asi_in0_endofpacket;
			if end_packet_ff = "01" and incoming_transfer_ongoing='1' and incoming_pix_count > c_accept_eop then
				--c_accept_eop: lock against unwanted eop and sop--> fifo generates them for some reason
				active_base_addr <= not active_base_addr;
				addr_switch_pending <= '1';
				incoming_transfer_ongoing <= '0';
			end if;

			if write_state=write_1 and avm_m0_waitrequest='0' then
				write_1_ff <= write_1_ff(0) & '1';
			else
				write_1_ff <= (others=>'0');
			end if;

			if write_state=write_2 and avm_m0_waitrequest='0' then
				write_2_ff <= write_2_ff(0) & '1';
			else
				write_2_ff <= (others=>'0');
			end if;

			if write_state=write_3 and avm_m0_waitrequest='0' then
				write_3_ff <= write_3_ff(0) & '1';
			else
				write_3_ff <= (others=>'0');
			end if;

			-- switch to other buffer: set base address and increments of addr
			if addr_switch_pending='1' and (write_state=idle or write_state=wait_valid) then
				if active_base_addr='0' then
					current_address_write <= BASE_ADDR_1;
					last_address_write <= BASE_ADDR_1;
				else
					current_address_write <= BASE_ADDR_2;
					last_address_write <= BASE_ADDR_2;
				end if;
				addr_switch_pending <= '0';

			elsif (write_state=write_1 and next_write_state=write_2) or next_write_state=end_write then
			-- elsif next_write_state=write_2 or next_write_state=end_write then
				current_address_write <= current_address_write + 1;
				last_address_write <= current_address_write;
			end if;

		END IF;
	END PROCESS p_write_ram_clocked;

	  --Interface that communicates with RAM controller over Avalon-MM
    p_AVM_Write_statemachine : process(all) is
    begin

		if main_state = main_write then
			case write_state is
				when idle =>
					if main_state = main_write then
						next_write_state <= wait_valid;
					else
						next_write_state <= idle;
					end if;

				when wait_valid =>
					if asi_in0_valid='1' then
						next_write_state <= write_1;
					else
						next_write_state <= wait_valid;
					end if;

				when write_1 =>
					if avm_m0_waitrequest='0' and write_1_ff(0)='1' then
						next_write_state <= write_2;
					else
						next_write_state <= write_1;
					end if;

				when write_2 =>
					-- next_write_state <= end_write;

					if avm_m0_waitrequest='0' and write_2_ff(0)='1' then
						if last_address_write(1 downto 0)="00" then
							next_write_state <= write_3;
						else
							next_write_state <= end_write;
						end if;
					else
						next_write_state <= write_2;
					end if;

				when write_3 =>
					-- next_write_state <= end_write;

					if avm_m0_waitrequest='0' and write_3_ff(1)='1' then
						next_write_state <= end_write;
					else
						next_write_state <= write_3;
					end if;

				when end_write =>
					next_write_state <= idle;

				when others =>
					next_write_state <= idle;
			end case;

			case write_state is
				when idle => --Idle, setup_write, write_1, write_2, end_write
					write_address <= (others => '0');
					avm_m0_write_n <= '1';
					avm_m0_writedata <= (others => '0');

				when wait_valid =>
					write_address <=  (others => '0');
					avm_m0_write_n <= '1';
					avm_m0_writedata <= (others => '0');

				when write_1 =>
					write_address <=  current_address_write;
					avm_m0_write_n <= '0';
					avm_m0_writedata <= data_in_buffer(23 downto 8);

				when write_2 =>
					write_address <=  current_address_write;
					avm_m0_write_n <= '0';
					avm_m0_writedata(15 downto 0) <= data_in_buffer(7 downto 0) & X"00";

				when write_3 =>
					if write_3_ff(0)= '0' then
						avm_m0_write_n <= '1';
					else
						avm_m0_write_n <= '0';
					end if;
					write_address <=  last_address_write;
					avm_m0_writedata <= data_in_buffer(23 downto 8);

				when end_write =>
					write_address <= (others => '0');
					avm_m0_write_n <= '1';
					avm_m0_writedata <= (others => '0');

				when others =>
					write_address <= (others => '0');
					avm_m0_write_n <= '1';
					avm_m0_writedata <= (others => '0');
			end case;
		else
			avm_m0_write_n <= '1';
			write_address <= (others => '0');
			avm_m0_writedata <= (others => '0');
			next_write_state <= idle;

		end if;

    end process p_AVM_Write_statemachine;
	current_chipselect_write <= not avm_m0_write_n;
	current_byteenable_write_b <= '1' when (write_state=write_1 or write_state=write_2) and avm_m0_waitrequest='0' else '0';
end architecture;


-- p_calculate_new_read_addr: process(all)
-- 	begin
-- 		if reset = '1' then
--
-- 		elsif rising_edge(clk) then
--
-- 		end if;
-- 	end process;

























