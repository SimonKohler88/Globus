library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


library std;
use std.textio.all;

entity avalon_slave_ram_emulator is
	port (
		rst : in std_ulogic;
		clk : in std_ulogic;
        address       : in  std_ulogic_vector(23 downto 0) := (others=>'0');
        read          : in  std_ulogic := '0';
        waitrequest   : out std_ulogic := '0';
        readdata      : out std_ulogic_vector(15 downto 0) := (others=> '0');
        readdatavalid : out std_ulogic :='0';
        write_en      : in  std_ulogic := '1';
        writedata     : in  std_ulogic_vector(15 downto 0) := (others=> '0');
        dump_ram      : in  std_ulogic := '0'
	);
end entity avalon_slave_ram_emulator;

architecture rtl of avalon_slave_ram_emulator is
  signal access_d : std_ulogic := '0';
  signal access_cnt : unsigned(1 downto 0) := (others=>'0');
  signal delay_cnt : unsigned(2 downto 0) := (others=>'0');
  signal enable_delay_cnt : std_ulogic := '0';
  signal write_en_int :std_ulogic := '0';
  signal read_n :std_ulogic := '0';

  signal dump_ram_ff :std_ulogic_vector(1 downto 0):= (others=>'0');

  constant clock_cycle_time_ns :integer := 10;
  constant refresh_time_ns: integer:=60;
  constant refresh_intervall_ns :integer:=15625;
  signal refresh_intervall_count : integer := 0;
  signal refresh_ram: std_ulogic:= '0';

  -- -- first read is always delayed by 3 clock cycles
  -- constant cas_delay : integer := 3;
  -- signal cas_delay: unsigned(1 downto 0):= (others => '0');

  -- constant RAM_ADDR_BITS : integer := 18;
  constant RAM_ADDR_BITS : integer := 13;
  --type t_mem is array (0 to 2**24-1) of std_ulogic_vector(15 downto 0);
  -- type t_mem is array (0 to 2** RAM_ADDR_BITS-1) of std_ulogic_vector(15 downto 0);
  signal read_data_buf : std_ulogic_vector(15 downto 0) := (others=>'0');
  -- type t_mem is array (0 to 511) of std_ulogic_vector(15 downto 0);
  type t_mem is array (0 to 16383) of std_ulogic_vector(15 downto 0);
  signal mem : t_mem := (others=>(others=>'0'));

 -- file input_file : text open read_mode is "./ram_content.txt";
  file output_file : text open write_mode is "./ram_dumps/ram_content_verify_ram_qspi.txt";


  type state_delay is (idle, delay, access_ok);
  signal delay_state : state_delay:=idle;
  signal next_delay_state : state_delay:=idle;
  signal wait_req_intern : std_ulogic;

begin
  write_en_int <= not write_en;
  read_n <= not read;

  p_delay : process(rst, clk)
  begin
    if rst = '1' then
      access_d <= '0';


    elsif rising_edge(clk) then
      access_d <= write_en_int or read_n;



    end if;
  end process p_delay;

  p_access_delay:process(all)
  begin
   if rst = '1' then
      delay_state <= idle;
      access_cnt <= (others=>'0');

    elsif rising_edge(clk) then
      delay_state <= next_delay_state;

      if delay_state=idle or delay_state=access_ok then
        access_cnt <= (others=>'0');
      elsif access_cnt < 1 then
        access_cnt <= access_cnt+1;
      end if;
    end if;

    case delay_state is
      when idle =>
        if ((write_en_int or read_n) and not(access_d)) = '1' then
          next_delay_state <= delay;
        end if;
      when delay =>
        if access_cnt = 1 then
          next_delay_state <= access_ok;
        end if;
      when access_ok =>
        if write_en_int='0' and read_n ='0' then
          next_delay_state <= idle;
        end if;
    end case;

    case delay_state is
      when idle =>
        wait_req_intern <= '1';
      when delay =>
        wait_req_intern <= '1';
      when access_ok =>
        wait_req_intern <= '0';
    end case;
  end process p_access_delay;

  waitrequest <= wait_req_intern or refresh_ram;


  p_dump_ram:process(all)
    variable v_output_line : line;
  begin
    if rst = '1' then
      dump_ram_ff <= (others => '0');

    elsif rising_edge(clk) then
      dump_ram_ff <= dump_ram_ff(0) & dump_ram;

      if dump_ram_ff= "01" then

        for i in 0 to mem'length-1 loop
          -- write(v_output_line, i, left, 8);
          write(v_output_line, "0x" & to_hstring(to_signed(i, 32)), left, 12);
          write(v_output_line, "0x"&to_hstring(mem(i)));
          writeline(output_file, v_output_line);
        end loop;
      end if;

    end if;

  end process p_dump_ram;


  p_ram_refresh: process(all)
  begin
    if rst = '1' then
      refresh_intervall_count <= 0;

    elsif rising_edge(clk) then
      if refresh_intervall_count < refresh_intervall_ns + refresh_time_ns  then
        refresh_intervall_count <= refresh_intervall_count + clock_cycle_time_ns;
      else
        refresh_intervall_count <= 0;
      end if;
    end if;
  end process p_ram_refresh;

  refresh_ram <= '1' when (refresh_intervall_count > refresh_intervall_ns) else '0';

  -- write:
  p_store : process(rst, clk)
  begin
    if rst = '1' then
      mem <= (others => (others => '0'));
    elsif rising_edge(clk) then
      if waitrequest = '0' and write_en_int = '1' then
        mem(to_integer(unsigned(address(RAM_ADDR_BITS downto 0)))) <= writedata;
      end if;
    end if;
  end process p_store;

  -- read:
  p_read : process(rst, clk)
  begin
    if rst = '1' then
      readdatavalid <= '0';
      readdata <= (others => '0');

    elsif rising_edge(clk) then
      if waitrequest = '0' and read_n = '1' then
        readdatavalid <= '1';
        readdata <= read_data_buf;
      else
        readdatavalid <= '0';
        readdata <= (others => '0');
      end if;
    end if;
  end process p_read;

  read_data_buf <= mem(to_integer(unsigned(address(RAM_ADDR_BITS downto 0))));

end architecture rtl;
