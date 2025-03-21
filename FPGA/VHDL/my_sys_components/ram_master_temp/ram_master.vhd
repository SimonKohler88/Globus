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
		image_rows : integer := 120;
		image_cols_bits: integer:= 8;
		BASE_ADDR_2_OFFSET : unsigned  :=  X"020000"
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

	type state_main is (main_write, main_read);
	signal main_state         : state_main;
	signal next_main_state    : state_main;

	type state_AVM_write is (idle, wait_valid, write_1, write_2, end_write);
	signal write_state        : state_AVM_write := idle;
	signal next_write_state   : state_AVM_write := idle;

	type state_AVM_read is (idle, wait_addr_calculated, read, end_read);
	signal read_state         : state_AVM_read := idle;
	signal next_read_state    : state_AVM_read := idle;

	signal write_address      : unsigned(23 downto 0) := (others => '0');
	signal read_address       : unsigned(23 downto 0) := (others => '0');

	signal data_in_buffer     : std_logic_vector(23 downto 0) := (others => '0');
	signal data_out_buffer    : std_logic_vector(23 downto 0) := (others => '0');

	constant BASE_ADDR_1      : unsigned(23 downto 0):= (others => '0');

	-- 60'000 pixel need addr range of 120'000          (= 0x01E000)
	--constant BASE_ADDR_2: unsigned(23 downto 0):=   X"020000"; --TODO: this in real design
	constant BASE_ADDR_2: unsigned(23 downto 0):=   BASE_ADDR_2_OFFSET;
	-- constant BASE_ADDR_2      : unsigned(23 downto 0):=  X"001000"; -- only for testing
	signal active_base_addr   : std_logic;

	constant addr_row_to_row_offset  : integer := 4* 2**image_cols_bits;
	constant addr_b_col_shift_offset : integer := 2**image_cols_bits/2;
	constant image_cols :integer := 2**image_cols_bits;

	signal addr_ab_converter         : unsigned (image_cols_bits-1 downto 0);
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

	signal read_data_count           : unsigned(8 downto 0):= (others=>'0');
	signal read_addr_count           : unsigned(8 downto 0):= (others=>'0');
	signal read_n_delay           : std_logic_vector(2 downto 0);

	signal col_fire_ff               : std_logic_vector(1 downto 0);
	signal fire_pending              : std_logic;
	signal read_in_progress			 : std_logic;

	signal ram_read_1_buffer         : std_logic_vector(15 downto 0);

	signal incoming_pix_count     : unsigned(15 downto 0);
	constant c_accept_eop		: integer:= image_rows*image_cols-100;
	signal incoming_transfer_ongoing :std_logic;

	type state_test is (none, t1,t2, t_data_in, t_col_nr, t_read_0);
	signal test_state         : state_test;

	signal test_pack_sig_stretch   :std_logic_vector(2 downto 0);
	signal test_pack_sig_stretch_2 :std_logic_vector(2 downto 0);
	signal test_pack_sig           :std_logic;
	signal test_pack_sig_2         :std_logic;


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
				if main_state=main_write and (avm_m0_address=X"000000" or avm_m0_address=std_logic_vector(BASE_ADDR_2)) and avm_m0_write_n='0' then
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
				conduit_debug_ram_out_2(0) <= test_pack_sig;

				conduit_debug_ram_out(15 downto 0) <= avm_m0_writedata;
				conduit_debug_ram_out(31 downto 16) <= (others => '0');
				conduit_debug_ram_out_2(1) <= avm_m0_read_n;
				conduit_debug_ram_out_2(25 downto 2) <= avm_m0_address;
				conduit_debug_ram_out_2(26) <= avm_m0_waitrequest;
				conduit_debug_ram_out_2(27) <= avm_m0_write_n;
				conduit_debug_ram_out_2(28) <= active_base_addr;
				conduit_debug_ram_out_2(29) <= test_pack_sig_2;
				conduit_debug_ram_out_2(31 downto 30) <= (others => '0');



			when t_data_in =>
				conduit_debug_ram_out(23 downto 0) <= asi_in0_data;
				conduit_debug_ram_out(31 downto 24) <= (others => '0');

				conduit_debug_ram_out_2(0) <= test_pack_sig;
				conduit_debug_ram_out_2(1) <= asi_in0_valid;
				conduit_debug_ram_out_2(2) <= test_pack_sig_2;
				conduit_debug_ram_out_2(3) <= asi_in0_ready;

				conduit_debug_ram_out_2(31 downto 4) <= (others => '0');


			when t_col_nr =>
				conduit_debug_ram_out(31 downto 9) <= (others => '0');
				conduit_debug_ram_out(8 downto 0) <= conduit_col_info_col_nr;
				conduit_debug_ram_out_2(31 downto 1) <= (others => '0');
				conduit_debug_ram_out_2(0) <= conduit_col_info_fire;

			when t_read_0 =>
				-- if main_state=main_read_A and (avm_m0_address=X"000000" or avm_m0_address=std_logic_vector(BASE_ADDR_2)) and avm_m0_read_n='0' then
				-- 	conduit_debug_ram_out_2(0) <= '1';
				-- else
				-- 	conduit_debug_ram_out_2(0) <= '0';
				-- end if;

				conduit_debug_ram_out_2(0) <= fire_pending;


				conduit_debug_ram_out(15 downto 0) <= avm_m0_readdata;
				conduit_debug_ram_out(31 downto 16) <= (others => '0');

				conduit_debug_ram_out_2(1) <= avm_m0_read_n;
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
				if fire_pending='1' and (write_state=idle or write_state=wait_valid) and addr_valid='1' then
					next_main_state <= main_read;
				else
					next_main_state <= main_write;
				end if;

			when main_read =>
				if read_state = end_read then
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

	addr_adder <= BASE_ADDR_1 when active_base_addr='1' else BASE_ADDR_2;

	read_in_progress <= '1' when main_state=main_read and read_state=read else '0';

	p_calculate_new_read_addr: process(all)
	variable stage : natural range 0 to 5;
	begin
		if reset_reset = '1' then
			addr_ab_converter <= (others=>'0');
			addr_a_preload <= (others => '0');
			addr_b_preload <= (others => '0');
			stage := 0;

			addr_valid <= '0';
			addr_ready <= '0';

		elsif rising_edge(clock_clk) then
			addr_ready <= '0';
			if conduit_col_info_fire='1' and stage=0 and read_in_progress='0' then
				addr_a_preload <= (others => '0');
				addr_b_preload <= (others => '0');
				addr_ab_converter <= (others => '0');

				addr_valid <= '0';
				stage := 1;

			elsif stage=1 then

				addr_a_preload(9 downto 1) <= unsigned(conduit_col_info_col_nr); -- factor 2 because every pix need 2 addresses
				addr_ab_converter <= unsigned(conduit_col_info_col_nr( image_cols_bits-1 downto 0 ));-- convert to unsigned

				addr_valid <= '0';
				stage:=2;
			elsif stage=2 then
				addr_a_preload <= addr_a_preload + addr_adder;
				addr_b_preload(image_cols_bits downto 0) <= (addr_ab_converter + addr_b_col_shift_offset) & "0"; -- let wrap around and shift for *2

				addr_valid <= '0';
				stage:=3;

			elsif stage=3 then
				addr_b_preload <= addr_b_preload + addr_adder + image_cols + image_cols;

				addr_ready <= '1';
				addr_valid <= '0';
				stage := 0;
			else
				addr_ready <= '0';
				addr_valid <= '1';
			end if;

			if addr_ack='1' then
				addr_valid <= '0';
			end if;
		end if;
	end process p_calculate_new_read_addr;


	p_count_address_access: process(all)
	begin
		if reset_reset = '1' then
			read_addr_count <= (others=>'0');

		elsif rising_edge(clock_clk) then
			if conduit_col_info_fire='1' and read_in_progress='0' then
				read_addr_count <= (others=>'0');

			elsif main_state=main_read and read_state=read then
				if avm_m0_waitrequest= '0'  then
					read_addr_count <= read_addr_count + 1;
				end if;
			end if;
		end if;
	end process ;

	p_update_addr: process(all)

	begin
		if reset_reset = '1' then
		read_address <= (others=>'0');

		elsif rising_edge(clock_clk) then

			if addr_ready='1' then
				read_address <= addr_a_preload;
			end if;

			if main_state=main_read and avm_m0_read_n='0' then
				if avm_m0_waitrequest = '0' then
					if read_addr_count(0) = '0' then
						read_address <= read_address + 1;
					else
						if read_addr_count = image_rows-1 then
							read_address <= addr_b_preload;
						else
							read_address <= read_address + addr_row_to_row_offset - 1;
						end if;
					end if;
				end if;
			end if;
		end if;
	end process p_update_addr;


	p_count_data_access: process(all)
	begin
		if reset_reset = '1' then
			read_data_count <= (others=>'0');

		elsif rising_edge(clock_clk) then
			if read_state=read then
				if avm_m0_readdatavalid then
					read_data_count <= read_data_count + 1;
				end if;
			else
				read_data_count <= (others=>'0');
			end if;
		end if;
	end process;

	avm_m0_read_n <= '0' when main_state=main_read and read_state=read else '1';
	current_chipselect_read <= not avm_m0_read_n;

	-- p_read_signal: process(all)
	-- begin
	-- if reset_reset = '1' then
	-- 	read_n_delay <= (others=> '0');
	-- elsif rising_edge(clock_clk) then
	-- 	read_n_delay <= read_n_delay(1 downto 0) & avm_m0_readdatavalid;
 --
	-- 	avm_m0_read_n <= '1';
	-- 	if main_state=main_read and read_state=read then
	-- 		if read_n_delay="001" and avm_m0_readdatavalid='1' then
	-- 			avm_m0_read_n <= '1';
	-- 		else
	-- 			avm_m0_read_n <= '0';
	-- 		end if;
	-- 	end if;
	-- 	current_chipselect_read <= not avm_m0_read_n;
	-- end if;
	-- end process;


	p_data_access: process(all)
	begin
	if reset_reset = '1' then
		active_aso_data <= (others=>'0');
	elsif rising_edge(clock_clk) then
		if read_state=read then
			-- due to clocked its one late --> read first part of pixel on '1'
			if read_data_count(0)='1' then
				active_aso_data(7 downto 0) <= avm_m0_readdata(15 downto 8);
			else
				active_aso_data(23 downto 8) <= avm_m0_readdata;
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
			if read_state=read and read_data_count(0)='1' and avm_m0_readdatavalid='1' then
				active_aso_valid <= '1';
			else
				active_aso_valid <= '0';
			end if;
		end if;
	end process ;

	p_AVM_Read_statemachine_clocked :process(all)
	begin
		if reset_reset = '1' then
			read_state <= idle;
		elsif rising_edge(clock_clk) then
			read_state <= next_read_state;

			if main_state = main_read then
			case read_state is
				when idle =>
					if main_state = main_read then
						next_read_state <= wait_addr_calculated;
					else
						next_read_state <= idle;
					end if;

				when wait_addr_calculated =>
					if addr_valid = '1' then
						next_read_state <= read;
					else
						next_read_state <= wait_addr_calculated;
					end if;

				when read =>
					if read_data_count(read_data_count'length-1 downto 1) = image_rows then
						next_read_state <= end_read;
					else
						next_read_state <= read;
					end if;

				when end_read =>
					next_read_state <= idle;

				when others =>
					next_read_state <= idle;
			end case;
			end if;
		end if;
	end process ;

	p_AVM_Read_output_mux: process(all)
    begin


		if main_state=main_read then
			case read_state is
				when idle =>
					active_aso_ready <= aso_out0_A_ready;

					aso_out0_A_data          <= (others =>'0');
					aso_out1_B_data          <= (others =>'0');
					aso_out0_A_valid         <= '0';
					aso_out1_B_valid         <= '0';

					aso_out0_startofpacket_1 <= '0';
					aso_out1_B_startofpacket <= '0';
					aso_out0_endofpacket_1   <= '0';
					aso_out1_B_endofpacket   <= '0';
					addr_ack <= '0';

				when wait_addr_calculated =>
					active_aso_ready <= aso_out0_A_ready;

					aso_out0_A_data          <= (others =>'0');
					aso_out1_B_data          <= (others =>'0');
					aso_out0_A_valid         <= '0';
					aso_out1_B_valid         <= '0';

					aso_out0_startofpacket_1 <= '0';
					aso_out1_B_startofpacket <= '0';
					aso_out0_endofpacket_1   <= '0';
					aso_out1_B_endofpacket   <= '0';
					addr_ack <= '0';

				when read =>
					if main_state=main_read and read_data_count(read_data_count'length-1 downto 1) < image_rows/2 + 1 then
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

					if active_aso_valid='1' and read_data_count(read_data_count'length-1 downto 1)=1 then
						aso_out0_startofpacket_1 <= '1';
					else
						aso_out0_startofpacket_1 <= '0';
					end if;

					if active_aso_valid='1' and read_data_count(read_data_count'length-1 downto 1)=image_rows/2 then
						aso_out0_endofpacket_1   <= '1';
					else
						aso_out0_endofpacket_1   <= '0';
					end if;

					if active_aso_valid='1' and read_data_count(read_data_count'length-1 downto 1)=image_rows/2+1 then
						aso_out1_B_startofpacket <= '1';
					else
						aso_out1_B_startofpacket <= '0';
					end if;

					if active_aso_valid='1' and read_data_count(read_data_count'length-1 downto 1)=image_rows then
						aso_out1_B_endofpacket   <= '1';
						addr_ack <= '1';
					else
						addr_ack <= '0';
						aso_out1_B_endofpacket   <= '0';
					end if;



				when end_read =>
					active_aso_ready <= aso_out0_A_ready;

					aso_out0_A_data          <= (others =>'0');
					aso_out1_B_data          <= (others =>'0');
					aso_out0_A_valid         <= '0';
					aso_out1_B_valid         <= '0';

					aso_out0_startofpacket_1 <= '0';
					aso_out1_B_startofpacket <= '0';
					aso_out0_endofpacket_1   <= '0';
					aso_out1_B_endofpacket   <= '0';
					addr_ack <= '0';

				when others =>
					active_aso_ready <= aso_out0_A_ready;

					aso_out0_A_data          <= (others =>'0');
					aso_out1_B_data          <= (others =>'0');
					aso_out0_A_valid         <= '0';
					aso_out1_B_valid         <= '0';
					aso_out0_startofpacket_1 <= '0';
					aso_out1_B_startofpacket <= '0';
					aso_out0_endofpacket_1   <= '0';
					aso_out1_B_endofpacket   <= '0';
					addr_ack <= '0';

			end case;
		else
			active_aso_ready <= aso_out0_A_ready;

			aso_out0_A_data          <= (others =>'0');
			aso_out1_B_data          <= (others =>'0');
			aso_out0_A_valid         <= '0';
			aso_out1_B_valid         <= '0';

			aso_out0_startofpacket_1 <= '0';
			aso_out1_B_startofpacket <= '0';
			aso_out0_endofpacket_1   <= '0';
			aso_out1_B_endofpacket   <= '0';
			addr_ack <= '0';
		end if;
	end process;
	current_byteenable_read_b <=  '1' when (avm_m0_waitrequest='0' or avm_m0_readdatavalid='1' ) and read_state=read else '0';

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
			current_address_write <= BASE_ADDR_1;
			incoming_pix_count <= (others=>'0');
			incoming_transfer_ongoing <= '0';

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

			-- switch to other buffer: set base address and increments of addr
			if addr_switch_pending='1' and (write_state=idle or write_state=wait_valid) then
				if active_base_addr='0' then
					current_address_write <= BASE_ADDR_1;
				else
					current_address_write <= BASE_ADDR_2;
				end if;
				addr_switch_pending <= '0';

			elsif (write_state=write_1 and next_write_state=write_2) or next_write_state=end_write then
			-- elsif next_write_state=write_2 or next_write_state=end_write then
				current_address_write <= current_address_write + 1;
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
					if avm_m0_waitrequest='0' then
						next_write_state <= write_2;
					else
						next_write_state <= write_1;
					end if;

				when write_2 =>
					-- next_write_state <= end_write;

					if avm_m0_waitrequest='0' then
						next_write_state <= end_write;
					else
						next_write_state <= write_2;
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
