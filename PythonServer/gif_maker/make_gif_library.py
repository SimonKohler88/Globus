#!/usr/bin/python3
# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
import numpy as np
from colorsys import hls_to_rgb, rgb_to_hls


class Quaternion(object):
    """
    Simplified Quaternion class for rotation of normalized vectors only!
    """

    def __init__(self, q0, qx, qy, qz):
        """
        Internally uses floats to avoid integer division issues.

        @param q0: int or float
        @param qx: int or float
        @param qy: int or float
        @param qz: int or float
        """
        self._q0 = float(q0)
        self._qx = float(qx)
        self._qy = float(qy)
        self._qz = float(qz)
        """
        Note if interpreted as rotation q0 -> -q0 doesn't make a difference
        q0 = cos( w ) so -cos( w ) = cos( w + pi ) and as the rotation
        is by twice the angle it is either 2w or 2w + 2pi, the latter being equivalent to the former.
        """

    def conjugate(q):
        """
        @return Quaternion
        """
        conjq = Quaternion(q._q0, -q._qx, -q._qy, -q._qz)
        return conjq

    def __mul__(q, r):
        """
        Non commutative quaternion multiplication.
        @return Quaternion
        """
        if isinstance(r, Quaternion):
            mq0 = q._q0 * r._q0 - q._qx * r._qx - q._qy * r._qy - q._qz * r._qz
            mqx = q._q0 * r._qx + q._qx * r._q0 + q._qy * r._qz - q._qz * r._qy
            mqy = q._q0 * r._qy - q._qx * r._qz + q._qy * r._q0 + q._qz * r._qx
            mqz = q._q0 * r._qz + q._qx * r._qy - q._qy * r._qx + q._qz * r._q0
            out = Quaternion(mq0, mqx, mqy, mqz)
        else:
            raise TypeError
        return out

    def __getitem__(q, idx):
        """
        @return float
        """
        if idx < 0:
            idx = 4 + idx
        if idx in [0, 1, 2, 3]:
            out = (q._q0, q._qx, q._qy, q._qz)[idx]
        else:
            raise IndexError
        return out

    def to_axisangle(q):
        """returns phi, theta
        phi = arcos(z)
        theta = atan2(y/x)
        """
        return np.arccos(q[3]), np.arctan2(q[2], q[1])

    def to_xyz(q):
        """returns x, y, x"""
        return q[1], q[2], q[3]


def img_to_pos_col(img):
    pos = []
    col = []

    x_max = img.shape[1]
    y_max = img.shape[0]

    for y, row in enumerate(img):
        for x in range(len(row)):
            # normalize y to [-pi/2 ... pi/2]
            y_norm = (y / y_max) * np.pi
            # normalize x to [-pi ... pi]
            x_norm = (x / x_max) * np.pi * 2
            theta_p = y_norm
            phi_p = x_norm
            x_n = np.sin(theta_p) * np.cos(phi_p)
            y_n = np.sin(theta_p) * np.sin(phi_p)
            z_n = np.cos(theta_p)

            pix = img[y][x]
            pix_norm = pix / 255

            pos.append((x_n, y_n, z_n))
            col.append(pix_norm)

    ret = np.asarray([pos, col])
    return ret


def convert_pic_and_rotate(img, theta, phi, alpha):
    """
    1. projects img onto a unit-sphere (Equidistant Cylindrical (Plate Carrée))
    2. rotates the sphere alpha radians around rotation axis given by theta and phi

    original axis  (zero of theta, phi) is taken from coordinates x[0] and ymax/2 of img
    new axis is the rotated original axis

    :param img: numpy array shape (y, x, 3). image to project on sphere and turn
    :param theta: azimut angle of rotation axis, in radians
    :param phi: polar angle of rotation axis, in radians
    :param alpha: angle to rotate sphere around rotation axis
    :return: (np.array, np.array), original axis, new axis
                (pixel-position xyz, color normalized),
    """
    theta_v = theta
    phi_v = phi
    xA = np.sin(theta_v) * np.cos(phi_v)
    yA = np.sin(theta_v) * np.sin(phi_v)
    zA = np.cos(theta_v)

    qA = Quaternion(np.cos(0.5 * alpha), xA * np.sin(0.5 * alpha), yA * np.sin(0.5 * alpha),
                    zA * np.sin(0.5 * alpha))
    qAi = qA.conjugate()

    # if len(img) % 2 != 0 or len(img[0]) % 2 != 0:
    #     raise ValueError("picture must be even sized")

    x_max = img.shape[1]
    y_max = img.shape[0]
    # x_half = x_max / 2
    # y_half = y_max / 2

    pos = []
    col = []

    for y, row in enumerate(img):
        for x in range(len(row)):
            # normalize y to [-pi/2 ... pi/2]
            y_norm = (y / y_max) * np.pi
            # normalize x to [-pi ... pi]
            x_norm = (x / x_max) * np.pi * 2

            # interpret x, y coordinates as angles on a unit sphere
            theta_p = y_norm
            phi_p = x_norm

            # calculate 3d point in Space from angles
            xB = np.sin(theta_p) * np.cos(phi_p)
            yB = np.sin(theta_p) * np.sin(phi_p)
            zB = np.cos(theta_p)

            if x == 0 and y == 0:
                globe_axis_orig = np.array([xB, yB, zB])

            qB = Quaternion(0, xB, yB, zB)

            qBr = qA * (qB * qAi)
            # phi_n, theta_n = qBr.to_axisangle()
            # y_n_2d = int(round(phi_n / np.pi * y_max, 0)) - 1
            # x_n_2d = int(round(theta_n / np.pi * x_half, 0)) - 1
            # img_out[y_n_2d][x_n_2d] = pix
            pix = img[y][x]

            pix_norm = pix / 255
            x_n, y_n, z_n = qBr.to_xyz()
            if x == 0 and y == 0:
                globe_axis = np.array([x_n, y_n, z_n])
            pos.append((x_n, y_n, z_n))
            col.append(pix_norm)
    ret = np.asarray([pos, col])
    return ret, globe_axis_orig, globe_axis


