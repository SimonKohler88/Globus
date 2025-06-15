#!/usr/bin/python3
# -*- coding: utf-8 -*-

import time  # noqa
import imageio

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

        self.cube = visuals.Box(2, 2, 2, color=(1, 1, 1, 0), edge_color="white")
        self.view.add(self.cube)

        # self.z_axis = visuals.Line([[0, 0, -1], [0, 0, 1]])
        # self.view.add(self.z_axis)
        angle_rad = np.deg2rad(23.4)
        self.spin_axis = visuals.Line([[0, 0, 0], [2*np.sin(angle_rad),0, 2*np.cos(angle_rad)]])
        self.view.add(self.spin_axis)


    def update_data(self, new_data_dict):
        col = new_data_dict['col']
        pos = new_data_dict['pos']
        # ax = new_data_dict['axis']
        self.scatter.set_data(pos, edge_width=0, face_color=col, size=10, symbol='o')
        # self.globe_axis.set_data(ax, color='r')


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

        # arr_file = 'pos_col_raw.npy'  # for raw pos col npy
        # arr_file = 'globe_world_blue_marble_nov2004.gif'  # for .gif
        # self.pics = imageio.get_reader(arr_file)  #for gif

        arr_file = 'pic_world_blue_marble_nov2004.npy'  # for img npy
        self.pics = np.load(arr_file)  # for npy files

    def run_data_creation(self, timer_event):
        i = self._count

        # img = self.pics.get_data(i)  # for gifs
        img = self.pics[i]  # for .npy

        pos, col = gl.img_to_pos_col(img)

        data_dict = {'pos': pos, 'col': col}

        self._count += 1
        if self._count >= 256:
            self._count = 0
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
    timer = Timer("0.05", connect=data_source.run_data_creation, start=True)

    # stop the timer when the window is closed and destroyed
    # not always needed, but needed for vispy gallery creation
    win.destroyed.connect(timer.stop)

    win.show()
    app.run()
