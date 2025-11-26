#!/usr/bin/python3
# -*- coding: utf-8 -*-

import cv2

JPEG_ENCODE_PARAM = [int(cv2.IMWRITE_JPEG_QUALITY), 80]



# Custom JPEG Compression Parameter override.
# Read from Video class, if name in dict-> override.
# Use with caution. To achieve high framerate without picture-twitching, the size of an image should be less than
# 10 kB. Final Size of a compressed JPEG is dependent on the amount of colors/edges in the picture.
# Example: in fhnw_double_logo_small.gif (2 colors, simple pictures) the sizes with 80% compression quality is
# between 3441 and 3668 Bytes, in globe.gif between 9719  and 10116.
# fhnw_double_logo_small.gif with 100% quality is between 7883 and 8875.
# The info is printed from Video when loading, use development server to show.
JPEG_PARM_CUSTOM_OVERRIDE = {
    'fhnw_double_logo_small': [int(cv2.IMWRITE_JPEG_QUALITY), 100],
    'fhnw_double_logo_small_dark1': [int(cv2.IMWRITE_JPEG_QUALITY), 100],
}

if __name__ == '__main__':
    pass
