#!/usr/bin/python3
# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
import numpy as np
import cv2


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
        """returns phi, theta"""
        return np.arccos(q[3]), np.arctan2(q[2], q[1])

    def to_xyz(q):
        """returns x, y, x"""
        return q[1], q[2], q[3]


if __name__ == '__main__':
    pic_bmp = "Earth_relief_120x256.bmp"
    img = cv2.imread(pic_bmp, cv2.IMREAD_COLOR_RGB)
    print(img.shape)

    theta_v = np.deg2rad(90)
    phi_v = np.deg2rad(90)
    xA = np.sin(theta_v) * np.cos(phi_v)
    yA = np.sin(theta_v) * np.sin(phi_v)
    zA = np.cos(theta_v)

    alpha = np.deg2rad(23.4)
    x_max = img.shape[1]
    y_max = img.shape[0]
    x_half = x_max / 2
    y_half = y_max / 2

    img_out = np.zeros((img.shape[0] * 2, img.shape[1] * 2, img.shape[2]), np.uint8)

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    for y, row in enumerate(img):
        for x in range(len(row)):
            # normalize y to [-pi/2 ... pi/2]
            y_norm = ((y) / y_max) * (np.pi)
            # normalize x to [-pi ... pi]
            x_norm = ((x) / x_max) * np.pi * 2

            theta_p = y_norm
            phi_p = x_norm
            xB = np.sin(theta_p) * np.cos(phi_p)
            yB = np.sin(theta_p) * np.sin(phi_p)
            zB = np.cos(theta_p)
            qB = Quaternion(0, xB, yB, zB)
            qA = Quaternion(np.cos(0.5 * alpha), xA * np.sin(0.5 * alpha), yA * np.sin(0.5 * alpha),
                            zA * np.sin(0.5 * alpha))
            qAi = qA.conjugate()
            qBr = qA * (qB * qAi)
            # phi_n, theta_n = qBr.to_axisangle()
            # y_n = int(phi_n / np.pi * y_max)
            # x_n = int(theta_n / np.pi * x_max)
            pix = img[y][x]
            # img_out[y_n][x_n] = pix

            x_n, y_n, z_n = qBr.to_xyz()
            col = f'#{pix[0]:02x}{pix[1]:02x}{pix[2]:02x}'
            ax.plot([x_n], [y_n], [z_n], marker='o', color=col)

            # pix = img[y_n][x_n]
            # img_out[y][x] = pix
            # print(x_norm, y_norm, x_n, y_n)

    plt.show()
    # plt.imshow(img_out)
