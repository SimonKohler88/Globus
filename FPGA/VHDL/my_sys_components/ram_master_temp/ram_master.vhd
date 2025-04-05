-- ram_master.vhd

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

entity ram_master is
	generic (
		G_IMAGE_ROWS : integer := 120;
		G_IMAGE_COLS_BITS: integer:= 8;
		G_BASE_ADDR_2_OFFSET : unsigned  :=  X"040000"
	);
	port (
		clock_clk                : in  std_logic                     := '0';             --                clock.clk
		reset_reset              : in  std_logic                     := '0';             --                reset.reset
		conduit_ping_or_pong     : in  std_logic                     := '0';             -- conduit_ping_or_pong.new_signal

		avm_m0_address           : out std_logic_vector(23 downto 0);                    --               avm_m0.address
		avm_m0_read_n            : out std_logic;                                        --                     .read
		avm_m0_waitrequest       : in  std_logic                     := '0';             --                     .waitrequest
		avm_m0_readdata          : in  std_logic_vector(15 downto 0) := (others => '0'); --                     .readdata
		avm_m0_readdatavalid     : in  std_logic                     := '0';             --                     .readdatavalid
		avm_m0_write_n           : out std_logic;                                        --                     .write
		avm_m0_writedata         : out std_logic_vector(15 downto 0);                    --                     .writedata
		avm_m0_byteenable        : out std_logic_vector(1 downto 0);                     --                     .byteenable
		avm_m0_chipselect        : out std_logic;                                        --                     .chipselect

		asi_in0_data             : in  std_logic_vector(23 downto 0) := (others => '0'); --              asi_in0.data
		asi_in0_ready            : out std_logic;                                        --                     .ready
		asi_in0_valid            : in  std_logic                     := '0';             --                     .valid
		asi_in0_endofpacket      : in  std_logic                     := '0';             --                     .endofpacket
		asi_in0_startofpacket    : in  std_logic                     := '0';             --                     .startofpacket

		conduit_col_info_col_nr  : in  std_logic_vector(8 downto 0)  := (others => '0'); --     conduit_col_info.col_nr
		conduit_col_info_fire    : in  std_logic                     := '0';             --                     .fire

		aso_out1_B_data          : out std_logic_vector(23 downto 0);                    --           aso_out1_B.data
		aso_out1_B_endofpacket   : out std_logic;                                        --                     .endofpacket
		aso_out1_B_ready         : in  std_logic                     := '0';             --                     .ready
		aso_out1_B_startofpacket : out std_logic;                                        --                     .startofpacket
		aso_out1_B_valid         : out std_logic;                                        --                     .valid

		aso_out0_startofpacket_1 : out std_logic;                                        --           aso_out0_A.startofpacket
		aso_out0_endofpacket_1   : out std_logic;                                        --                     .endofpacket
		aso_out0_A_data          : out std_logic_vector(23 downto 0);                    --                     .data
		aso_out0_A_ready         : in  std_logic                     := '0';             --                     .ready
		aso_out0_A_valid         : out std_logic;                                        --                     .valid

		avs_s1_address           : in  std_logic_vector(7 downto 0)  := (others => '0'); --               avs_s1.address
		avs_s1_read              : in  std_logic                     := '0';             --                     .read
		avs_s1_readdata          : out std_logic_vector(31 downto 0);                    --                     .readdata
		avs_s1_write             : in  std_logic                     := '0';             --                     .write
		avs_s1_writedata         : in  std_logic_vector(31 downto 0) := (others => '0'); --                     .writedata
		avs_s1_waitrequest       : out std_logic;                                         --                     .waitrequest

		conduit_debug_ram_out    : out  std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_ram.ram_out
		conduit_debug_ram_out_2    : out  std_logic_vector(31 downto 0)  := (others => '0'); --     conduit_debug_ram.ram_out_2
		conduit_debug_ram_in     : in   std_logic_vector(31 downto 0)  := (others => '0') --                         .ram_in
	);
end entity ram_master;

