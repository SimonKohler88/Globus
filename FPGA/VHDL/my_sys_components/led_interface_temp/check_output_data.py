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
first_led_byte = 'e8'


def read_stream_data(file):
    with open(file, 'r') as f:
        data = f.readlines()
    temp = [line.split() for line in data]
    byte_nr = [int(t[0]) for t in temp]
    data = [t[1] for t in temp]
    d = [[byte_nr[i], data[i]] for i in range(len(byte_nr))]
    print(d)
    if len(d) % 4 != 0:
        print('not multiple of 4', len(d))
    out = []
    n = 0
    word = ''
    q = 0
    for i in range(len(d)):
        word += d[i][1]
        # print(word)
        if q == 3:
            q = 0
            out.append([n, word])
            word = ''
            n += 1
        else:
            q += 1
    print('32 Bit Words: ', len(out))
    return out


if __name__ == '__main__':
    pass
    with open(input_file, 'r') as f:
        in_data = f.readlines()

    stream_A = read_stream_data(output_file_A)
    # stream_B = read_stream_data(output_file_B)
    # stream_C = read_stream_data(output_file_C)
    # stream_D = read_stream_data(output_file_D)

    


    # burst = 0
    # pix = 0
    # bursted = 0
    # for w_num, word in stream_A:
    #     # if w_num < pixel_per_burst_per_stream + 2:
    #     #     continue
    #
    #     if w_num % (pixel_per_burst_per_stream + 2) == 0 and w_num != 0:
    #         burst += 1
    #         pix = 0
    #         bursted = 1
    #         print('burst')
    #
    #     match burst:
    #         case 1:
    #             if not bursted and pix < pixel_per_burst_per_stream:
    #                 print(pix)
    #                 # print(f'A: {w_num:<5}0x{word},  {in_data[pix]}')
    #                 pix += 1
    #
    #                 brightness = word[:2]
    #                 pixel_data = word[2:]
    #
    #                 print(f'A: {w_num:<5}{brightness} {pixel_data} ,  {in_data[pix]}')
    #
    #                 if brightness.lower() != first_led_byte:
    #                     print(f'ERROR Brightness A, {w_num}: {brightness}, expected {first_led_byte}')
    #                 if pixel_data.lower() != in_data[pix]:
    #                     print(f'ERROR Data A, {w_num}: {pixel_data}, expected {in_data[pix]}')
    #
    #     bursted = 0
