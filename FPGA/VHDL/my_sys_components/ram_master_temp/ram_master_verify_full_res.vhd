
library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library std;
use std.textio.all;

entity ram_master_verify_full_res is
generic (
		--image_cols : integer := 256;
		image_rows : integer := 120;
		image_cols_bits : integer := 8;
		ram_address_bits :integer := 10
	);
	port (
        clock_clk              : out  std_ulogic                   ;
        reset_reset            : out  std_ulogic                   ;

        aso_out0_data          : out   std_ulogic_vector(23 downto 0);
        aso_out0_endofpacket   : out   std_ulogic                   ;
        aso_out0_ready         : in  std_ulogic                   ;
        aso_out0_startofpacket : out   std_ulogic                   ;
        aso_out0_valid         : out   std_ulogic                   ;

        avm_m0_address           : in std_logic_vector(23 downto 0);                    --               avm_m0.address
		avm_m0_read              : in std_logic;                                        --                     .read
		avm_m0_waitrequest       : out  std_logic                     := '0';             --                     .waitrequest
		avm_m0_readdata          : out  std_logic_vector(15 downto 0) := (others => '0'); --                     .readdata
		avm_m0_readdatavalid     : out  std_logic                     := '0';             --                     .readdatavalid
		avm_m0_write             : in std_logic;                                        --                     .write
		avm_m0_writedata         : in std_logic_vector(15 downto 0);                    --                     .writedata

        asi_in1_data                : in  std_logic_vector(23 downto 0) := (others => '0'); --            asi_in1_B.data
		asi_in1_ready               : out std_logic;                                        --                     .ready
		asi_in1_valid               : in  std_logic                     := '0';             --                     .valid
		asi_in1_startofpacket       : in  std_logic                     := '0';             --                     .startofpacket
		asi_in1_endofpacket         : in  std_logic                     := '0';             --                     .endofpacket
		asi_in0_data                : in  std_logic_vector(23 downto 0) := (others => '0'); --            asi_in0_A.data
		asi_in0_ready               : out std_logic;                                        --                     .ready
		asi_in0_valid               : in  std_logic                     := '0';             --                     .valid
		asi_in0_startofpacket       : in  std_logic                     := '0';             --                     .startofpacket
		asi_in0_endofpacket         : in  std_logic                     := '0';              --                     .endofpacket
		conduit_intern_col_nr      : out std_logic_vector(8 downto 0);                     -- conduit_intern_col_info.col_nr
		conduit_intern_col_fire    : out std_logic                                         --                        .fire

	);
end entity ram_master_verify_full_res;

architecture rtl of ram_master_verify_full_res is

-- testing with pic size of 10* 120 pix (= 1200 pixel = 0x4B0)
-- -> ram_emulator: minimum 1200 * 2 * 2 = 4800 addresses  (2400= 0x960)
-- -> ram_master offset: A00 => 2* A00 = 0x1400 ::> 14 Bits addr width in emulator

