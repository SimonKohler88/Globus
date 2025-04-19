#!/usr/bin/python3
# -*- coding: utf-8 -*-

import make_gif_library as gl
import numpy as np
from PIL import Image

if __name__ == '__main__':
    pic = "Earth_relief_120x256.bmp"
    img = np.asarray(Image.open(pic))
    globe, orig_ax, new_ax = gl.convert_pic_and_rotate(img, np.deg2rad(90), np.deg2rad(90), np.deg2rad(23.4))
    globe_line_axis = np.array([new_ax * (-1), new_ax]) * 2

    pic_raw = []
    for i in range(256):
        t = i / 256 - i // 256
        ang = t * np.pi * 2

        g = gl.rotate_converted(*globe, new_ax, ang)
        img = gl.pos_col_to_img(*g, 256, 120)
        img2 = gl.interpolate_picture(img)
        pic_raw.append(img2)

    np.save("pic_raw", np.asarray(pic_raw))