architecture rtl of ram_master is

	signal SystemEnable                  : std_logic;

	type state_main_t is (main_write, main_read);
	signal main_state         : state_main_t;
	signal next_main_state    : state_main_t;

	type state_AVM_write_t is (idle, wait_valid, write_1, write_2, write_3, end_write);
	signal write_state        : state_AVM_write_t := idle;
	signal next_write_state   : state_AVM_write_t := idle;

	type state_AVM_read_t is (idle, set_addr, wait_waitrequest, wait_end, end_read);
	signal read_state         : state_AVM_read_t := idle;
	signal next_read_state    : state_AVM_read_t := idle;

	type state_AVSO_read_to_stream_t is (idle , read_to_stream, end_read);
	signal aso_state          : state_AVSO_read_to_stream_t := idle;
	signal next_aso_state     : state_AVSO_read_to_stream_t := idle;

	signal write_address      : unsigned(23 downto 0) := (others => '0');
	signal read_address       : unsigned(23 downto 0) := (others => '0');

	signal data_in_buffer     : std_logic_vector(23 downto 0) := (others => '0');
	signal data_out_buffer    : std_logic_vector(23 downto 0) := (others => '0');

	constant C_BASE_ADDR_1      : unsigned(23 downto 0):= (others => '0');

	-- 60'000 pixel need addr range of 120'000          (= 0x01E000)
	--constant C_BASE_ADDR_2: unsigned(23 downto 0):=   X"020000"; --TODO: this in real design
	constant C_BASE_ADDR_2: unsigned(23 downto 0):=   G_BASE_ADDR_2_OFFSET;
	-- constant C_BASE_ADDR_2      : unsigned(23 downto 0):=  X"001000"; -- only for testing
	signal active_base_addr   : std_logic;

	constant C_ADDR_ROW_TO_ROW_OFFSET  : integer := 4* 2**G_IMAGE_COLS_BITS;
	constant C_ADDR_B_COL_SHIFT_OFFSET : integer := 2**G_IMAGE_COLS_BITS/2;
	constant C_IMAGE_COLS :integer := 2**G_IMAGE_COLS_BITS;

	signal addr_ab_converter         : unsigned (G_IMAGE_COLS_BITS-1 downto 0);
	signal addr_ack                  : std_logic;
	signal addr_valid                : std_logic;
	signal addr_ready 				 : std_logic;
	signal addr_a_preload            : unsigned(23 downto 0) := (others => '0');
	signal addr_b_preload            : unsigned(23 downto 0) := (others => '0');
	signal addr_adder                : unsigned(23 downto 0) := (others => '0');

	signal end_packet_ff             : std_logic_vector(1 downto 0);
	signal start_packet_ff           : std_logic_vector(1 downto 0);
	signal addr_switch_pending       : std_logic;

	signal current_address_write     : unsigned(23 downto 0) := (others => '0');
	signal last_address_write     : unsigned(23 downto 0) := (others => '0');
	signal current_byteenable_write     : std_logic_vector(1 downto 0) := (others => '0');
	signal current_byteenable_read      : std_logic_vector(1 downto 0) := (others => '0');
	signal current_byteenable_read_b       : std_logic;
	signal current_byteenable_write_b      : std_logic;
	signal current_chipselect_read   : std_logic;
	signal current_chipselect_write  : std_logic;


	signal active_aso_startofpacket  : std_logic;
	signal active_aso_endofpacket    : std_logic;
	signal active_aso_data           : std_logic_vector(23 downto 0);
	signal active_aso_ready          : std_logic;
	signal active_aso_valid          : std_logic;

	signal aso_send_count            : unsigned(8 downto 0):= (others=>'0');
	signal read_data_count           : unsigned(8 downto 0):= (others=>'0');
	signal read_addr_count           : unsigned(8 downto 0):= (others=>'0');
	signal read_n_delay           : std_logic_vector(2 downto 0);

	signal col_fire_ff               : std_logic_vector(1 downto 0);
	signal fire_pending              : std_logic;
	signal read_in_progress			 : std_logic;

	signal ram_read_1_buffer         : std_logic_vector(15 downto 0);

	signal incoming_pix_count     : unsigned(15 downto 0);
	constant C_ACCEPT_EOP_CNT		: integer:= G_IMAGE_ROWS*C_IMAGE_COLS-100;
	signal incoming_transfer_ongoing :std_logic;

	type state_test_t is (none, t1,t2, t_fifo_check, t_col_nr, t_read_0);
	signal test_state         : state_test_t;

	signal test_pack_sig_stretch   :std_logic_vector(2 downto 0);
	signal test_pack_sig_stretch_2 :std_logic_vector(2 downto 0);
	signal test_pack_sig           :std_logic;
	signal test_pack_sig_2         :std_logic;

	signal valid_ff       : std_logic_vector(1 downto 0);
	signal clock_counter_set_addr : unsigned(8 downto 0);
	signal clock_counter_wait_end : unsigned(8 downto 0);
	signal read_n             : std_logic;
	signal valid :std_logic;
	signal wait_request :std_logic;

	signal write_1_ff :std_logic_vector(1 downto 0);
	signal write_2_ff :std_logic_vector(1 downto 0);
	signal write_3_ff :std_logic_vector(1 downto 0);

	signal aso_pix_send_count            : unsigned(7 downto 0):= (others=>'0');


