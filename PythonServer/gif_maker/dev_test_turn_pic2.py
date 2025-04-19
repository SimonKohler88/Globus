#!/usr/bin/python3
# -*- coding: utf-8 -*-

import matplotlib.pyplot as plt
import numpy as np
import cv2
from colorsys import hls_to_rgb, rgb_to_hls

import vispy.scene
from vispy.scene import visuals


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



#
# Make a canvas and add simple view
#
canvas = vispy.scene.SceneCanvas(keys='interactive', show=True)
view = canvas.central_widget.add_view()

# generate data
# pos = np.random.normal(size=(100000, 3), scale=0.2)
# one could stop here for the data generation, the rest is just to make the
# data look more interesting. Copied over from magnify.py
# centers = np.random.normal(size=(50, 3))
# indexes = np.random.normal(size=100000, loc=centers.shape[0] / 2,
#                            scale=centers.shape[0] / 3)
# indexes = np.clip(indexes, 0, centers.shape[0] - 1).astype(int)
# symbols = np.random.choice(['o', '^'], len(pos))
# scales = 10**(np.linspace(-2, 0.5, centers.shape[0]))[indexes][:, np.newaxis]
# pos *= scales
# pos += centers[indexes]

# create scatter object and fill in the data
# scatter = visuals.Markers()
# scatter.set_data(pos, edge_width=0, face_color=(1, 1, 1, .5), size=5, symbol=symbols)


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

img_out = np.zeros((img.shape[0], img.shape[1], img.shape[2]), np.uint8)
img_out_inter = np.zeros((img.shape[0], img.shape[1], img.shape[2]), np.uint8)
#
fig = plt.figure()
ax = fig.add_subplot(311)
ax.imshow(img)
ax2 = fig.add_subplot(312)
ax3 = fig.add_subplot(313)

pos = []
col = []
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

        if x == 0 and y == 0:
            globe_axis_orig = np.array([[xB, yB, zB], [-xB, -yB, -zB]])

        qB = Quaternion(0, xB, yB, zB)
        qA = Quaternion(np.cos(0.5 * alpha), xA * np.sin(0.5 * alpha), yA * np.sin(0.5 * alpha),
                        zA * np.sin(0.5 * alpha))
        qAi = qA.conjugate()
        qBr = qA * (qB * qAi)
        phi_n, theta_n = qBr.to_axisangle()
        y_n_2d = int(round(phi_n / np.pi * y_max, 0)) - 1
        x_n_2d = int(round(theta_n / np.pi * x_half, 0)) - 1
        pix = img[y][x]
        img_out[y_n_2d][x_n_2d] = pix
        pix_norm = pix / 255
        x_n, y_n, z_n = qBr.to_xyz()
        if x == 0 and y == 0:
            globe_axis = np.array([[x_n, y_n, z_n], [-x_n, -y_n, -z_n]])
        pos.append((x_n, y_n, z_n))
        col.append(pix_norm)

pos = np.asarray(pos)
col = np.asarray(col)
scatter = visuals.Markers()
scatter.set_data(pos, edge_width=0, face_color=col, size=10, symbol='o')
view.add(scatter)

view.camera = 'turntable'  # or try 'arcball'
# view.camera = 'arcball'  # or try 'arcball'
ln_scale = 2
turn_axis = np.array([[xA, yA, zA], [-xA, -yA, -zA]]) * ln_scale
plot = visuals.Line(turn_axis, parent=view.scene)

plot = visuals.Line(globe_axis_orig * ln_scale, parent=view.scene)

plot = visuals.Line(globe_axis * ln_scale, parent=view.scene)

cube = visuals.Box(2, 2, 2, color=(1, 1, 1, 0), edge_color="white", parent=view.scene)
# add a colored 3D axis for orientation
axis = visuals.XYZAxis(parent=view.scene)

# cv2.imshow("img", img_out)
# cv2.waitKey(0)
# cv2.destroyAllWindows()


black = np.array([0, 0, 0], np.uint8)
# interpolation




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

                else:
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
                if y < 20:
                    print(x, c)
                if c.all() == 0:
                    pix_cnt = x - idx_col2
                    for i in range(pix_cnt):
                        img_out[y][idx_col2 + i] = col2 * 255
    return img_out

img_out_inter = interpolate_picture(img_out)
idx_from = 0
idx_to = 20
# img_out_small = img_out[idx_from:idx_to, idx_from:idx_to]
# ax2.imshow(img_out_small)
ax2.imshow(img_out)
# ax3.imshow(img_out_inter[idx_from:idx_to, idx_from:idx_to])
ax3.imshow(img_out_inter)
plt.show()
# if __name__ == '__main__':
#     import sys
#
#     if sys.flags.interactive != 1:
#         vispy.app.run()
