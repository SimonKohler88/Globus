import os

input_file = 'Earth_relief_120x256_raw2_reshaped_256x120.txt'
with open(input_file, 'r') as fily:
    all_lines = fily.readlines()

all_lines = all_lines[1:]
all_lines = [a.split() for a in all_lines]
all_lines = [a[1:] for a in all_lines]
errors = 0

this_file_path = os.path.dirname(os.path.abspath(__file__))
COLUMNS = 256


def read_file(file):
    p = os.path.join(this_file_path, file)
    with open(p, 'r') as fily:
        all_lines = fily.readlines()

    all_lines = all_lines[1:]
    all_lines = [a.split() for a in all_lines]
    all_lines = [a[1:] for a in all_lines]
    return all_lines


if __name__ == '__main__':
    print('python check starting')
    for file in os.listdir(this_file_path):
        if file[0].isdigit():  # first letter of filename must be a "zahl"
            splitted = file.split('_')
            testcase = int(splitted[0])
            stream_no = splitted[1]
            col_nr = int(splitted[-1].split('.')[0])

            has_error = False

            file_ = os.path.join(this_file_path, file)

            with open(file_) as f:
                file_lines = f.readlines()
            file_lines = file_lines[1:]
            file_lines = [a.split() for a in file_lines]
            file_lines = [a[1] for a in file_lines]

            if stream_no == 'in1':
                col_nr += int(COLUMNS / 2)
                if col_nr > (COLUMNS - 1):
                    col_nr -= COLUMNS

            for i, each in enumerate(file_lines):
                num = each.lower()
                if stream_no == 'in0':  # pix 0,2,4,6,...
                    pix_num = i * 2
                else:
                    pix_num = i * 2 + 1

                try:
                    num_soll = all_lines[pix_num][col_nr]
                    if num_soll != num:
                        has_error = True
                        errors += 1
                        print(f'Error in {file}, pix {pix_num}, num_soll {num_soll}, num {num}')

                except IndexError:
                    has_error = True
                    errors += 1
                    print(f'IndexError in {file}, pix {pix_num}, num {num}')

            if not has_error:
                print(f'File {file} OK')

    print(f'Total Errors: {errors}')