begin
	test_state <= t_read_0;
	p_test: process(all)
	begin
		case test_state is
			when none =>
				conduit_debug_ram_out(31 downto 1) <= (others => '0');
				conduit_debug_ram_out(0) <= reset_reset;
				conduit_debug_ram_out_2(31 downto 0) <= (others => '0');

			when t1 =>
				if main_state=main_write and (avm_m0_address=X"000000" or avm_m0_address=std_logic_vector(C_BASE_ADDR_2)) and avm_m0_write_n='0' then
					conduit_debug_ram_out_2(0) <= '1';
				else
					conduit_debug_ram_out_2(0) <= '0';
				end if;

				conduit_debug_ram_out(15 downto 0) <= avm_m0_writedata;
				conduit_debug_ram_out(31 downto 16) <= (others => '0');
				conduit_debug_ram_out_2 <= (
					1 => avm_m0_read_n,

					26 => avm_m0_waitrequest,
					27 => avm_m0_write_n,
					28 => active_base_addr,
					others => '0'
				);
				conduit_debug_ram_out_2(25 downto 2) <= avm_m0_address;

			when t2 =>
				conduit_debug_ram_out_2 <= (others => '0');
				conduit_debug_ram_out_2(0) <= test_pack_sig;

				conduit_debug_ram_out(15 downto 0) <= avm_m0_writedata;
				conduit_debug_ram_out(31 downto 16) <= (others => '0');
				conduit_debug_ram_out_2(1) <= avm_m0_read_n;
				conduit_debug_ram_out_2(25 downto 2) <= avm_m0_address;
				conduit_debug_ram_out_2(26) <= avm_m0_waitrequest;
				conduit_debug_ram_out_2(27) <= avm_m0_write_n;
				conduit_debug_ram_out_2(28) <= active_base_addr;
				conduit_debug_ram_out_2(29) <= test_pack_sig_2;



			when t_fifo_check =>
				conduit_debug_ram_out_2(31 downto 0) <= (others => '0');

				conduit_debug_ram_out <= (
					24 => test_pack_sig, -- sop
					25 => test_pack_sig_2, -- eop
					26 => asi_in0_valid,
					27 => asi_in0_ready,
					others => '0'
				);
 				conduit_debug_ram_out(23 downto 0) <= asi_in0_data;

			when t_col_nr =>
				conduit_debug_ram_out(31 downto 9) <= (others => '0');
				conduit_debug_ram_out(8 downto 0) <= conduit_col_info_col_nr;
				conduit_debug_ram_out_2(31 downto 1) <= (others => '0');
				conduit_debug_ram_out_2(0) <= conduit_col_info_fire;

			when t_read_0 =>
				-- if main_state=main_read_A and (avm_m0_address=X"000000" or avm_m0_address=std_logic_vector(C_BASE_ADDR_2)) and avm_m0_read_n='0' then
				-- 	conduit_debug_ram_out_2(0) <= '1';
				-- else
				-- 	conduit_debug_ram_out_2(0) <= '0';
				-- end if;

				-- conduit_debug_ram_out_2(0) <= fire_pending;
				conduit_debug_ram_out_2(0) <= fire_pending or test_pack_sig;

				if main_state=main_write then
					conduit_debug_ram_out(15 downto 0) <= avm_m0_writedata ;
					conduit_debug_ram_out_2(1) <= avm_m0_write_n;
				else
					conduit_debug_ram_out(15 downto 0) <= avm_m0_readdata ;
					conduit_debug_ram_out_2(1) <= avm_m0_read_n;
				end if;
				conduit_debug_ram_out(31 downto 16) <= (others => '0');

				conduit_debug_ram_out_2(25 downto 2) <= avm_m0_address;
				conduit_debug_ram_out_2(26) <= avm_m0_waitrequest;
				conduit_debug_ram_out_2(27) <= avm_m0_readdatavalid;
				conduit_debug_ram_out_2(31 downto 28) <= conduit_col_info_col_nr(3 downto 0);

			when others =>
				conduit_debug_ram_out(31 downto 0) <= (others => '0');
				conduit_debug_ram_out_2(31 downto 0) <= (others => '0');

		end case;
	end process;

	--debug process
	p_debug: process(all)
	begin
	 if reset_reset = '1' then
            test_pack_sig_stretch     <= (others => '0');
            test_pack_sig_stretch_2   <= (others => '0');
        elsif rising_edge(clock_clk) then
           test_pack_sig_stretch   <= test_pack_sig_stretch(1 downto 0)   & asi_in0_startofpacket;
           test_pack_sig_stretch_2 <= test_pack_sig_stretch_2(1 downto 0) & asi_in0_endofpacket;
        end if;
	end process;

	test_pack_sig <=   test_pack_sig_stretch(2) or test_pack_sig_stretch(1) or test_pack_sig_stretch(0);
	test_pack_sig_2 <= test_pack_sig_stretch_2(2) or test_pack_sig_stretch_2(1) or test_pack_sig_stretch_2(0);

	aso_pix_send_count <= aso_send_count(8 downto 1);

	avs_s1_waitrequest <= '1'; -- not implemented
	avs_s1_readdata <= (others => '0');

	avm_m0_address <= std_logic_vector(write_address)  when main_state = main_write  else
					  std_logic_vector(read_address)  when main_state = main_read else
					  (others=>'0');

	avm_m0_chipselect <= current_chipselect_write when main_state = main_write  else
						 current_chipselect_read when main_state = main_read else
					     '0';

	-- avm_m0_byteenable <= current_byteenable_write when main_state = main_write else
	-- 					current_byteenable_read when main_state = main_read else "00";

	-- avm_m0_byteenable <= "11" when ((current_byteenable_write_b='1' and main_state=main_write) or(current_byteenable_read_b='1' and main_state=main_read))
	-- 					 else "00";

	avm_m0_byteenable <= "11" ;
	read_in_progress <= '1' when read_state/=idle or aso_state/=idle else '0';


 --Startup Delay for the RAM Controller
    InitialDelay : process(all) is
        variable delayCount : unsigned(9 downto 0);
    begin
        if reset_reset = '1' then
            delayCount   := (others => '1');
            SystemEnable <= '0';
        elsif rising_edge(clock_clk) then
            if delayCount = 0 then
                SystemEnable <= '1';
            else
                SystemEnable <= '0';
                delayCount   := delayCount - 1;
            end if;
        end if;
    end process InitialDelay;


	--*************************************** Main State Machine ***************************************************************

	p_main_state_clocked :process(all)
	begin
		if reset_reset = '1' then
		main_state <= main_write;
		col_fire_ff <= (others => '0');
		fire_pending <= '0';

		elsif rising_edge(clock_clk) then
			main_state <= next_main_state;

			col_fire_ff <= col_fire_ff(0) & conduit_col_info_fire;

			if col_fire_ff = "01" then
				fire_pending <= '1';
			end if;

			if main_state = main_read then
				fire_pending <= '0';
			end if;

		end if;
	end process p_main_state_clocked;

	p_main_state_statemachine: process(all)
	begin

		case main_state is
			when main_write =>
				if fire_pending='1' and (write_state=idle or write_state=wait_valid) then
					next_main_state <= main_read;
				else
					next_main_state <= main_write;
				end if;

			when main_read =>
				if aso_state=end_read then
					next_main_state <= main_write;
				else
					next_main_state <= main_read;
				end if;
			when others =>
				next_main_state <= main_write;
		end case;
	end process p_main_state_statemachine;

	--*************************************** Read from ram processes ***************************************************************
	-- general procedure:
	-- 1. on firepulse, calculate new address for stream a and stream b.
	--     (stream A: LEDs AB, first pixel at current column first row, then every second row in same column --> uneven rows)
	--     (stream B: LEDs CD, first pixel at current col + columns/2 (wraparound!) second row, then every second in this column -->even rows)
	--     read_state is in wait_addr_calculated
	-- 2. in read:
	--      -when not waitrequest, increase address. count every increase. if an even increase, adddr+1, else addr + 2 rows.
	--      -when data valid, count every data read.
	--                    on even access: put on outstream(23 to 8), no valid out.
	--                    on uneven access: put data(15 to 8) on outstream(7 to 0), give valid to current_aso
	--                    when count(len to 1) == 0 -> startpacket aso A, data to stream A
	--                    when count(len to 1) == 59 -> endpacket aso A
	--                    when count(len to 1) == 60 -> startpacket aso B, data to stream B
	--                    when count(len to 1) == 119 -> endpacket aso B, readstate to idle
	--
 --

