#!/usr/bin/python3
# -*- coding: utf-8 -*-

import time  # noqa
from math import sin, pi

from PyQt5 import QtWidgets, QtCore

from vispy.scene import SceneCanvas, visuals
from vispy.app import use_app, Timer

import make_gif_library as gl
import numpy as np
from PIL import Image


class CanvasWrapper:
    def __init__(self):
        self.canvas = SceneCanvas(keys='interactive', show=True)
        self.view = self.canvas.central_widget.add_view()
        self.view.camera = 'turntable'
        self.scatter = visuals.Markers()
        self.globe_axis = visuals.Line()
        self.view.add(self.scatter)
        self.view.add(self.globe_axis)

    def update_data(self, new_data_dict):
        col = new_data_dict['col']
        pos = new_data_dict['pos']
        ax = new_data_dict['axis']
        self.scatter.set_data(pos, edge_width=0, face_color=col, size=10, symbol='o')
        self.globe_axis.set_data(ax, color='r')


class MyMainWindow(QtWidgets.QMainWindow):
    def __init__(self, canvas_wrapper: CanvasWrapper, *args, **kwargs):
        super().__init__(*args, **kwargs)

        central_widget = QtWidgets.QWidget()
        main_layout = QtWidgets.QHBoxLayout()

        self._canvas_wrapper = canvas_wrapper
        main_layout.addWidget(self._canvas_wrapper.canvas.native)

        central_widget.setLayout(main_layout)
        self.setCentralWidget(central_widget)


class DataSource(QtCore.QObject):
    """Object representing a complex data producer."""

    new_data = QtCore.pyqtSignal(dict)

    def __init__(self, num_iterations=1000, parent=None):
        super().__init__(parent)
        self._count = 0
        self._num_iters = num_iterations

        pic = "Earth_relief_120x256.bmp"
        img = np.asarray(Image.open(pic))
        self.globe, self.orig_ax, self.new_ax = gl.convert_pic_and_rotate(img, np.deg2rad(90), np.deg2rad(90),
                                                                          np.deg2rad(23.4))
        self.globe_line_axis = np.array([self.new_ax * (-1), self.new_ax]) * 2

    def run_data_creation(self, timer_event):
        if self._count >= self._num_iters:
            return

        t = self._count / 256 - self._count // 256
        ang = t * np.pi * 2

        g = gl.rotate_converted(*self.globe, self.new_ax, ang)
        img = gl.pos_col_to_img(*g, 256, 120)
        img2 = gl.interpolate_picture(img)
        pos, col = gl.img_to_pos_col(img2)

        data_dict = {'pos': pos, 'col': col, 'axis': self.globe_line_axis }

        self._count += 1

        self.new_data.emit(data_dict)


if __name__ == "__main__":
    app = use_app("pyqt5")
    app.create()
    data_source = DataSource()
    canvas_wrapper = CanvasWrapper()
    win = MyMainWindow(canvas_wrapper)

    data_source.new_data.connect(canvas_wrapper.update_data)
    # Change "1.0" to "auto" to run connected function as quickly as possible
    # timer = Timer("1.0", connect=data_source.run_data_creation, start=True)
    timer = Timer("0.2", connect=data_source.run_data_creation, start=True)

    # stop the timer when the window is closed and destroyed
    # not always needed, but needed for vispy gallery creation
    win.destroyed.connect(timer.stop)

    win.show()
    app.run()
