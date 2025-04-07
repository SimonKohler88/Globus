#!/usr/bin/python3
# -*- coding: utf-8 -*-


import os
import numpy as np

if __name__=='__main__':
    original_data_file = 'Earth_relief_120x256_raw2.txt'

    sigtap_csv = os.path.expanduser('~/Downloads/stp_t_fifo_check_2.csv')

    with open(sigtap_csv, 'r') as f:
        sigtap_temp = f.readlines()


    sigtap_data = sigtap_temp[6:]
    sigtap_data = [data_line.split(',') for data_line in sigtap_data]
    sigtap_data = [data_line for data_line in sigtap_data if data_line[2].strip()=='1']
    sigtap_data_2 = []

    for each in sigtap_data:
        sigtap_data_2.append([n.strip() for n in each if n.strip() != ''])

    sigtap_data_t = [data_line[0] for data_line in sigtap_data_2]
    sigtap_data = [data_line[3:-1] for data_line in sigtap_data_2]
    sigtap_data = [''.join(data_line) for data_line in sigtap_data]
    sigtap_data = [int(data_line, base=2) for data_line in sigtap_data]
    sigtap_data = [f'{data_line:06x}' for data_line in sigtap_data]

    with open(original_data_file, 'r') as f:
        orig_data = f.readlines()

    orig_data = [data_line.strip() for data_line in orig_data]


    for i, (sig_data, or_data, t) in enumerate(zip(sigtap_data, orig_data, sigtap_data_t)):
        print(f'{or_data}  {sig_data}')
        if or_data.lower() != sig_data.lower():
            print(f'ERROR---------------------------   {i}  {int(t) -2048}')
    print(f'Checked {i} pixels')

    print()