-- ****************************************************************************************
--                            Setup adress for Ram Processes
-- ****************************************************************************************
-- compability renamings
valid <= avm_m0_readdatavalid;
current_chipselect_read <= not avm_m0_read_n;
avm_m0_read_n <= read_n when main_state=main_read else '1';
wait_request <= avm_m0_waitrequest;

-- Change Memorylocation to read from
addr_adder <= C_BASE_ADDR_1 when active_base_addr='1' else C_BASE_ADDR_2;

p_calculate_new_read_addr: process(all)
	variable stage : natural range 0 to 5;
begin
	if reset_reset = '1' then
		addr_ab_converter <= (others=>'0');
		addr_a_preload <= (others => '0');
		addr_b_preload <= (others => '0');
		stage := 0;

		addr_ready <= '0';

	elsif rising_edge(clock_clk) then
		addr_ready <= '0';
		if conduit_col_info_fire='1' and stage=0 and read_state=idle then --TODO
			addr_a_preload <= (others => '0');
			addr_b_preload <= (others => '0');
			addr_ab_converter <= (others => '0');

			stage := 1;

		elsif stage=1 then

			addr_a_preload(9 downto 1) <= unsigned(conduit_col_info_col_nr); -- factor 2 because every pix need 2 addresses
			addr_ab_converter <= unsigned(conduit_col_info_col_nr( G_IMAGE_COLS_BITS-1 downto 0 ));-- convert to unsigned

			stage:=2;
		elsif stage=2 then
			addr_a_preload <= addr_a_preload + addr_adder;
			addr_b_preload(G_IMAGE_COLS_BITS downto 0) <= (addr_ab_converter + C_ADDR_B_COL_SHIFT_OFFSET) & "0"; -- let wrap around and shift for *2
			addr_ready <= '1';

			stage:=3;

		elsif stage=3 then
			addr_b_preload <= addr_b_preload + addr_adder + C_IMAGE_COLS + C_IMAGE_COLS;

			stage := 0;
		else
			addr_ready <= '0';
		end if;

	end if;
