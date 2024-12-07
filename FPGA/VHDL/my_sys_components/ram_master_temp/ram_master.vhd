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
		image_cols : integer := 256;
		-- image_rows : integer := 120
		image_rows : integer := 20
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
	);--avs_s1_waitrequest
end entity ram_master;

architecture rtl of ram_master is

	signal SystemEnable                  : std_logic;

	type state_main is (main_write, main_read_A, main_read_B);
	signal main_state         : state_main;
	signal next_main_state    : state_main;

	type state_AVM_write is (idle, wait_valid, write_1, write_2, end_write);
	signal write_state        : state_AVM_write := idle;
	signal next_write_state   : state_AVM_write := idle;

	type state_AVM_read is (idle, wait_read_1, read_1, wait_read_2, read_2, end_read);
	signal read_state         : state_AVM_read := idle;
	signal next_read_state    : state_AVM_read := idle;

	signal write_address      : unsigned(23 downto 0) := (others => '0');
	signal read_address       : unsigned(23 downto 0) := (others => '0');

	signal data_in_buffer     : std_logic_vector(23 downto 0) := (others => '0');
	signal data_out_buffer    : std_logic_vector(23 downto 0) := (others => '0');

	constant BASE_ADDR_1      : unsigned(23 downto 0):= (others => '0');

	-- 60'000 pixel need addr range of 120'000          (= 0x01E000)
	constant BASE_ADDR_2: unsigned(23 downto 0):=   X"020000"; --TODO: this in real design
	-- constant BASE_ADDR_2      : unsigned(23 downto 0):=  X"001000"; -- only for testing
	signal active_base_addr   : std_logic;

	constant addr_row_to_row_offset  : integer := 2* image_cols -1;
	constant addr_b_col_shift_offset : integer := image_cols/2;
	signal addr_ab_converter         : unsigned (7 downto 0); -- TODO: only valid for 256 cols

	signal addr_ready                : std_logic;
	signal addr_valid                : std_logic;
	signal addr_a_preload            : unsigned(23 downto 0) := (others => '0');
	signal addr_b_preload            : unsigned(23 downto 0) := (others => '0');
	signal addr_adder                : unsigned(23 downto 0) := (others => '0');

	signal end_packet_ff             : std_logic_vector(1 downto 0);

	signal current_address_write     : unsigned(23 downto 0) := (others => '0');
	signal current_address_read      : unsigned(23 downto 0) := (others => '0');

	signal active_aso_startofpacket  : std_logic;
	signal active_aso_endofpacket    : std_logic;
	signal active_aso_data           : std_logic_vector(23 downto 0);
	signal active_aso_ready          : std_logic;
	signal active_aso_valid          : std_logic;
	signal pix_count                 : unsigned(7 downto 0);

	signal col_fire_ff               : std_logic_vector(1 downto 0);
	signal fire_pending              : std_logic;

	signal ram_read_1_buffer : std_logic_vector(15 downto 0);


	type state_test is (none, t1, t_data2ram);
	signal test_state         : state_test;


