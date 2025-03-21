import os

if __name__ == '__main__':
    file = 'Earth_relief_120x256_raw2.txt'

    file_name, ext = os.path.splitext(file)
    addr_offset = 0x00

    with open(file, 'r') as f:
        all_lines = f.readlines()

    out_string = ''
    for i, each_line in enumerate(all_lines):
        row_nr = addr_offset + i * 2
        data_1 = each_line[:4]
        data_2 = each_line[4:].strip() + '00'

        out_string += f'0x{row_nr:08X}  0x{data_1.upper()}\n0x{row_nr + 1:08X}  0x{data_2.upper()}\n'

    new_name = f'{file_name}_addr_space{ext}'

    with open(new_name, 'w') as f:
        f.write(out_string)
