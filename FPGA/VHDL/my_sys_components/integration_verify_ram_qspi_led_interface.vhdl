
library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library std;
use std.textio.all;


entity integration_verify_ram_qspi_led_interface is
	generic (
		image_cols : integer := 256;
		image_rows : integer := 120
	);
	port (
		clock_clk                : out  std_logic                     := '0';             --                clock.clk
		reset_reset              : out  std_logic                     := '0';             --                reset.reset
		clock_led_spi_clk           : out  std_logic                     := '0';             --        clock_led_spi.clk

		avm_m0_address           : in std_logic_vector(23 downto 0);                    --               avm_m0.address
		avm_m0_read              : in std_logic;                                        --                     .read
		avm_m0_waitrequest       : out  std_logic                     := '0';             --                     .waitrequest
		avm_m0_readdata          : out  std_logic_vector(15 downto 0) := (others => '0'); --                     .readdata
		avm_m0_readdatavalid     : out  std_logic                     := '0';             --                     .readdatavalid
		avm_m0_write             : in std_logic;                                        --                     .write
		avm_m0_writedata         : in std_logic_vector(15 downto 0);                    --                     .writedata


		conduit_qspi_data      : out  std_ulogic_vector(3 downto 0);
        conduit_qspi_clk       : out  std_ulogic                   ;
        conduit_qspi_cs        : out  std_ulogic;

		conduit_encoder_A          : out  std_logic                     := '0';             --         conduit_encoder.enc_a
		conduit_encoder_B          : out  std_logic                     := '0';             --                        .enc_b
		conduit_encoder_index      : out  std_logic                     := '0';             --                        .enc_index
		conduit_encoder_sim_switch : out  std_logic                     := '0';             --                        .sim_switch
		conduit_encoder_sim_pulse  : out  std_logic                     := '0';            --                        .sim_pulse

		conduit_LED_A_CLK           : in   std_logic;                                        --        conduit_LED_A.led_a_clk
		conduit_LED_A_DATA          : in   std_logic;                                        --                     .led_a_data
		conduit_LED_B_CLK           : in   std_logic;                                        --        conduit_LED_B.led_b_clk
		conduit_LED_B_DATA          : in   std_logic;                                        --                     .led_b_data
		conduit_LED_C_CLK           : in   std_logic;                                        --        conduit_LED_C.led_c_clk
		conduit_LED_C_DATA          : in   std_logic;                                        --                     .led_c_data
		conduit_LED_D_CLK           : in   std_logic;                                        --        conduit_LED_D.led_d_clk
		conduit_LED_D_DATA          : in   std_logic                                        --                     .new_signal_1

	);
end entity integration_verify_ram_qspi_led_interface;


