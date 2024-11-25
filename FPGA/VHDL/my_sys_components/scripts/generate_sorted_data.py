import matplotlib.pyplot as plt
from PIL import Image
import numpy as np

if __name__ == '__main__':

    # file with hex codes: 1 pixel per line (1pixel = 24 bit)

    with open('row_col_num.txt', 'w') as f:
        for row in range(120):
            for col in range(256):
                f.write(f'{row:02x}{col:02x}{0xFF:02x}\n')



