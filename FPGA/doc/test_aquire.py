import cv2
import os
import numpy as np
import matplotlib.pyplot as plt

if __name__ == '__main__':
    p = 'acq0001.csv'
    img = None

    # with open(p, 'r') as f:
    #     lines = f.readlines()
    #
    # ln = lines[8:]
    # lnSPI = [l for l in ln if 'SPI,' in l]
    # lnSPI2 = [l for l in ln if 'SPI2,' in l]
    #
    # lnSPI = [l.strip().split(',')[2][1:] for l in lnSPI]
    # lnSPI = lnSPI[4:-4]
    # lnSPI = [l for i, l in enumerate(lnSPI) if i % 4 != 0]
    # lnSPI = [int(l, 16) for l in lnSPI]
    # # print(lnSPI)
    # npSPI = np.array(lnSPI)
    # nSPI = np.reshape(npSPI, (-1, 1, 3))
    # # print(nSPI)
    #
    # lnSPI2 = [l.strip().split(',')[2][1:] for l in lnSPI2]
    # # print(lnSPI2)
    # lnSPI2 = lnSPI2[4:-4]
    # # print(lnSPI2)
    # lnSPI2 = [l for i, l in enumerate(lnSPI2) if i % 4 != 0]
    # # lnSPI2 = [l for l in lnSPI2 if l.lower() != 'e8']
    # # print(lnSPI2)
    # lnSPI2 = [int(l, 16) for l in lnSPI2]
    # npSPI2 = np.array(lnSPI2)
    # nSPI2 = np.reshape(npSPI2, (-1, 1, 3))
    # nSPI2 = nSPI2[::-1]
    # print(nSPI2)
    path = './aquire'
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

            lnSPI = [l.strip().split(',')[2][1:] for l in lnSPI]
            lnSPI = lnSPI[4:-4]
            lnSPI = [l for i, l in enumerate(lnSPI) if i % 4 != 0]
            lnSPI = [int(l, 16) for l in lnSPI]
            # print(lnSPI)
            npSPI = np.array(lnSPI)
            nSPI = np.reshape(npSPI, (-1, 1, 3))
            # print(nSPI)

            lnSPI2 = [l.strip().split(',')[2][1:] for l in lnSPI2]
            # print(lnSPI2)
            lnSPI2 = lnSPI2[4:-4]
            # print(lnSPI2)
            lnSPI2 = [l for i, l in enumerate(lnSPI2) if i % 4 != 0]
            # lnSPI2 = [l for l in lnSPI2 if l.lower() != 'e8']
            # print(lnSPI2)
            lnSPI2 = [int(l, 16) for l in lnSPI2]
            npSPI2 = np.array(lnSPI2)
            nSPI2 = np.reshape(npSPI2, (-1, 1, 3))
            nSPI2 = nSPI2[::-1]

            col = np.append(nSPI, nSPI2, axis=0)
            if img is None:
                img = col
            else:
                if col.shape[0] == 60:
                    tmp = col
                else:
                    tmp = np.zeros([60,1,3])

                img = np.append(img, tmp, axis=1)
            # print(col)


    print(img.shape)
    plt.imshow(img)
    plt.show()
    # cv2.imshow('image', img)
    # cv2.waitKey(0)