end process p_calculate_new_read_addr;

p_counter: process(all)
begin
	if reset_reset = '1' then
		clock_counter_set_addr <= (others => '0');
		clock_counter_wait_end <= (others => '0');

	elsif rising_edge(clock_clk) then
		if read_state=set_addr  and wait_request='0'  then
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
		read_data_count <= (others=>'0');

	elsif rising_edge(clock_clk) then

		if wait_request = '0' and read_state /=idle then
			if read_data_count(0) = '0' then
				read_address <= read_address + 1;
			else
				if read_data_count = G_IMAGE_ROWS-1 then
					read_address <= addr_b_preload;
				else
					read_address <= read_address + C_ADDR_ROW_TO_ROW_OFFSET - 1;
				end if;
			end if;

			read_data_count <= read_data_count + 1;

		elsif addr_ready = '1' then
			read_data_count <= (others=>'0');
			read_address <= addr_a_preload;
		end if;
	end if;
end process;


p_Read_statemachine_clocked :process(all)
begin
	if reset_reset = '1' then
		read_state <= idle;
		aso_state <= idle;
	elsif rising_edge(clock_clk) then
		read_state <= next_read_state;
		aso_state <= next_aso_state;
	end if;
end process ;

p_AVM_read_state_machine_comb: process(all)
begin
	if main_state = main_read then
		case read_state is
		when idle =>
			if addr_ready <= '1' then
				next_read_state <= set_addr;
			else
				next_read_state <= idle;
			end if;
		when set_addr =>
			if clock_counter_set_addr=1 then
				next_read_state <= wait_waitrequest;
			else
				next_read_state <= set_addr;
			end if;
		when wait_waitrequest =>
			if wait_request='1' then
				next_read_state <= wait_end;
			else
				next_read_state <= wait_waitrequest;
			end if;

		when wait_end =>
			if read_data_count = 240 then
				next_read_state <= end_read;
			elsif clock_counter_wait_end=0 then
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
		when idle | end_read=>
			if aso_state /= read_to_stream then
				read_n <= '1';
			else
				read_n <= '0';
			end if;
		when set_addr =>
			read_n <= '0';
		when wait_waitrequest  =>
			read_n <= '0';
		when wait_end =>
			-- read_n <= '1';
			read_n <= '0';
		-- when end_read =>
		-- 	-- read_n <= '1';
		-- 	if aso_state /= read_to_stream then
		-- 		read_n <= '1';
		-- 	else
		-- 		read_n <= '0';
		-- 	end if;

	end case;
