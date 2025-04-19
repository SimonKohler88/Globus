#!/usr/bin/python3
# -*- coding: utf-8 -*-

import cv2
import numpy as np
import time

if __name__ == '__main__':
    arr_file = 'pic_raw.npy'

    pics = np.load(arr_file)

    for each in pics:
        bgr = cv2.cvtColor(each, cv2.COLOR_RGB2BGR)
        cv2.imshow('win', bgr)
        time.sleep(0.1)
