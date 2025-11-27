#!/usr/bin/python3
# -*- coding: utf-8 -*-

import os
import cv2
import numpy as np
from PIL import Image


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
    img = cv2.imread(image_jpeg, cv2.IMREAD_COLOR_RGB)

    if len(img.shape) == 2:
        img = cv2.cvtColor(img, cv2.COLOR_GRAY2RGB)

    print('Incoming Image shape: ', img.shape)
    # Syntax: cv2.resize(source, dsize, dest, fx, fy, interpolation)
    # keep aspect ratio
    y_size = 120
    y_factor = y_size / img.shape[0]
    x_size = int(np.round(img.shape[1] * y_factor))

    img_resized = cv2.resize(img, (x_size, y_size))

    # print(img_resized.shape, expand_x)
    print('Image resized shape: ', img_resized.shape)

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

    elif img_resized.shape[1] > 256:
        n = np.abs(img_resized.shape[1] - 256)
        print(f'cropping {n} cols')
        img_expanded = img_resized[:, :-n, :]
    else:
        img_expanded = img_resized

    print('final Image dimensions: ', img_expanded.shape)
    # TODO: if grayscale, extend dimension
    return img_expanded
    # return img_resized


def gifify(image_np, output_name):
    """
    creates a gif in this directory with given name which shifts the
    picture a col_shift column per frame

    :param image_np: numpy array with dimension 120x256x3
    :param output_name: name of the output gif file
    :return: gif, image shifted 1 column per frame
    """

    if type(image_np) != np.ndarray:
        raise TypeError('image_np must be a numpy array')
    if image_np.shape != (120, 256, 3):
        raise ValueError(
            f'Image size is not 120x256x3, w:{image_np.shape[1]}, h:{image_np.shape[0]}, pix bytes: {image_np.shape[2]}')
    height = image_np.shape[0]
    width = image_np.shape[1]

    # Create list to store PIL Image frames
    frames = []

    print('Gifify: start rolling')
    # Generate frames by rolling columns
    for i in range(width):
        # Roll the image by i columns
        rolled_image = np.roll(image_np, shift=i, axis=1)
        # Convert numpy array to PIL Image
        pil_image = Image.fromarray(rolled_image.astype('uint8'))
        frames.append(pil_image)

    # Write frames to gif
    if not output_name.endswith('.gif'):
        output_file = output_name + '.gif'
    else:
        output_file = output_name

    frames[0].save(output_file, save_all=True, append_images=frames[1:],
                   duration=33, loop=0)

    print(f"GIF saved as {output_file}")


if __name__ == '__main__':
    p = r'D:\Downloads'
    # pic = os.path.join(p, 'Black_Panther3.jpg')
    pic = os.path.join(p, 'fhnw_double_logo_small_dark1.bmp')

    img = cv2.imread(pic, cv2.IMREAD_COLOR_RGB)

    gifify(img, os.path.join(p, 'test.gif'))