end process;

-- read_n <= '0';

-- ****************************************************************************************
--                            Read Data from Ram
-- ****************************************************************************************

p_aso_send_pix_count: process(all)
begin
if reset_reset ='1' then
	aso_send_count <= (others=>'0');
elsif rising_edge(clock_clk) then

	if main_state=main_read then
		if valid = '1' then
			aso_send_count <= aso_send_count + 1;
		elsif aso_state=end_read then
			aso_send_count <= (others=>'0');
		end if;
	end if;

end if;
end process;

p_data_access: process(all)
begin
if reset_reset = '1' then
	active_aso_data <= (others=>'0');
elsif rising_edge(clock_clk) then

	if main_state=main_read then
		if valid='1' then
			if aso_send_count(0)='0' then
				active_aso_data(23 downto 8) <= avm_m0_readdata;
			elsif aso_send_count(0)='1' then
				active_aso_data(7 downto 0) <= avm_m0_readdata(15 downto 8);
			end if;
		end if;
	else
		active_aso_data <= (others=>'0');
	end if;

end if;
end process;

p_valid_out_signal :process(all)
begin
	if reset_reset = '1' then
		active_aso_valid <= '0';
	elsif rising_edge(clock_clk) then
		if main_state=main_read and aso_send_count(0)='0' and valid_ff(0)='1' then
			active_aso_valid <= '1';
		else
			active_aso_valid <= '0';
		end if;
	end if;
end process;