procedure avalon_stream_out_write_many_pixel(
        constant c_num     : in integer;
        constant c_time_intervall_ns : in integer range 5 to 100;
        signal clk         : in std_ulogic;
        signal data        : out std_ulogic_vector(23 downto 0);
        signal valid       : out std_ulogic;
        signal in_ready    : in std_ulogic;
        signal packet_start: out std_ulogic;
        signal packet_end  : out std_ulogic;
        constant delay_ready2valid_clocks : in integer

    ) is
        file input_file : text open read_mode is "./Earth_relief_120x256_raw2.txt";
        variable v_input_data : std_ulogic_vector(23 downto 0);
        variable v_input_line : line;
        variable pix_count : integer := 0;
    begin

        wait for 10 ns;
        for i in 0 to c_num-1 loop
            readline(input_file, v_input_line);
            hread(v_input_line, v_input_data);

            wait until rising_edge(in_ready);

            if delay_ready2valid_clocks > 0 then
                for j in 0 to delay_ready2valid_clocks loop
                    wait until rising_edge(clock_clk);
                end loop;
            end if;

            if i=0 then
                packet_start <= '1';
            end if;

            if i=c_num-1 then
                packet_end <= '1';
            end if;

            data <= v_input_data;
            valid <= '1';
            pix_count := pix_count +1;

            wait until rising_edge(clk);
            valid <= '0';
            packet_start <= '0';
            packet_end <= '0';

            wait for 10 ns;
            --qspi_write_pixel(v_input_data, clk, qspi_clk, data );
        end loop;
        write(output, "Pixel Sent: " & to_string(pix_count) & lf);
    end procedure avalon_stream_out_write_many_pixel;

    procedure pulse_out(signal out_signal: out std_ulogic; signal clk : in std_ulogic) is
    begin
        wait until rising_edge(clk);
        out_signal <= '1';
        wait until rising_edge(clk);
        out_signal <= '0';
    end procedure pulse_out;

    procedure save_stream(
            constant ab         :in std_ulogic;
            signal f_nr       :in integer;
            constant assert_num :in integer;
            signal col_nr       :in std_logic_vector(8 downto 0);
            signal valid        :in std_ulogic;
            signal data         :in std_ulogic_vector(23 downto 0);
            signal endofpacket  :in std_ulogic;
            signal clk          :in std_ulogic ) is
        variable pix_count: integer:=0;
        variable v_output_line : line;
        file pix_stream_file : text open write_mode is "./stream_"& to_string(image_rows)&"x"&to_string(2**image_cols_bits)&"/"& to_string(f_nr) & "_in" & to_string(ab) & "_col_" & to_string(to_integer(unsigned(col_nr))) & ".txt";
    begin
        pix_count := pix_count + 1;

        write(v_output_line, "col " & to_hstring(conduit_intern_col_nr));
        writeline(pix_stream_file, v_output_line);
        wait until falling_edge(clk);
        write(v_output_line, to_string(pix_count), left, 5);
        write(v_output_line, to_hstring(data) );
        writeline(pix_stream_file, v_output_line);

        while true loop
            wait until rising_edge(valid);
            pix_count := pix_count + 1;
            wait until falling_edge(clk);
            write(v_output_line, to_string(pix_count), left, 5);
            write(v_output_line, to_hstring(data) );
            writeline(pix_stream_file, v_output_line);
             if endofpacket = '1' then
                exit;
            end if;

        end loop;
        assert pix_count = assert_num report "in" & to_string(ab) & ": Received wrong amount of pixels: expected " & to_string(assert_num) & ", received " & to_string(pix_count) & lf severity error;
    end procedure save_stream;

    component avalon_slave_ram_emulator is
    generic (
		RAM_ADDR_BITS : integer := 10
	);
	port (
		rst : in std_ulogic;
		clk : in std_ulogic;
        address       : in  std_ulogic_vector(23 downto 0);
        read          : in  std_ulogic;
        waitrequest   : out std_ulogic;
        readdata      : out std_ulogic_vector(15 downto 0);
        readdatavalid : out std_ulogic;
        write_en         : in  std_ulogic;
        writedata     : in  std_ulogic_vector(15 downto 0);
		dump_ram      : in  std_ulogic
	);
    end component avalon_slave_ram_emulator;

    signal s_dump_ram                 : std_ulogic;

    constant c_cycle_time_100M : time := 10 ns;
    signal enable :boolean:=true;
    constant c_image_cols : integer:= 2**image_cols_bits;

    signal colAB: integer;
    signal colCD: integer;

    signal transfer_out_ongoing : std_ulogic;

    signal column_count: unsigned(10 downto 0);