begin
	test_state <= none;
	p_test: process(all)
	begin
		case test_state is
			when none =>
				conduit_debug_ram_out(31 downto 1) <= (others => '0');
				conduit_debug_ram_out(0) <= reset_reset;
				conduit_debug_ram_out_2(31 downto 0) <= (others => '0');

			when t1 =>

				-- conduit_debug_ram_out_2(0) <= '1' when ((main_state=main_write) and (avm_m0_address="000000") and (not avm_m0_write_n)) else '0';
				if (main_state=main_write) then
					if (avm_m0_address="000000") then
						if ( avm_m0_write_n='0') then
							conduit_debug_ram_out_2(0) <= '1';
						else
							conduit_debug_ram_out_2(0) <= '0';
						end if;
					else
						conduit_debug_ram_out_2(0) <= '0';
					end if;
				else
					conduit_debug_ram_out_2(0) <= '0';
				end if;

				conduit_debug_ram_out(15 downto 0) <= avm_m0_writedata;
				conduit_debug_ram_out(31 downto 16) <= (others => '0');
				conduit_debug_ram_out_2(31 downto 1) <= (others => '0');

			when t_data2ram =>
				conduit_debug_ram_out(23 downto 0) <= asi_in0_data;
				conduit_debug_ram_out(31) <= asi_in0_valid;
				conduit_debug_ram_out(30) <= asi_in0_ready;
				conduit_debug_ram_out(29 downto 4) <= (others => '0');

				conduit_debug_ram_out_2(31 downto 1) <= (others => '0');

			when others =>
				conduit_debug_ram_out(31 downto 0) <= (others => '0');
				conduit_debug_ram_out_2(31 downto 0) <= (others => '0');

		end case;
	end process;

	avs_s1_waitrequest <= '1'; -- not implemented
	avs_s1_readdata <= (others => '0');

	avm_m0_address <= std_logic_vector(write_address)  when main_state = main_write  else
					  std_logic_vector(read_address)  when main_state = main_read_A or main_state=main_read_B else
					  (others=>'0');





	--TODO: uncomment startup delay
	-- SystemEnable <= '1';
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

	-- p_end_packet_switch :process(all)
	-- begin
	-- 	if reset_reset = '1' then
	-- 	elsif rising_edge(clock_clk) then
	-- 	end if;
	-- end process p_end_packet_switch;

	addr_adder <= BASE_ADDR_1 when active_base_addr='1' else BASE_ADDR_2;

	p_calculate_new_read_addr: process(all)
	variable stage : natural range 0 to 5;
	begin
	-- constant addr_row_to_row_offset: integer := 2* image_cols;
	-- constant addr_b_col_shift_offset : integer := image_cols/2;
	-- signal addr_ab_converter : unsigned (7 downto 0); -- TODO: only valid for 256 cols

		if reset_reset = '1' then
			addr_ab_converter <= (others=>'0');
			addr_a_preload <= (others => '0');
			addr_b_preload <= (others => '0');
			stage := 0;
			addr_ready <= '0';
			addr_valid <= '0';

		elsif rising_edge(clock_clk) then
			if conduit_col_info_fire='1' and stage=0 then
				addr_a_preload <= (others => '0');
				addr_b_preload <= (others => '0');

				addr_valid <= '0';
				stage := 1;

			elsif stage=1 then

				addr_a_preload(9 downto 1) <= unsigned(conduit_col_info_col_nr); -- *2 --TODO: need this
				-- addr_a_preload(0) <= '0';
				-- addr_a_preload(8 downto 0) <= unsigned(conduit_col_info_col_nr);

				addr_ab_converter <= unsigned(conduit_col_info_col_nr( 7 downto 0 )); -- TODO: case 512 cols: need all bits


				addr_valid <= '0';
				stage:=2;
			elsif stage=2 then
				addr_a_preload <= addr_a_preload + addr_adder;
				-- addr_b_preload(9 downto 0) <= (addr_ab_converter - addr_b_col_shift_offset) & "0"; -- let wrap around and shift for *2 (512 version)
				addr_b_preload(8 downto 0) <= (addr_ab_converter - addr_b_col_shift_offset) & "0"; -- let wrap around and shift for *2
				addr_b_preload(7 downto 0) <= (addr_ab_converter - addr_b_col_shift_offset);

				addr_valid <= '0';
				stage:=3;

			elsif stage=3 then
				addr_b_preload <= addr_b_preload + addr_adder + image_cols; -- TODO:does that work when addr is doubled?
				addr_ready <= '1';
				addr_valid <= '0';
				stage := 0;
			else
				addr_ready <= '0';
				addr_valid <= '1';
			end if;
		end if;
	end process p_calculate_new_read_addr;

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

			if main_state = main_read_A then
				fire_pending <= '0';
			end if;

		end if;
	end process p_main_state_clocked;


	p_main_state_statemachine: process(all)
	begin
	--(write, read_A, write_2, read_B);

		case main_state is
			when main_write =>
				if fire_pending='1' and (write_state=idle or write_state=wait_valid) and addr_valid='1' then
					next_main_state <= main_read_A;
				else
					next_main_state <= main_write;
				end if;

			when main_read_A =>
				if read_state = end_read then
					next_main_state <= main_read_B;
				else
					next_main_state <= main_read_A;
				end if;

			when main_read_B =>
				if read_state = end_read then
					next_main_state <= main_write;
				else
					next_main_state <= main_read_B;
				end if;

		end case;

		case main_state is
			when main_write =>
				aso_out0_startofpacket_1 <=  active_aso_startofpacket  ;
				aso_out0_endofpacket_1   <=  active_aso_endofpacket    ;
				aso_out0_A_data          <=  active_aso_data           ;
				aso_out0_A_valid         <=  active_aso_valid          ;

				active_aso_ready <= aso_out0_A_ready;

				aso_out1_B_startofpacket <= '0'           ;
				aso_out1_B_endofpacket   <= '0'           ;
				aso_out1_B_data          <= (others =>'0');
				aso_out1_B_valid         <= '0'           ;

			when main_read_A =>
				aso_out0_startofpacket_1 <=  active_aso_startofpacket  ;
				aso_out0_endofpacket_1   <=  active_aso_endofpacket    ;
				aso_out0_A_data          <=  active_aso_data           ;
				aso_out0_A_valid         <=  active_aso_valid          ;


				active_aso_ready <= aso_out0_A_ready;

				aso_out1_B_startofpacket <= '0'           ;
				aso_out1_B_endofpacket   <= '0'           ;
				aso_out1_B_data          <= (others =>'0');
				aso_out1_B_valid         <= '0'           ;

			when main_read_B =>
				aso_out1_B_startofpacket <=  active_aso_startofpacket  ;
				aso_out1_B_endofpacket   <=  active_aso_endofpacket    ;
				aso_out1_B_data          <=  active_aso_data           ;
				aso_out1_B_valid         <=  active_aso_valid          ;

				active_aso_ready <= aso_out1_B_ready;

				aso_out0_startofpacket_1 <= '0'           ;
				aso_out0_endofpacket_1   <= '0'           ;
				aso_out0_A_data          <= (others =>'0');
				aso_out0_A_valid         <= '0'           ;

		end case;


	end process p_main_state_statemachine;

	--*************************************** Read from ram processes ***************************************************************

	-- main_state_b_active <= '1' when main_state=main_read_B else '0';
	--type state_AVM_read is (idle, wait_read_1, read_1, wait_read_2, read_2, end_read);
	p_read_ram_clocked :process(all)
	begin
		if reset_reset = '1' then
			read_state <= idle;
		elsif rising_edge(clock_clk) then
			read_state <= next_read_state;
		end if;
	end process p_read_ram_clocked;