architecture rtl of integration_verify_ram_qspi_led_interface is

	procedure qspi_write_pixel(
        constant c_data    : in std_ulogic_vector(23 downto 0);
        signal clk         : in std_ulogic;
        signal qspi_clk    : out std_ulogic;
        signal data        : out std_ulogic_vector(3 downto 0)
    ) is
    begin
        qspi_clk <= '0';
        data <= (others => '0');

        wait for 2 ns;

        FOR i IN 5 downto 0 LOOP
            data <= c_data(i*4 + 3 downto i*4 );
            wait until rising_edge(clk);
            qspi_clk <= '1';
            wait until falling_edge(clk);
            qspi_clk <= '0';
        END LOOP;
    end procedure qspi_write_pixel;

	procedure qspi_write_n_pixel (
		constant pixel_to_write    : in integer range 0 to 60000;
		signal clk               : in std_logic;
        signal qspi_clk          : out std_logic;
        signal qspi_cs           : out std_logic;
        signal data              : out std_logic_vector(3 downto 0)

	) is
		file input_file : text open read_mode is "./Earth_relief_120x256_raw2.txt";
		variable v_write_data : std_ulogic_vector(23 downto 0);
        variable v_input_line : line;
	begin
		qspi_cs <= '0';
        wait for 2 ns;
        for i in 0 to pixel_to_write-1 loop
            readline(input_file, v_input_line);
            hread(v_input_line, v_write_data);
            qspi_write_pixel(v_write_data, clk, qspi_clk, data );
        end loop;
        qspi_cs <= '1';
	end procedure;


    constant c_cycle_time_100M : time := 10 ns;
    constant c_cycle_time_qspi : time := 38 ns; --26M
    constant c_cycle_time_26M : time := 33 ns;

    signal internal_qspi_clock: std_ulogic;
    signal enable :boolean:=true;


    constant c_pixel_to_send : integer := 4*20;


    component avalon_slave_ram_emulator is
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

	--file output_file_stream_A : text open write_mode is "./testio_verify_ram_qspi/stream_received_A.txt";
	--file output_file_stream_B : text open write_mode is "./testio_verify_ram_qspi/stream_received_B.txt";

	signal A :std_ulogic_vector(3 downto 0) := "1100";
	signal B :std_ulogic_vector(3 downto 0) := "0110";
	signal indexCount : unsigned(8 downto 0) := (others => '0');
	signal enable_a_b         : std_ulogic := '1';
	-- constant c_enc_S_time : time := 49 ns;
	constant c_enc_S_time : time := 6000 ns; -- real: 1/16896/4 = 1.47964e-5 = 14796.4e-9 = 14796 ns = 14,796 us
	-- time between fires @ 512 cols: 58599 ns => 5859.9 clock ccles @ 100 Mhz
	-- 2 clock per pix fetch from ram, 120 pix needed = 240 clocks (  )
	-- incoming pix: 1 pix per 22 clocks

	file input_SPI_A_file : text open write_mode is "./complete_tb_textoutput/SPI_A.txt";



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

	p_spi_clk : process
	begin
		while enable loop
            clock_led_spi_clk <= '0';
            wait for c_cycle_time_26M/2;
            clock_led_spi_clk <= '1';
            wait for c_cycle_time_26M/2;
		end loop;
		wait;  -- don't do it again
	end process p_spi_clk;


	-- qspi_clk
	p_qspi_clk : process
	begin
		while enable loop
            internal_qspi_clock <= '0';
            wait for c_cycle_time_qspi/2;
            internal_qspi_clock<= '1';
            wait for c_cycle_time_qspi/2;
		end loop;
		wait;  -- don't do it again
	end process p_qspi_clk;


	p_log_spi: process
		variable v_line : line;
		variable byte : std_logic_vector(31 downto 0);
	begin

		byte := (others => '0');
		for i in 31 downto 0 loop
			wait until rising_edge(conduit_LED_A_CLK);
			byte := byte(30 downto 0) & conduit_LED_A_DATA;
			write(v_line, to_string(now) & "   0x" & to_hstring(byte));
			writeline(input_SPI_A_file, v_line);
		end loop;


	end process;


    p_stimuli_qspi: process
        variable v_write_data : std_ulogic_vector(23 downto 0);
        variable v_input_line : line;
    begin
        conduit_qspi_cs <= '1';
        conduit_qspi_clk <= '0';
        conduit_qspi_data <= (others => '0');


        wait for 50 ns;

		qspi_write_n_pixel(100, internal_qspi_clock, conduit_qspi_clk, conduit_qspi_cs, conduit_qspi_data );
        -- conduit_qspi_cs <= '0';
        -- wait for 2 ns;
        -- for i in 0 to c_pixel_to_send loop
        --     readline(input_file, v_input_line);
        --     hread(v_input_line, v_write_data);
        --     qspi_write_pixel(v_write_data, internal_qspi_clock, conduit_qspi_clk, conduit_qspi_data );
        -- end loop;
        -- conduit_qspi_cs <= '1';

        wait for 50 us;

        -- conduit_qspi_cs <= '0';
        -- wait for 2 ns;
        -- for i in 0 to c_pixel_to_send loop
        --     readline(input_file, v_input_line);
        --     hread(v_input_line, v_write_data);
        --     qspi_write_pixel(v_write_data, internal_qspi_clock, conduit_qspi_clk, conduit_qspi_data );
        -- end loop;
        -- conduit_qspi_cs <= '1';


        wait;
    end process;



	stim_proc_encoder: process
	begin
		-- A before B
		-- From AMTS datasheet
		-- T = 1/(360/512 * 33.33) = 0.042s
		-- P = T/2 = 0.0213 s
		-- S = P/2 = 0.0106 s
		indexCount <= "000000000";
		--indexCount  <= indexCount + 1;

		wait for 50 ns;
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
    end process;

	conduit_encoder_A <= A(0);
	conduit_encoder_B <= B(0);
	conduit_encoder_index <= '1' when indexCount = 1 else '0';
    p_stimuli: process
    begin
		conduit_encoder_sim_switch <= '0';
		conduit_encoder_sim_pulse <= '0';

		wait for 50 ns;


        wait for 200 us;

        wait;
    end process p_stimuli;

    p_monitor: process

    begin
		s_dump_ram <= '0';


		-- wait until col_fire='1';
		wait for 100 us;
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