p_packet_sop_eop_signal :process(all)
begin
	if reset_reset = '1' then
			aso_out0_startofpacket_1 <= '0';
			aso_out0_endofpacket_1   <= '0';
			aso_out1_B_startofpacket <= '0';
			aso_out1_B_endofpacket <= '0';
			addr_ack <= '0';

	elsif rising_edge(clock_clk) then
		addr_ack <= '0';

		aso_out0_startofpacket_1 <= '0';
		aso_out0_endofpacket_1   <= '0';
		aso_out1_B_startofpacket <= '0';
		aso_out1_B_endofpacket <= '0';

		if main_state=main_read and aso_send_count(0)='0' and valid_ff(0)='1'then
			-- aso_send_count counts valid's --> 2  valid's per pix
			if aso_send_count = 2 then
				aso_out0_startofpacket_1 <= '1';
			elsif aso_send_count = 120 then
				aso_out0_endofpacket_1   <= '1';
			elsif aso_send_count = 122 then
				aso_out1_B_startofpacket <= '1';
			elsif aso_send_count = 240 then
				addr_ack <= '1';
				aso_out1_B_endofpacket <= '1';
			end if;
		end if;
	end if;
end process;

p_aso_state_machine_comb: process(all)
begin
case aso_state is
	when idle =>
		if main_state=main_read then
			next_aso_state <= read_to_stream;
		else
			next_aso_state <= idle;
		end if;
	when read_to_stream =>
		if aso_send_count= 240 and active_aso_valid='1' then
			next_aso_state <= end_read;
		else
			next_aso_state <= read_to_stream;
		end if;

	when end_read =>
		next_aso_state <= idle;

end case;


-- handle data to led-interface
case aso_state is
	when read_to_stream =>
		if main_state=main_read and aso_send_count <= G_IMAGE_ROWS then
			aso_out0_A_data   <= active_aso_data;
			aso_out0_A_valid  <=  active_aso_valid;
			active_aso_ready  <= aso_out0_A_ready;
			aso_out1_B_data   <= (others =>'0');
			aso_out1_B_valid         <= '0';
		else
			aso_out1_B_data   <= active_aso_data;
			aso_out1_B_valid  <=  active_aso_valid;
			active_aso_ready  <= aso_out1_B_ready;
			aso_out0_A_data   <= (others =>'0');
			aso_out0_A_valid  <= '0';
		end if;
	when others =>
		aso_out1_B_data   <= (others =>'0');
		aso_out1_B_valid  <=  '0';
		aso_out0_A_data   <= (others =>'0');
		aso_out0_A_valid  <= '0';
end case;

end process;



--*************************************** Write to ram processes ***************************************************************

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
		current_address_write <= C_BASE_ADDR_1;
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
		if end_packet_ff = "01" and incoming_transfer_ongoing='1' and incoming_pix_count > C_ACCEPT_EOP_CNT then
			--C_ACCEPT_EOP_CNT: lock against unwanted eop and sop--> fifo generates them for some reason
			active_base_addr <= not active_base_addr;
			addr_switch_pending <= '1';
			incoming_transfer_ongoing <= '0';
		end if;

		if write_state=write_1 then
			write_1_ff <= write_1_ff(0) & '1';
		else
			write_1_ff <= (others=>'0');
		end if;

		if write_state=write_2 then
			write_2_ff <= write_2_ff(0) & '1';
		else
			write_2_ff <= (others=>'0');
		end if;

		if write_state=write_3 then
			write_3_ff <= write_3_ff(0) & '1';
		else
			write_3_ff <= (others=>'0');
		end if;

		-- switch to other buffer: set base address and increments of addr
		if addr_switch_pending='1' and (write_state=idle or write_state=wait_valid) then
			if active_base_addr='0' then
				current_address_write <= C_BASE_ADDR_1;
				last_address_write <= C_BASE_ADDR_1;
			else
				current_address_write <= C_BASE_ADDR_2;
				last_address_write <= C_BASE_ADDR_2;
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
				-- leave data for 2 clock cycles on bus
				if avm_m0_waitrequest='0' and write_1_ff(0)='1' then
					next_write_state <= write_2;
				else
					next_write_state <= write_1;
				end if;

			when write_2 =>
				-- leave data for 2 clock cycles on bus
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
				-- had problems with addresses 0, 4, 8, ...
				-- if one occurs, write it again
				-- altough it was most likely a clock-timing issue, leave it in
				if avm_m0_waitrequest='0' and write_3_ff(0)='1' then
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





































end architecture rtl; -- of ram_master
