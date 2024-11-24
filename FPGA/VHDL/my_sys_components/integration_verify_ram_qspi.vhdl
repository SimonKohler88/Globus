
library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library std;
use std.textio.all;


entity integration_verify_ram_qspi is
	generic (
		image_cols : integer := 256;
		image_rows : integer := 120
	);
	port (
		clock_clk                : out  std_logic                     := '0';             --                clock.clk
		reset_reset              : out  std_logic                     := '0';             --                reset.reset
		avm_m0_address           : in std_logic_vector(24 downto 0);                    --               avm_m0.address
		avm_m0_read              : in std_logic;                                        --                     .read
		avm_m0_waitrequest       : out  std_logic                     := '0';             --                     .waitrequest
		avm_m0_readdata          : out  std_logic_vector(15 downto 0) := (others => '0'); --                     .readdata
		avm_m0_readdatavalid     : out  std_logic                     := '0';             --                     .readdatavalid
		avm_m0_write             : in std_logic;                                        --                     .write
		avm_m0_writedata         : in std_logic_vector(15 downto 0);                    --                     .writedata

		aso_out1_B_data          : in std_logic_vector(23 downto 0);                    --           aso_out1_B.data
		aso_out1_B_endofpacket   : in std_logic;                                        --                     .endofpacket
		aso_out1_B_ready         : out  std_logic                     := '0';             --                     .ready
		aso_out1_B_startofpacket : in std_logic;                                        --                     .startofpacket
		aso_out1_B_valid         : in std_logic;                                        --                     .valid
		aso_out0_startofpacket_1 : in std_logic;                                        --           aso_out0_A.startofpacket
		aso_out0_endofpacket_1   : in std_logic;                                        --                     .endofpacket
		aso_out0_A_data          : in std_logic_vector(23 downto 0);                    --                     .data
		aso_out0_A_ready         : out  std_logic                     := '0';             --                     .ready
		aso_out0_A_valid         : in std_logic;                                        --                     .valid
		avs_s1_address           : out  std_logic_vector(7 downto 0)  := (others => '0'); --               avs_s1.address
		avs_s1_read              : out  std_logic                     := '0';             --                     .read
		avs_s1_readdata          : in std_logic_vector(31 downto 0);                    --                     .readdata
		avs_s1_write             : out  std_logic                     := '0';             --                     .write
		avs_s1_writedata         : out  std_logic_vector(31 downto 0) := (others => '0'); --                     .writedata
		avs_s1_waitrequest       : in std_logic ;                                        --                     .waitrequest

		conduit_encoder_A          : out  std_logic                     := '0';             --         conduit_encoder.enc_a
		conduit_encoder_B          : out  std_logic                     := '0';             --                        .enc_b
		conduit_encoder_index      : out  std_logic                     := '0';             --                        .enc_index
		conduit_encoder_sim_switch : out  std_logic                     := '0';             --                        .sim_switch
		conduit_encoder_sim_pulse  : out  std_logic                     := '0';            --                        .sim_pulse

		col_fire : in std_ulogic
	);
end entity integration_verify_ram_qspi;