begin

    helper_ram_emulator: avalon_slave_ram_emulator
    generic map(
        RAM_ADDR_BITS => ram_address_bits
    )
    port map (
		rst           => reset_reset              ,
		clk           => clock_clk                ,
        address       => avm_m0_address           ,
        read          => avm_m0_read              ,
        waitrequest   => avm_m0_waitrequest       ,
        readdata      => avm_m0_readdata          ,
        readdatavalid => avm_m0_readdatavalid     ,
        write_en      => avm_m0_write             ,
        writedata     => avm_m0_writedata,
        dump_ram      => s_dump_ram
	);

    reset_reset <= transport '1', '0' after 5 ns;

    -- some psl stuff
    default clock is rising_edge (clock_clk);

	-- 100MHz
	p_system_clk : process
	begin
		while enable loop
            clock_clk <= '0';
            wait for c_cycle_time_100M/2;
            clock_clk <= '1';
            wait for c_cycle_time_100M/2;
		end loop;
		wait;  -- don't do it again
	end process p_system_clk;

    -- check : must be a endofpacket after startofpacket
    assert always aso_out0_startofpacket -> eventually! aso_out0_endofpacket;
	p_stimuli_avs_out: process
        alias dut_const_row2row_offset is <<constant .ram_master_full_res_tb.dut_ram_master.addr_row_to_row_offset:integer>>;
        alias addr_b_col_shift_offset is <<constant .ram_master_full_res_tb.dut_ram_master.addr_b_col_shift_offset:integer>>;
        alias image_cols is <<constant .ram_master_full_res_tb.dut_ram_master.image_cols:integer>>;

	begin
        aso_out0_data <= (others => '0');
        aso_out0_Valid <= '0';
        aso_out0_endofpacket <= '0';
        aso_out0_startofpacket <= '0';
        transfer_out_ongoing <= '0';
        s_dump_ram <= '0';
        write(output, "addr row2row off: " & to_string(dut_const_row2row_offset) & lf);
        write(output, "addr_b_col_shift_offset: " & to_string(addr_b_col_shift_offset) & lf);
        write(output, "image_cols: " & to_string(image_cols) & lf);

        wait for 50 ns;

        transfer_out_ongoing <= '1';
        avalon_stream_out_write_many_pixel(120*256, 20, clock_clk, aso_out0_data, aso_out0_valid,
                aso_out0_ready, aso_out0_startofpacket, aso_out0_endofpacket, 0 );
        transfer_out_ongoing <= '0';
        pulse_out(s_dump_ram, clock_clk);
        wait;

	end process p_stimuli_avs_out;
	p_stimuli_fire: process
	begin
        conduit_intern_col_fire <= '0';
        conduit_intern_col_nr <= (others => '0');
        column_count <= (others => '0');

        wait until falling_edge(transfer_out_ongoing);
        wait for 10 ns;

        for i in 0 to 512 loop
            pulse_out(conduit_intern_col_fire, clock_clk);
            wait until rising_edge(asi_in1_endofpacket);
            column_count <= column_count + 1;
            conduit_intern_col_nr(7 downto 0) <= std_logic_vector(column_count(7 downto 0));
            wait for 20 ns;
            wait until rising_edge(clock_clk);

        end loop;
        wait for 100 ns;
        enable <= false;
        -- conduit_intern_col_nr(3 downto 0) <= X"1";
        -- pulse_out(conduit_intern_col_fire, clock_clk);

        -- wait until rising_edge(asi_in1_endofpacket);
        -- wait for 20 ns;
        -- conduit_intern_col_nr(3 downto 0) <= X"6";
        -- pulse_out(conduit_intern_col_fire, clock_clk);
        wait;

	end process p_stimuli_fire;



	-- --------------------------- STREAM IN AB ---------------------------------------------

    assert always asi_in0_startofpacket -> eventually! asi_in0_endofpacket;
    assert always conduit_intern_col_fire -> next_e[ 0 to 20 ] (asi_in0_startofpacket and asi_in0_valid);
    assert always asi_in0_startofpacket -> asi_in0_valid;
    assert always never not asi_in0_ready and (asi_in0_valid or asi_in0_startofpacket or asi_in0_endofpacket );

	p_stimuli_avs_in_AB: process

	begin
        asi_in0_ready <= '1';

        while enable loop
            wait until rising_edge(asi_in0_startofpacket);
            colAB <= to_integer(column_count)/c_image_cols;
            save_stream('0', colAB, 60, conduit_intern_col_nr, asi_in0_valid, asi_in0_data, asi_in0_endofpacket, clock_clk );

        end loop;
        -- wait until rising_edge(asi_in0_startofpacket);
        -- save_stream('0', 1, 60, conduit_intern_col_nr, asi_in0_valid, asi_in0_data, asi_in0_endofpacket, clock_clk );
        -- wait until rising_edge(asi_in0_startofpacket);
        -- save_stream('0', 1, 60, conduit_intern_col_nr, asi_in0_valid, asi_in0_data, asi_in0_endofpacket, clock_clk );


		wait;

	end process p_stimuli_avs_in_AB;

	-- --------------------------- STREAM IN CD ---------------------------------------------

    assert always asi_in0_endofpacket -> next_e[ 0 to 20 ] ( asi_in1_startofpacket );
    assert always asi_in1_startofpacket -> eventually! asi_in1_endofpacket;
    assert always asi_in1_startofpacket -> asi_in1_valid;
    assert always never not asi_in1_ready and (asi_in1_valid or asi_in1_startofpacket or asi_in1_endofpacket );

	p_stimuli_avs_in_CD: process
	begin
        asi_in1_ready <= '1';

        while enable loop
            wait until rising_edge(asi_in1_startofpacket);
            colCD <= to_integer(column_count)/c_image_cols;
            save_stream('1', colCD, 60, conduit_intern_col_nr, asi_in1_valid, asi_in1_data, asi_in1_endofpacket, clock_clk );

        end loop;

        -- wait until rising_edge(asi_in1_startofpacket);
        -- save_stream('1', 1, 60, conduit_intern_col_nr, asi_in1_valid, asi_in1_data, asi_in1_endofpacket, clock_clk );
        -- wait until rising_edge(asi_in1_startofpacket);
        -- save_stream('1', 1, 60, conduit_intern_col_nr, asi_in1_valid, asi_in1_data, asi_in1_endofpacket, clock_clk );
		wait;

	end process p_stimuli_avs_in_CD;











































end architecture rtl; -- of qspi_interface
