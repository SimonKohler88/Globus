#!/usr/bin/python3
# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D


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
    theta = np.deg2rad(90)  # axis
    phi = np.deg2rad(90)
    xA = np.sin(theta) * np.cos(phi)
    yA = np.sin(theta) * np.sin(phi)
    zA = np.cos(theta)

    Theta = np.deg2rad(45)
    Phi = np.deg2rad(-45)
    xB = np.sin(Theta) * np.cos(Phi)
    yB = np.sin(Theta) * np.sin(Phi)
    zB = np.cos(Theta)

    qB = Quaternion(0, xB, yB, zB)

    cX = [xB]
    cY = [yB]
    cZ = [zB]

    for alpha in np.linspace(0.1, 6, 20):
        qA = Quaternion(np.cos(0.5 * alpha), xA * np.sin(0.5 * alpha), yA * np.sin(0.5 * alpha),
                        zA * np.sin(0.5 * alpha))
        qAi = qA.conjugate()
        qBr = qA * (qB * qAi)
        cX += [qBr[1]]
        cY += [qBr[2]]
        cZ += [qBr[3]]
        print(qBr.to_axisangle())

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    u = np.linspace(0, 2 * np.pi, 50)
    v = np.linspace(0, np.pi, 25)
    x = .9 * np.outer(np.cos(u), np.sin(v))
    y = .9 * np.outer(np.sin(u), np.sin(v))
    z = .9 * np.outer(np.ones(np.size(u)), np.cos(v))

    ax.plot_wireframe(x, y, z, color='g', alpha=.3)
    ax.plot([0, xA], [0, yA], [0, zA], color='r')
    ax.plot([0, xB], [0, yB], [0, zB], color='b')
    ax.plot(cX, cY, cZ, color='b')

    plt.show()
