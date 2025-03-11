import os

this_file_path = os.path.dirname(os.path.abspath(__file__))
input_file = os.path.join(this_file_path, 'row_col_num.txt')

output_file_A = os.path.join(this_file_path, 'stream_out', 'stream_received_A.txt')
output_file_B = os.path.join(this_file_path, 'stream_out', 'stream_received_B.txt')
output_file_C = os.path.join(this_file_path, 'stream_out', 'stream_received_C.txt')
output_file_D = os.path.join(this_file_path, 'stream_out', 'stream_received_D.txt')

second_stream_start_pix = 100
pixel_per_burst_per_stream = 30
starting_0_bytes = 4
ending_1_bytes = 4
bytes_per_pixel = 4


def read_stream_data(file):
    with open(file, 'r') as f:
        data = f.readlines()
    temp = [line.split() for line in data]
    byte_nr = [int(t[0]) for t in temp]
    data = [t[1][1:] for t in temp]
    d = [[byte_nr[i], data[i]] for i in range(len(byte_nr))]
    print(d)
    out = []
    word = ''
    n = 0
    if len(d) % 4 != 0:
        print('not multiple of 4', len(d))
    for i in range(len(d)):
        word += d[i][1]
        if i % 4 == 0 and i != 0:
            out.append([n, word])
            word = ''
            n += 1
    print('32 Bit Words: ', len(out) )

    return out


if __name__ == '__main__':
    pass
    with open(input_file, 'r') as f:
        in_data = f.readlines()

    stream_A = read_stream_data(output_file_A)
    # stream_B = read_stream_data(output_file_B)
    # stream_C = read_stream_data(output_file_C)
    # stream_D = read_stream_data(output_file_D)

    print(stream_A)