-- 	avm_m0_address
-- avm_m0_read
-- avm_m0_waitrequest
-- avm_m0_readdata
-- avm_m0_readdatavalid

	p_AVM_Read_statemachine: process(all)
	constant PIX_PER_ASO: integer:= image_rows/2;
	-- constant PIX_PER_ASO: integer:= 20; --TODO:check value. must be 60 in real design
    begin
		if (main_state = main_read_A or main_state = main_read_B) then
			case read_state is
				when idle =>
					if main_state = main_read_A or main_state = main_read_B then
						next_read_state <= wait_read_1;
					else
						next_read_state <= idle;
					end if;

				when wait_read_1 =>
					-- if active_aso_ready='1' and avm_m0_readdatavalid='1' and avm_m0_waitrequest='0' then
					if active_aso_ready='1' and avm_m0_waitrequest='0' then
						next_read_state <= read_1;
					else
						next_read_state <= wait_read_1;
					end if;

				when read_1 =>
					if avm_m0_waitrequest='0' and avm_m0_readdatavalid='1' then
						next_read_state <= read_2;
					else
						next_read_state <= wait_read_2;
					end if;

				when wait_read_2 =>
					if avm_m0_waitrequest='0' and avm_m0_readdatavalid='1' then
						next_read_state <= read_2;
					else
						next_read_state <= wait_read_2;
					end if;

				when read_2 =>
					if pix_count < PIX_PER_ASO then
						if avm_m0_waitrequest='1' then
							next_read_state <= wait_read_1;
						else
							next_read_state <= read_1;
						end if;
					else
						next_read_state <= end_read;
					end if;

				when end_read =>
					next_read_state <= idle;

				when others =>
					next_read_state <= idle;
			end case;

			case read_state is
				when idle =>
					read_address <= (others =>'0');
					avm_m0_read_n <= '1';
					active_aso_valid <= '0';
					active_aso_startofpacket <= '0';
					active_aso_endofpacket <= '0';

				when wait_read_1 =>
					read_address <= current_address_read;
					avm_m0_read_n <= '0';
					active_aso_valid <= '0';
					active_aso_startofpacket <= '0';
					active_aso_endofpacket <= '0';

				when read_1 =>
					read_address <= current_address_read;
					avm_m0_read_n <= '0';
					active_aso_valid <= '0';
					active_aso_endofpacket <= '0';
					active_aso_startofpacket <= '0';

				when wait_read_2 =>
					read_address <= current_address_read;
					avm_m0_read_n <= '0';
					active_aso_valid <= '0';
					active_aso_endofpacket <= '0';
					active_aso_startofpacket <= '0';

				when read_2 =>
					read_address <= current_address_read;
					avm_m0_read_n <= '0';
					active_aso_valid <= '1';

					if pix_count=0 and (next_read_state=read_1 or next_read_state=wait_read_1) then
						active_aso_startofpacket <= '1';
						active_aso_endofpacket <= '0';
					elsif pix_count >= PIX_PER_ASO-1 and next_read_state=end_read then
						active_aso_endofpacket <= '1';
						active_aso_startofpacket <= '0';
					else
						active_aso_endofpacket <= '0';
						active_aso_startofpacket <= '0';
					end if;

				when end_read =>
					read_address <= (others =>'0');
					avm_m0_read_n <= '1';
					active_aso_valid <= '0';
					active_aso_startofpacket <= '0';
					active_aso_endofpacket <= '0';

				when others =>
					read_address <= (others =>'0');
					avm_m0_read_n <= '1';
					active_aso_valid <= '0';
					active_aso_startofpacket <= '0';
					active_aso_endofpacket <= '0';
			end case;
		else
			next_read_state <= idle;
			avm_m0_read_n <= '1';
			active_aso_valid <= '0';
			active_aso_startofpacket <= '0';
			active_aso_endofpacket <= '0';
			read_address <= (others =>'0');

		end if;

	end process p_AVM_Read_statemachine;

	p_pix_count: process(all)
	begin
		if reset_reset = '1' then
			pix_count <= (others=>'0');

		elsif rising_edge(clock_clk) then
			if (main_state = main_read_A or main_state = main_read_B) then
				-- case next_read_state is
				case read_state is
					when end_read | idle => pix_count <= (others=>'0');
					when read_2 => pix_count <= pix_count +1;
					when others => pix_count <= pix_count;
				end case;
			else
				pix_count <= (others=>'0');
			end if;
		end if;
	end process p_pix_count;

	p_update_addr: process(all)
	begin
		if reset_reset = '1' then
		current_address_read <= (others=>'0');

		elsif rising_edge(clock_clk) then
			if addr_ready='1' then
				current_address_read <= addr_a_preload;

			elsif main_state=main_read_A and read_state=end_read then
				current_address_read <= addr_b_preload;

			elsif next_read_state=read_1 then
				current_address_read <= current_address_read + 1;

			elsif next_read_state= read_2 then -- TODO: change for hot version
				current_address_read <= current_address_read + addr_row_to_row_offset;
				-- current_address_read <= current_address_read + 1;--test version;
			end if;
		end if;
	end process p_update_addr;

	p_set_stream_out:process(all)
	begin
	if reset_reset = '1' then
		ram_read_1_buffer <= (others=> '0');

	elsif rising_edge(clock_clk) then
		if read_state=read_1 then
			ram_read_1_buffer <= avm_m0_readdata;
		end if;
	end if;

	-- not clocked !!
	if read_state = read_2 then
		active_aso_data <=  ram_read_1_buffer & avm_m0_readdata(15 downto 8);
	else
		active_aso_data <= (others =>'0');
	end if;
	end process p_set_stream_out;

	--*************************************** Write to ram processes ***************************************************************

	asi_in0_ready <= '1' when main_state=main_write and ( write_state = wait_valid ) and SystemEnable='1' else '0';

	--control input (but not state) of "write to ram" statemachine
	p_write_ram_clocked : PROCESS(all)
	variable addr_switch_pending: std_ulogic;
	BEGIN
		IF reset_reset = '1' THEN
			write_state <= idle; -- Zustand nach reset
			end_packet_ff <= (others=>'0');
			active_base_addr <= '0';
			data_in_buffer <= (others =>'0');
			addr_switch_pending := '0';
			current_address_write <= BASE_ADDR_1;

		ELSIF rising_edge(clock_clk)THEN
			write_state <= next_write_state;

			-- buffering data in. we dont know if we can write it already
			if asi_in0_valid = '1' then
				data_in_buffer <= asi_in0_data;
			end if;

			-- remember end of packet -> switch to other buffer
			end_packet_ff <= end_packet_ff(0) & asi_in0_endofpacket;
			if end_packet_ff = "01" then
				active_base_addr <= not active_base_addr;
				addr_switch_pending := '1';
			end if;

			-- switch to other buffer: set base address and increments of addr
			if addr_switch_pending='1' and (write_state=idle or write_state=wait_valid) then
				if active_base_addr='0' then
					current_address_write <= BASE_ADDR_1;
				else
					current_address_write <= BASE_ADDR_2;
				end if;
				addr_switch_pending := '0';

			elsif next_write_state=write_2 or next_write_state=end_write then
				current_address_write <= current_address_write + 1;
			end if;

		END IF;
	END PROCESS p_write_ram_clocked;

	  --Interface that communicates with RAM controller over Avalon-MM
    p_AVM_Write_statemachine : process(all) is
    begin
--set_base_addr, idle, wait_write_1, write_1, wait_write_2, write_2, end_write

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
					next_write_state <= end_write;

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






































end architecture rtl; -- of ram_master
