import shutil
import re
import os

if __name__ == "__main__":
    file = 'led_interface.vhd'

    # re_entity = re.compile(r'entity.+\w+.+is.+(\w\W)+.*end.+entity')

    with open(file, 'r') as f:
        file_raw = f.read()

    re_entity = re.compile(r'entity (\w+)+ is[ \t\n]*([\W\w]+)end entity')
    entity = re_entity.findall(file_raw)
    entity_name = entity[0][0]
    entity_definition = entity[0][1]

    re_port_generics = re.compile(r'(?:generic.*\(([\W\w]+)\);+)*[ \t\n]*port.*\(([\W\w]+)\);')
    gen_port = re_port_generics.findall(entity_definition)

    generics = gen_port[0][0]
    ports = gen_port[0][1]
    if generics != '':
        re_generics = re.compile(r'[ \t\n]*(?:(\w+)[ \t]*:[ \t]*(\w+)[ \t]*:=[ \t]*(\w+))[ \t]*;*')
        gen_elements = re_generics.findall(generics)
    else:
        gen_elements = None


    re_ports = re.compile(r'[ \t\n]*(?:(\w+)[ \t]*:[ \t]*(\w+)[ \t]*([ ()\w]+)[ \t]*(?=:|;))')
    port_elements = re_ports.findall(ports)
    # port_elements = re_ports.match(ports)
    # print(port_elements.groups())

    t_str = """
library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

library std;
use std.textio.all;

entity {e_name}_tb is
end entity {e_name}_tb;

architecture rtl of {e_name}_tb is
    component {e_name} is
    {e_body}
    end component {e_name};
    
    component {e_name}_verify is
    {e_verify_body}
    end component {e_name}_verify;
begin
    """.format(e_name=entity_name, e_body=entity_definition)

    print(t_str)

