#!/usr/bin/python3
# -*- coding: utf-8 -*-

import socket
import threading
import time
import math
import tftpy
import random
from collections import deque
import ctypes as ct
import struct
from PIL import Image
import numpy as np

# DATA_SIZE_PACKET = 1024
DATA_SIZE_PACKET = 1400


# S\x02\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00w\x03\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00w\x03\x00\x00
class EspStatus(ct.Structure):
    # class EspStatus(ct.BigEndianStructure):
    _fields_ = [
        # FIFO status
        ("ready_4_fpga_frames", ct.c_uint8),
        ("free_frames", ct.c_uint8),
        ("current_transfer_2_esp", ct.c_uint8),
        ("current_transfer_2_fpga", ct.c_uint8),

        # QSPI status
        ("missed_frames", ct.c_uint16),
        ("missed_spi_transfers", ct.c_uint16),
        # FPGA Control
        ("leds", ct.c_uint32),
        ("width ", ct.c_uint32),
        ("height", ct.c_uint32),
        ("brightness", ct.c_uint32),
        ("current_speed", ct.c_uint32)
    ]


class PicStream:
    def __init__(self, pic):
        img = np.asarray(Image.open(pic))
        self.pic_raw = img.flatten().tobytes()
        # with open(pic, 'rb') as f:
        #     self.pic_raw = f.read()

        print('pic size: ', len(self.pic_raw), ' bytes')
        self.current_byte_pos = 0
        self.closed = False

    def open(self):
        self.closed = False
        self.current_byte_pos = 0

    def read(self, block_size):
        # print(self.current_byte_pos, block_size)
        to_read = min(block_size, len(self.pic_raw[self.current_byte_pos:]))
        to_send = self.pic_raw[self.current_byte_pos: self.current_byte_pos + to_read]
        if to_read != block_size:
            self.current_byte_pos = 0
        else:
            self.current_byte_pos += to_read
        return to_send

    def close(self):
        self.current_byte_pos = 0
        self.closed = True

    def size(self):
        return len(self.pic_raw)

    def __len__(self):
        return len(self.pic_raw)


def packethook(*args):
    print(args)


WHERE = 2
if __name__ == '__main__':
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    if WHERE == 1:
        my_ip = '192.168.0.22'
        ip = '192.168.0.72'
    elif WHERE == 2:
        my_ip = '192.168.137.1'
        ip = None
        ip = '192.168.137.244'

    my_port = 1234

    # s.settimeout(0.1)

    s.bind((my_ip, my_port))
    i = 0
    # s.sendto(f"CS".encode(), ('192.168.0.72', 1234))
    pic_stream = PicStream("Earth_relief_120x256.bmp")
    with open("Earth_relief_120x256.bmp", "rb") as fd:
        picture = fd.read()
    frame_start_time = 0
    frame_end_time = 0
    current_index = 0
    client = None
    port = 1234
    _time = time.time()
    received_data = None

    # flush receiving buffer before starting
    s.settimeout(0.01)
    while True:
        try:
            received_data, addr = s.recvfrom(128)  # Timeout programmieren?
            if not received_data:
                break
        except TimeoutError:
            break
    s.settimeout(0.5)
    print('Start listening', my_ip, my_port)
    while True:
        if (time.time() - _time) >= 10:
            _time = time.time()
            if ip is not None:
                s.sendto(f"CS".encode(), (ip, port))

        try:
            received_data, addr = s.recvfrom(256)  # Timeout programmieren?
        except TimeoutError:
            pass

        if received_data and received_data[0] == 0x53:  # its S for status
            # print(received_data, len(received_data))
            byte_arr = bytearray(received_data)
            # byte_arr.append(0x00)
            stat = EspStatus.from_buffer_copy(byte_arr[1:])
            stat_str = ' '.join([f'{field[0]}:{getattr(stat, field[0])}' for field in stat._fields_])
            # print(byte_arr)
            print(stat_str)
            time.sleep(0.5)

        elif received_data:
            ip, port = addr
            client = tftpy.TftpClient(ip, my_port, options={'blksize': DATA_SIZE_PACKET})  # , localip=my_ip)
            print('Receiving CMD')
            ans = received_data.decode()
            # print(ans)
            if ans == 'FRAME':
                try:
                    frame_start_time = time.time()
                    client.upload('f', pic_stream)  # , packethook=packethook)
                    t = time.time() - frame_start_time
                    print(f'Frame time taken: {t}')
                except tftpy.TftpShared.TftpTimeout:
                    pass
                except tftpy.TftpShared.TftpException as e:
                    print('TFTP Exception: ', e)
                    pic_stream.close()
            #     i = 1
            #     print('frame')
            #     frame_start_time = time.time()
            #     current_index = 0
            #     s.sendto(b"D" + picture[current_index:current_index + DATA_SIZE_PACKET], addr)
            #     current_index += DATA_SIZE_PACKET
            #
            # elif ans == 'OK':
            #     len_left = len(picture[current_index:])
            #     len2_send = min(DATA_SIZE_PACKET, len_left)
            #     s.sendto(b"D" + picture[current_index:current_index + len2_send], addr)
            #     frame_end_time = time.time()
            #     i += 1
            #     if len2_send < DATA_SIZE_PACKET:
            #         t = frame_end_time - frame_start_time
            #         print(f'Frame time_taken: {t}')
            #         print(i)
            #         print(f'Frame size: {len(picture)}')
            #     current_index += len2_send
            #
            # s.sendto(f"CS".encode(), (ip, port))

        else:
            time.sleep(0.5)
        received_data = None
