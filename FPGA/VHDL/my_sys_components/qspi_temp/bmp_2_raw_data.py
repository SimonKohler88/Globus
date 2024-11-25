import matplotlib.pyplot as plt
from PIL import Image
import numpy as np

if __name__ == '__main__':
    f = 'Earth_relief_120x256.bmp'
    img = np.asarray(Image.open(f))
    print(repr(img))
    pix_per_row = img.transpose(2, 0, 1).reshape(3, -1).T

    # file with hex codes: 1 pixel per line (1pixel = 24 bit)
    with open('Earth_relief_120x256_raw2.txt', 'w') as f:
        for row in pix_per_row:
            f.write(f'{row[0]:02x}{row[1]:02x}{row[2]:02x}\n')