def rotate_converted(pos, col, axis, alpha):
    xA = axis[0]
    yA = axis[1]
    zA = axis[2]
    qA = Quaternion(np.cos(0.5 * alpha), xA * np.sin(0.5 * alpha), yA * np.sin(0.5 * alpha),
                    zA * np.sin(0.5 * alpha))
    qAi = qA.conjugate()

    if len(pos) != len(col):
        raise ValueError("pos and col must have same length")

    pos_n = []
    col_n = []
    for i, (x, y, z) in enumerate(pos):
        xB = x
        yB = y
        zB = z

        qB = Quaternion(0, xB, yB, zB)

        qBr = qA * (qB * qAi)
        # phi_n, theta_n = qBr.to_axisangle()
        # y_n_2d = int(round(phi_n / np.pi * y_max, 0)) - 1
        # x_n_2d = int(round(theta_n / np.pi * x_half, 0)) - 1
        # img_out[y_n_2d][x_n_2d] = pix
        pix = col[i]

        x_n, y_n, z_n = qBr.to_xyz()

        pos_n.append((x_n, y_n, z_n))
        col_n.append(pix)
    ret = np.asarray([pos_n, col_n])
    return ret


def pos_col_to_img(pos, col, size_x, size_y):
    y_max = size_y
    x_half = size_x / 2

    img_out = np.zeros((size_y, size_x, 3), np.uint8)
    for i, (x, y, z) in enumerate(pos):
        pix = col[i]

        phi_n = np.arccos(z)
        theta_n = np.arctan2(y, x)

        y_n = int(round(phi_n / np.pi * y_max, 0)) - 1
        x_n = int(round(theta_n / np.pi * x_half, 0)) - 1
        img_out[y_n][x_n] = pix*255
    return img_out


def interpolate_colors(col1, col2, span):
    col1_hls = rgb_to_hls(*col1)
    col2_hls = rgb_to_hls(*col2)
    hue1 = col1_hls[0]
    hue2 = col2_hls[0]

    lum1 = col1_hls[1]
    lum2 = col2_hls[1]

    sat1 = col1_hls[2]
    sat2 = col2_hls[2]

    hue_diff = hue2 - hue1
    lum_diff = lum2 - lum1
    sat_diff = sat2 - sat1

    h_range = np.linspace(0, hue_diff, span + 1) + hue1
    l_range = np.linspace(0, lum_diff, span + 1) + lum1
    s_range = np.linspace(0, sat_diff, span + 1) + sat1

    cols = [hls_to_rgb(h, l, s) for h, l, s in zip(h_range, l_range, s_range)]
    return np.asarray(cols)[1:]


def interpolate_picture(img_in):
    """
    interpolate rowwise black pixels in between two colored pixels.
    black edge pixels are colored with the first (respectively last) color found.

    interpolation is done by converting the RGB to HSL-values and interpolate Hue, luminance and saturation.

    :param img_in: numpy-array shaped (y,x,3), where 3 is the RGB-channel.
    :return:
    """
    img_out = np.zeros((img_in.shape[0], img_in.shape[1], img_in.shape[2]), np.uint8)
    for y, row in enumerate(img_in):
        col1 = None
        idx_col1 = None
        col2 = None
        idx_col2 = None
        for x in range(len(row)):

            c = img_in[y][x]

            if c.all() != 0:
                c_norm = c / 255
                col1 = col2
                idx_col1 = idx_col2

                col2 = c_norm
                idx_col2 = x

                if col1 is None:  # first colored pixel found
                    for i in range(idx_col2):
                        img_out[y][i] = col2 * 255

                else:  # two colored pixels
                    pix_cnt = idx_col2 - idx_col1 - 1
                    if pix_cnt == 0:
                        img_out[y][idx_col2] = col2 * 255
                        img_out[y][idx_col1] = col1 * 255
                    else:
                        cols = interpolate_colors(col1, col2, pix_cnt)

                        # if x < 20 and y < 20:
                        #     print(col1, col2, pix_cnt)
                        #     print(cols)
                        #     print(cols[0], cols[-1])
                        #     print(pix_cnt, len(cols))
                        img_out[y][idx_col2] = col2 * 255
                        img_out[y][idx_col1] = col1 * 255
                        for i, col in enumerate(cols):
                            img_out[y][idx_col1 + i + 1] = col * 255

            if x == len(row) - 1:  # end of picture
                if c.all() == 0:
                    pix_cnt = x - idx_col2
                    for i in range(pix_cnt):
                        img_out[y][idx_col2 + i] = col2 * 255
    return img_out


if __name__ == '__main__':
    pass
