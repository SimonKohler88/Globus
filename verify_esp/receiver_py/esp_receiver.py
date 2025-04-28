#!/usr/bin/python3
# -*- coding: utf-8 -*-

import socket
import threading
import time
import math
import random
from collections import deque
import ctypes as ct
import struct
from PIL import Image
import numpy as np
from bitarray import bitarray

DATA_SIZE_PACKET = 1024


def packethook(*args):
    print(args)


WHERE = 1
if __name__ == '__main__':
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    if WHERE == 1:
        my_ip = '192.168.0.22'
        ip = '192.168.0.72'
    elif WHERE == 2:
        my_ip = '192.168.137.1'
        ip = '192.168.137.89'

    my_port = 1234

    s.bind((my_ip, my_port))
    port = 1234
    received_data = None

    # flush receiving buffer before starting
    # while True:
    #     try:
    #         received_data, addr = s.recvfrom(1024)  # Timeout programmieren?
    #         if not received_data:
    #             break
    #     except TimeoutError:
    #         break
    print('Start listening')
    while True:
        received_data, addr = s.recvfrom(2048)  # Timeout programmieren?
        if received_data:
           print(received_data)
        else:
            time.sleep(0.1)
        received_data = None
