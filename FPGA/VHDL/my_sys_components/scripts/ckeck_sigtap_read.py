#!/usr/bin/python3
# -*- coding: utf-8 -*-


import os
import numpy as np

if __name__=='__main__':
    original_data_file = 'Earth_relief_120x256_raw2_addr_space.txt'

    sigtap_csv = os.path.expanduser('~/Downloads/sigtap_csv/sigtap_read.csv')


    with open(original_data_file, 'r') as f:
        orig_data = f.readlines()
    orig_data = [data_line.strip() for data_line in orig_data]
    orig_data = [n.split() for n in orig_data]
    orig_data_d = {n[0][2:].upper():n[1][2:].upper() for n in orig_data}

    with open(sigtap_csv, 'r') as f:
        sigtap_temp = f.readlines()

    # prep data
    sigtap_data_temp = sigtap_temp[7:]
    sigtap_data_temp = [data_line.split(',') for data_line in sigtap_data_temp]

    sigtap_data_2 = []
    for each in sigtap_data_temp:
        sigtap_data_2.append([n.strip() for n in each if n.strip() != ''])

    # search trigger
    sigtap_data = []
    trigger_found = False
    trigger_num = None
    for i in range(len(sigtap_data_2)):
        if not trigger_found and sigtap_data_2[i][-1] == '1':
            trigger_num = sigtap_data_2[i][0]
            trigger_found = True

        if trigger_found:
            sigtap_data.append(sigtap_data_temp[i])
    sigtap_data = sigtap_data[2:]

    # split to multiple arrays and convert
    sigtap_data_t = [data_line[0] for data_line in sigtap_data]
    sigtap_data_dbg0 = [data_line[1] for data_line in sigtap_data]
    sigtap_data_valid = [data_line[19] for data_line in sigtap_data]
    sigtap_data_waitr = [data_line[20] for data_line in sigtap_data]
    sigtap_data_addr = [data_line[21:-3] for data_line in sigtap_data]

    sigtap_data_addr = [''.join(data_line) for data_line in sigtap_data_addr]
    sigtap_data_addr = [int(data_line.replace(' ', ''), base=2) for data_line in sigtap_data_addr]
    sigtap_data_addr = [f'{data_line:08X}' for data_line in sigtap_data_addr]

    # for i in range(len(sigtap_data)):
    #     print(sigtap_data_dbg0[i], ' ', sigtap_data_valid[i], ' ', sigtap_data_addr[i], ' ', sigtap_data_waitr[i])


    # filter out according to valid/writereq
    data_valid = []
    for i in range(len(sigtap_data_valid)):
        if sigtap_data_valid[i].strip() == '1':
            data_valid.append(sigtap_data_dbg0[i])

    addr_valid = []
    for i in range(len(sigtap_data_waitr)):
        if sigtap_data_waitr[i].strip() == '0':
            addr_valid.append(sigtap_data_addr[i])

    print(sigtap_data_valid)
    print(data_valid)
    print(addr_valid)

    print(len(data_valid), len(addr_valid))
    read_cnt = 0
    errors = []
    for data, addr in zip(data_valid, addr_valid):
        read_cnt += 1
        ram_data = orig_data_d[addr]
        print(addr, '  ', data, ram_data, '  ', read_cnt)
        if data != ram_data:
            errors.append([read_cnt, addr, data, ram_data])
    print(errors)
