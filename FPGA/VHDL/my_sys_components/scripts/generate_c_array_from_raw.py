#!/usr/bin/python3
# -*- coding: utf-8 -*-

if __name__ == '__main__':
    original_data_file = 'Earth_relief_120x256_raw2.txt'
    with open(original_data_file, 'r') as f:
        orig_data = f.readlines()

    orig_data = [data_line.strip() for data_line in orig_data]

    c_arr = []


    for each in orig_data:
        c_arr.append(f'0x{each[:2]}')
        c_arr.append(f'0x{each[2:4]}')
        c_arr.append(f'0x{each[4:]}')

    for i, each in enumerate(c_arr):
        if i%20 == 0 and i != 0:
            print('\t')
        print(each, end= ', ')
    print(i)
