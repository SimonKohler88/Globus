#!/usr/bin/python3
# -*- coding: utf-8 -*-

import imageio
import numpy as np
from PIL import Image
import os

if __name__ == '__main__':

    """
    Create a simple gif from a picture, which runs around vertical center of the sphere.
    
    Input Image size must be 120*256.
    
    saves the created gif in the same directory as the source picture, with the same name.
    """

    # path to file
    file = r'D:\Downloads\fhnw_double_logo_small.bmp'

    img = np.asarray(Image.open(file))

    if img.shape != (120, 256, 3):
        raise ValueError('Image shape must be (120, 256, 3)')

    width = img.shape[1]

    p, n = os.path.split(file)
    n, e = os.path.splitext(n)

    new_filename = os.path.join(p, n + '.gif')

    with imageio.get_writer(new_filename, mode='I', loop=0, fps=20) as writer:

        for i in range(width):
            img_rolled = np.roll(img, i, 1)
            writer.append_data(img_rolled)
