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

DATA_SIZE_PACKET = 1024


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
        with open(pic, 'rb') as f:
            self.pic_raw = f.read()

        print('pic size: ', len(self.pic_raw), ' bytes')
        self.current_byte_pos = 0
        self.closed = False

    def open(self):
        self.closed = False
        self.current_byte_pos = 0

    def read(self, block_size):
        to_read = min(block_size, len(self.pic_raw[self.current_byte_pos:]))
        to_send = self.pic_raw[self.current_byte_pos: self.current_byte_pos + to_read]
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


if __name__ == '__main__':
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # my_ip = '192.168.0.22'
    my_ip = '192.168.137.1'
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
    print('Start listening')
    client = None
    port = 1234
    # ip = '192.168.0.72'
    ip = '192.168.137.89'
    _time = time.time()
    s.settimeout(0.5)
    received_data = None
    while True:
        if (time.time() - _time) >= 1:
            _time = time.time()
            s.sendto(f"CS".encode(), (ip, port))
        try:
            received_data, addr = s.recvfrom(128)  # Timeout programmieren?
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
            client = tftpy.TftpClient(ip, my_port, options={'blksize': 1024})  # , localip=my_ip)
            print('Receiving CMD')
            ans = received_data.decode()
            print(ans)
            if ans == 'FRAME':
                try:
                    frame_start_time = time.time()
                    client.upload('f', pic_stream)  # , packethook=packethook)
                    t = time.time() - frame_start_time
                    print(f'Frame time taken: {t}')
                except tftpy.TftpShared.TftpTimeout:
                    pass
                except tftpy.TftpShared.TftpException:
                    print('Unknown Transfer ID')
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
