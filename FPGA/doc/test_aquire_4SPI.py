import os
import numpy as np
import matplotlib.pyplot as plt
import shutil


def clear_dir(p):
    if os.path.exists(p):
        shutil.rmtree(p)
    os.mkdir(p)


def __reconstruct_single(arr):
    lnSPI = [l.strip().split(',')[2][1:] for l in arr]
    lnSPI = lnSPI[4:-4]
    lnSPI = [l for i, l in enumerate(lnSPI) if i % 4 != 0]
    lnSPI = [int(l, 16) for l in lnSPI]
    npSPI = np.array(lnSPI)
    nSPI = np.reshape(npSPI, (-1, 1, 3))
    return nSPI


def reconstruct_line_append(single_1, single_2, img):
    col = np.append(single_1, single_2, axis=0)
    if img is None:
        img = col
    else:
        if col.shape[0] == 60:
            tmp = col
        else:
            tmp = np.zeros([60, 1, 3])

        img = np.append(img, tmp, axis=1)
    return img


def reconstruct_csv(path):
    imgAB = None
    imgCD = None
    for each in os.listdir(path):
        if each.endswith('.csv'):
            each_file = os.path.join(path, each)
            with open(each_file, 'r') as f:
                lines = f.readlines()
            print(each)
            # print(len(lines))
            ln = lines[8:]
            lnSPI = [l for l in ln if 'SPI,' in l]
            lnSPI2 = [l for l in ln if 'SPI2,' in l]
            lnSPI3 = [l for l in ln if 'SPI3,' in l]
            lnSPI4 = [l for l in ln if 'SPI4,' in l]

            spi_1 = __reconstruct_single(lnSPI)
            spi_2 = __reconstruct_single(lnSPI2)
            spi_2 = spi_2[::-1]

            imgAB = reconstruct_line_append(spi_1, spi_2, imgAB)

            spi_3 = __reconstruct_single(lnSPI3)
            spi_4 = __reconstruct_single(lnSPI4)
            spi_4 = spi_4[::-1]

            imgCD = reconstruct_line_append(spi_3, spi_4, imgCD)

    fig, ax = plt.subplot_mosaic("A\nC")
    axAB = ax['A']
    axCD = ax['C']

    axAB.imshow(imgAB)
    axAB.set_ylabel('Pixel 0, 2, 4, ...')
    axAB.set_xlabel('Encoder kolonne (t)')
    axAB.set_title('Vorderseite')

    axCD.imshow(imgCD)
    axCD.set_ylabel('Pixel 1, 3, 5, ...')
    axCD.set_xlabel('Encoder kolonne (t)')
    axCD.set_title('Rückseite')

    # plt.imsave('aquire_plot.png')
    plt.show()


if __name__ == '__main__':
    p = './aquire'

    reconstruct_csv(p)
    # clear_dir(p)
