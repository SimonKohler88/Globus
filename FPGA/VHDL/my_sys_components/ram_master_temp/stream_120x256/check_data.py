import os

input_file = 'Earth_relief_120x256_raw2_reshaped_256x120.txt'
with open(input_file, 'r') as fily:
    all_lines = fily.readlines()

all_lines = all_lines[1:]
all_lines = [a.split() for a in all_lines]
all_lines = [a[1:] for a in all_lines]
errors = 0
for file in os.listdir('.'):
    if file.startswith('0') or file.startswith('1'):
        splitted = file.split('_')
        stream_no = splitted[1]
        col_nr = int(splitted[-1].split('.')[0])

        has_error = False

        with open(file) as f:
            file_lines = f.readlines()
        file_lines = file_lines[1:]
        file_lines = [a.split() for a in file_lines]
        file_lines = [a[1] for a in file_lines]

        if stream_no == 'in1':
            col_nr += 128
            if col_nr > 255:
                col_nr -= 256

        for i, each in enumerate(file_lines):
            num = each.lower()
            if stream_no == 'in0':  # pix 0,2,4,6,...
                pix_num = i * 2
            else:
                pix_num = i * 2 + 1

            num_soll = all_lines[pix_num][col_nr]
            if num_soll != num:
                has_error = True
                errors += 1
                print(f'Error in {file}, pix {pix_num}, num_soll {num_soll}, num {num}')

        if not has_error:
            print(f'File {file} OK')

print(f'Total Errors: {errors}')