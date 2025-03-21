

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;


entity diverses is
	generic (
		counter_bits : integer := 19;
		on_clocks : integer := 10000;
		dbg_enc_after_req0_clocks : integer := 1024-- default 1024 clock-cycles
	);
	port (
		clock_clk                   : in  std_logic                     := '0';             --                clock.clk
		reset_reset                 : in  std_logic                     := '0';
		enable_pin_input            : in  std_logic                     := '0';
		pin_input                   : in  std_logic                     := '0';
		pulse_out                   : out std_logic;
		enable_dbg_enc              : in std_logic;
		dbg_enc_out                 : out std_logic
    );
end entity diverses;

architecture rtl of diverses is


	 signal addr_a_preload            : unsigned(23 downto 0) := (others => '0');
	signal addr_b_preload            : unsigned(23 downto 0) := (others => '0');
	signal addr_adder                : unsigned(23 downto 0) := (others => '0');

	constant image_cols_bits: integer:= 8;
	signal addr_ab_converter         : unsigned (image_cols_bits-1 downto 0); -- TODO: only valid for 256
	constant addr_row_to_row_offset  : integer := 2* 2**image_cols_bits -1;
	constant addr_b_col_shift_offset : integer := 2**image_cols_bits/2;
	constant image_cols :integer := 2**image_cols_bits;
	signal conduit_col_info_col_nr  :   std_logic_vector(8 downto 0)  := (others => '0');

begin
	
	-- generate internal pulse
		p_calculate_new_read_addr: process(all)
	variable stage : natural range 0 to 5;
	begin
		if reset_reset = '1' then
			addr_ab_converter <= (others=>'0');
			addr_a_preload <= (others => '0');
			addr_b_preload <= (others => '0');
			stage := 0;



		elsif rising_edge(clock_clk) then
			if pin_input='1' and stage=0 then
				addr_a_preload <= (others => '0');
				addr_b_preload <= (others => '0');
				addr_ab_converter <= (others => '0');


				stage := 1;

			elsif stage=1 then

				addr_a_preload(9 downto 1) <= unsigned(conduit_col_info_col_nr); -- factor 2 because every pix need 2 addresses
				addr_ab_converter <= unsigned(conduit_col_info_col_nr( image_cols_bits-1 downto 0 ));-- convert to unsigned

				stage:=2;
			elsif stage=2 then
				addr_a_preload <= addr_a_preload + addr_adder;
				addr_b_preload(image_cols_bits downto 0) <= (addr_ab_converter + addr_b_col_shift_offset) & "0"; -- let wrap around and shift for *2

				stage:=3;

			elsif stage=3 then
				addr_b_preload <= addr_b_preload + addr_adder + image_cols + image_cols;

				stage := 0;
			end if;

		end if;
	end process p_calculate_new_read_addr;
end architecture;




























