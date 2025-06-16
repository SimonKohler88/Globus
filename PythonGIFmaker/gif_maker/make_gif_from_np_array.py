#!/usr/bin/python3
# -*- coding: utf-8 -*-

import imageio
import numpy as np
import time

if __name__ == '__main__':
    arr_file = 'pic_raw.npy'
    arr_file = 'pic_world_blue_marble_nov2004.npy'

    pics = np.load(arr_file)

    with imageio.get_writer('globe_world_blue_marble_nov2004.gif', mode='I', loop=0, fps=20) as writer:

        for each in pics:
            # image = imageio.imread(filename)
            writer.append_data(each)

