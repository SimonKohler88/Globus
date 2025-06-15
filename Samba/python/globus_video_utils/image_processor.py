#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import cv2
import numpy as np


def scale_crop_image(image_jpeg):
    """
    opens and scales picture down to 120x256x3
    aspect ratio is preserved.
    scales img down to 120 px height, then
    expands/crops pixels left and right to fit dimensions

    adapts Grayscale Images to 120x256x3


    :param image_jpeg: Absolute Path to jpeg, png
    :return: numpy array, 120x256x3
    """
    if not os.path.exists(image_jpeg):
        raise FileExistsError(f'Image not existing: {image_jpeg}')
    img = cv2.imread(image_jpeg)

    if len(img.shape) == 2:
        img = cv2.cvtColor(img, cv2.COLOR_GRAY2RGB)

    print(img.shape)
    # Syntax: cv2.resize(source, dsize, dest, fx, fy, interpolation)
    # keep aspect ratio
    y_size = 120
    y_factor = y_size / img.shape[0]
    x_size = int(round(img.shape[1] * y_factor))

    img_resized = cv2.resize(img, (x_size, y_size))

    # print(img_resized.shape, expand_x)
    print(img_resized.shape)

    # extend to 120x256
    if img_resized.shape[1] < 256:
        print('expand')
        arr = np.zeros((120, int((256 - img_resized.shape[1]) / 2), 3), dtype=np.uint8)
        img_expanded = np.append(arr, img_resized, axis=1)
        img_expanded = np.append(img_expanded, arr, axis=1)
        if img_expanded.shape[1] != 256:
            n = 256 - img_expanded.shape[1]
            arr = np.zeros((120, n, 3), dtype=np.uint8)
            img_expanded = np.append(img_expanded, arr, axis=1)
    elif img_resized.shape[1] > x_size:
        print('crop')
        n = img_resized.shape[1] - x_size
        img_expanded = img_resized[:, :-n, :]
    else:
        img_expanded = img_resized

    print(img_expanded.shape)
    # TODO: if grayscale, extend dimension
    return img_expanded
    # return img_resized


def gifify(image_np, output_name, col_shift):
    """
    creates a gif in this directory with given name which shifts the
    picture a col_shift column per frame

    :param image_np: numpy array with dimension 120x256x3
    :return: gif, image shifted 1 column per frame
    """

    if type(image_np) != np.ndarray:
        raise TypeError('image_np must be a numpy array')
    if image_np.shape != (120, 256, 3):
        raise ValueError(
            f'Image size is not 120x256x3, w:{image_np.shape[1]}, h:{image_np[0]}, pix bytes: {image_np.shape[2]}')
    height = image_np.shape[0]
    width = image_np.shape[1]

    # cap = cv2.VideoWriter()
    # cap = cv2.VideoCapture()
    # for i in range(image_np.shape[1]):
    #     np.roll()


if __name__ == '__main__':
    p = r'D:\Downloads'
    # pic = os.path.join(p, 'Black_Panther3.jpg')
    pic = os.path.join(p, 'kittyKat.jpg')
    print(pic)
    print(os.path.exists(pic))

    img = scale_crop_image(pic)

    cv2.imshow('frame', img)
    cv2.waitKey(0)
