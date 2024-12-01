import matplotlib.pyplot as plt
from PIL import Image
import numpy as np

if __name__ == '__main__':
    pic = 'Earth_relief_120x256.bmp'
    img = np.asarray(Image.open(pic))
    print(repr(img))
    print('****************************')
    pix_per_row = img.transpose(2, 0, 1).reshape(3, -1)
    pix_per_row2 = img.flatten()
    print(pix_per_row)
    print('****************************')
    print(pix_per_row2)

    s = '\t'
    for i in range(len(pix_per_row2)):
        if i % 20 == 0:
            s += '\n\t'

        s += f'0x{pix_per_row2[i]:02x}, '

    print(s)
    print(len(pix_per_row2))
    # with open(pic, 'rb') as f:
    #     pic_raw = f.read()
    #
    # print(pic_raw)
    # # file with hex codes: 1 pixel per line (1pixel = 24 bit)
    # with open('Earth_relief_120x256_raw2.txt', 'w') as f:
    #     for row in pix_per_row:
    #         f.write(f'{row[0]:02x}{row[1]:02x}{row[2]:02x}\n')
