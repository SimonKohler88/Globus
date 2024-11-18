library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity avalon_slave_ram_emulator is
	port (
		rst : in std_ulogic;
		clk : in std_ulogic;
        address       : in  std_ulogic_vector(24 downto 0);
        read          : in  std_ulogic;
        waitrequest   : out std_ulogic;
        readdata      : out std_ulogic_vector(15 downto 0);
        readdatavalid : out std_ulogic;
        write         : in  std_ulogic;
        writedata     : in  std_ulogic_vector(15 downto 0)
	);
end entity avalon_slave_ram_emulator;

architecture rtl of avalon_slave_ram_emulator is
  signal access_d : std_ulogic;
  signal access_cnt : unsigned(1 downto 0);
  signal delay_cnt : unsigned(2 downto 0);
  signal enable_delay_cnt : std_ulogic;
  constant RAM_ADDR_BITS : integer := 18;
  --type t_mem is array (0 to 2**24-1) of std_ulogic_vector(15 downto 0);
  type t_mem is array (0 to 2** RAM_ADDR_BITS -1) of std_ulogic_vector(15 downto 0);
  signal mem : t_mem;

 -- file input_file : text open read_mode is "./ram_content.txt";
 --    file output_file : text open write_mode is "./ram_content.txt";
begin

  p_delay : process(rst, clk)
  begin
    if rst = '1' then
      access_d <= '0';
    elsif rising_edge(clk) then
      access_d <= write or read;
    end if;
  end process p_delay;

  -- count accesses: every 3rd is delayed
  p_access_cnt : process(rst, clk)
  begin
    if rst = '1' then
      access_cnt <= (others => '0');
    elsif rising_edge(clk) then
      if ((write or read) and not(access_d)) = '1' then  -- rising-edge(wr or rd)
        if access_cnt = 2 then
          access_cnt <= (others => '0');
        else
          access_cnt <= access_cnt+1;
        end if;
      end if;
    end if;
  end process p_access_cnt;

  p_enable : process(rst, clk)
  begin
    if rst = '1' then
      enable_delay_cnt <= '0';
    elsif rising_edge(clk) then
      if access_cnt = 2 then
        enable_delay_cnt <= '1';
      elsif delay_cnt = 4 then
        enable_delay_cnt <= '0';
      end if;
    end if;
  end process p_enable;

  p_delay_cnt : process(rst, clk)
  begin
    if rst = '1' then
      delay_cnt <= (others => '0');
    elsif rising_edge(clk) then
      if enable_delay_cnt = '1' then
        delay_cnt <= (delay_cnt+1) mod 5;
      end if;
    end if;
  end process p_delay_cnt;

  waitrequest <= '1' when delay_cnt /= 0 else '0';

  -- write:
  p_store : process(rst, clk)
  begin
    if rst = '1' then
      mem <= (others => (others => '0'));
    elsif rising_edge(clk) then
      if waitrequest = '0' and write = '1' then
        mem(to_integer(unsigned(address(RAM_ADDR_BITS downto 0)))) <= writedata;
      end if;
    end if;
  end process p_store;

  -- read:
  p_read : process(rst, clk)
  begin
    if rst = '1' then
      readdatavalid <= '0';
    elsif rising_edge(clk) then
      if waitrequest = '0' and read = '1' then
        readdatavalid <= '1';
      else
        readdatavalid <= '0';
      end if;
    end if;
  end process p_read;

  readdata <= mem(to_integer(unsigned(address(RAM_ADDR_BITS downto 0))));

end architecture rtl;