architecture rtl of integration_verify_ram_qspi is

    component avalon_slave_ram_emulator is
	port (
		rst : in std_ulogic;
		clk : in std_ulogic;
        address       : in  std_ulogic_vector(24 downto 0);
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

	file output_file_stream_A : text open write_mode is "./testio_verify_ram_qspi/stream_received_A.txt";
	file output_file_stream_B : text open write_mode is "./testio_verify_ram_qspi/stream_received_B.txt";

	signal A :std_ulogic_vector(3 downto 0) := "1100";
	signal B :std_ulogic_vector(3 downto 0) := "0110";
	signal indexCount : unsigned(8 downto 0) := (others => '0');
	signal enable_a_b         : std_ulogic := '1';
	-- constant c_enc_S_time : time := 49 ns;
	constant c_enc_S_time : time := 3000 ns; -- real: 1/16896/4 = 1.47964e-5 = 14796.4e-9 = 14796 ns = 14,796 us
	-- time between fires @ 512 cols: 58599 ns => 5859.9 clock ccles @ 100 Mhz
	-- 2 clock per pix fetch from ram, 120 pix needed = 240 clocks (  )
	-- incoming pix: 1 pix per 22 clocks


begin
	 helper_ram_emulator: avalon_slave_ram_emulator port map (
		rst           => reset_reset               ,
		clk           => clock_clk               ,
        address       => avm_m0_address           ,
        read          => avm_m0_read              ,
        waitrequest   => avm_m0_waitrequest       ,
        readdata      => avm_m0_readdata          ,
        readdatavalid => avm_m0_readdatavalid     ,
        write_en      => avm_m0_write             ,
        writedata     => avm_m0_writedata,
        dump_ram      => s_dump_ram
	);
	--s_dump_ram <= '0';

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

    reset_reset <= transport '1', '0' after 5 ns;

     p_store_stream_A: process(all) --untested yet
        variable v_output_line : line;
    begin
        if rising_edge(clock_clk) then
            if aso_out0_A_ready = '1' and aso_out0_A_valid ='1' then
                write(v_output_line, to_hstring(aso_out0_A_data));
                writeline(output_file_stream_A, v_output_line);
            end if;
        end if;
    end process p_store_stream_A;

     p_store_stream_B: process(all) --untested yet
        variable v_output_line : line;
    begin
        if rising_edge(clock_clk) then
            if aso_out1_B_ready = '1' and aso_out1_B_valid ='1' then
                write(v_output_line, to_hstring(aso_out1_B_data));
                writeline(output_file_stream_B, v_output_line);
            end if;
        end if;
    end process p_store_stream_B;

	stim_proc_encoder: process
	begin
		-- A before B
		-- From AMTS datasheet
		-- T = 1/(360/512 * 33.33) = 0.042s
		-- P = T/2 = 0.0213 s
		-- S = P/2 = 0.0106 s
		indexCount <= "000000000";
		--indexCount  <= indexCount + 1;
		while enable loop
			if enable_a_b='1' then
				wait for c_enc_S_time;
				A <= A(2 downto 0) & A(3);
				B <= B(2 downto 0) & B(3);
				wait for c_enc_S_time;
				A <= A(2 downto 0) & A(3);
				B <= B(2 downto 0) & B(3);
				wait for c_enc_S_time;
				A <= A(2 downto 0) & A(3);
				B <= B(2 downto 0) & B(3);
				wait for c_enc_S_time;
				A <= A(2 downto 0) & A(3);
				B <= B(2 downto 0) & B(3);
				indexCount   <= indexCount + 1;

			else
				wait for c_enc_S_time;
				A <= A(0) & A(3 downto 1);
				B <= B(0) & B(3 downto 1);
				wait for c_enc_S_time;
				A <= A(0) & A(3 downto 1);
				B <= B(0) & B(3 downto 1);
				wait for c_enc_S_time;
				A <= A(0) & A(3 downto 1);
				B <= B(0) & B(3 downto 1);
				wait for c_enc_S_time;
				A <= A(0) & A(3 downto 1);
				B <= B(0) & B(3 downto 1);
				indexCount   <= indexCount - 1;
			end if;
		end loop;
        wait;
    end process stim_proc_encoder;

	conduit_encoder_A <= A(0);
	conduit_encoder_B <= B(0);
	conduit_encoder_index <= '1' when indexCount = 1 else '0';



    p_stimuli: process
    begin
		conduit_encoder_sim_switch <= '0';
		conduit_encoder_sim_pulse <= '0';

		wait for 50 ns;
		aso_out0_A_ready <= '1';
		aso_out1_B_ready <= '1';


        wait for 200 us;

        wait;
    end process p_stimuli;

    p_monitor: process

    begin
		s_dump_ram <= '0';


		wait until col_fire='1';
		s_dump_ram <= '1';
		wait for 20 ns;
		s_dump_ram <= '0';

		wait for 120 us;
		enable <= false;
		enable_a_b <= '0';
		write(output, "all tested " & to_string(now) & lf);
		wait;
    end process p_monitor;























































end architecture;
