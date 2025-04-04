#!/usr/bin/python3
# -*- coding: utf-8 -*-


def get_string_1(c_arr):
    s = """
    #include "psram_fifo_static_pic.h"
    #include <string.h>
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"

    void ext_copy_static_pic_to_PSRAM( uint8_t* start_ptr )
    {
        /* Copy from code directly to PSRAM, So it does not use Stack */

        uint8_t* temp_ptr = start_ptr;
        temp_ptr = start_ptr;
        memcpy( temp_ptr, ( uint8_t[] ) {
        """
    bytes_per_block = int(len(c_arr) / 160)
    block = 0
    block_byte = 0
    for i, each in enumerate(c_arr):
        if block_byte % 20 == 0 and block_byte != 0:
            s += '\n\t'

        if block_byte == bytes_per_block:
            s += f' }},{bytes_per_block} );\n\n'
            # s+= '\tvTaskDelayUntil( 1 );\n\n'
            s += '\ttaskYIELD();\n\n'
            s += f'\ttemp_ptr += {bytes_per_block};\n'
            s += '\tmemcpy( temp_ptr, ( uint8_t[] ) {\n'
            s += '\t'
            block_byte = 0
        s += each
        s += ', '
        block_byte += 1

    s += f' }},{bytes_per_block} );\n'
    s += '}'
    return s


def get_string_2(c_arr):
    s = """
    #include "psram_fifo_static_pic.h"
    #include <string.h>
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"

    void ext_copy_static_pic_to_PSRAM( uint8_t* start_ptr )
    {
        /* Copy from code directly to PSRAM, So it does not use Stack */

        
        """
    for i, each in enumerate(c_arr):
        s += f'\tstart_ptr[{i}] = {each};\n'
    s += '}'
    return s

if __name__ == '__main__':
    original_data_file = 'Earth_relief_120x256_raw2.txt'

    c_file_name_2_write = 'psram_fifo_static_pic.c'
    with open(original_data_file, 'r') as f:
        orig_data = f.readlines()

    orig_data = [data_line.strip() for data_line in orig_data]

    c_arr = []

    for each in orig_data:
        c_arr.append(f'0x{each[:2]}')
        c_arr.append(f'0x{each[2:4]}')
        c_arr.append(f'0x{each[4:]}')

    s = get_string_2(c_arr)

    with open(c_file_name_2_write, 'w') as f:
        f.write(s)
    print()
