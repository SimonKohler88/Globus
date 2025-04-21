#!/usr/bin/python3
# -*- coding: utf-8 -*-

import make_gif_library as gl
import numpy as np
from PIL import Image
import os


def make_raw_bmp_images(globe, new_ax, file_name='raw'):
    pic_raw = []
    for i in range(256):
        t = i / 256 - i // 256
        ang = t * np.pi * 2

        g = gl.rotate_converted(*globe, new_ax, ang)
        img = gl.pos_col_to_img(*g, 256, 120)
        img2 = gl.interpolate_picture(img)
        pic_raw.append(img2)

    np.save(f'pic_{file_name}', np.asarray(pic_raw))


def make_raw_pos_col_npy(globe, new_axis, file_name='raw'):
    pic_raw = []
    for i in range(256):
        t = i / 256 - i // 256
        ang = t * np.pi * 2

        g = gl.rotate_converted(*globe, new_axis, ang)

        pic_raw.append(g)

    np.save(f'pos_col_{file_name}', np.asarray(pic_raw))


if __name__ == '__main__':
    # pic = "Earth_relief_120x256.bmp"
    pic = "world_blue_marble_nov2004.bmp"
    img = np.asarray(Image.open(pic))
    globe, orig_ax, new_ax = gl.convert_pic_and_rotate(img, np.deg2rad(90), np.deg2rad(90), np.deg2rad(23.4))
    globe_line_axis = np.array([new_ax * (-1), new_ax]) * 2

    f, ext = os.path.splitext(pic)
    make_raw_bmp_images(globe, new_ax, file_name=f)
    # make_raw_pos_col_npy(globe, new_ax, file_name=f)
