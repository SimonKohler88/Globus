#!/usr/bin/python3
# -*- coding: utf-8 -*-
from http.server import BaseHTTPRequestHandler, HTTPServer
import json
import cgi
import time
hostName = "192.168.137.1"
serverPort = 8123



import cv2

FRAME_SIZE = (120, 256)

t_last = None

# process the video files
# video_data = process_videos("movies", FRAME_SIZE)

pic = "Earth_relief_120x256.jpg"
pic_bmp = "Earth_relief_120x256.bmp"
pic2 = "Earth_relief_120x256.jpeg"


# img = cv2.imread(pic_bmp, 0)  # works, but ts greyscale
img = cv2.imread(pic_bmp, cv2.IMREAD_COLOR_RGB)
# esp_jpeg decompressor in ESP only takes YCrCb Color Space
img = cv2.cvtColor(img, cv2.COLOR_RGB2YCrCb)
# Compress, 90% quality
encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 90]
_, buffer = cv2.imencode(".jpg", img, encode_param)
pic3 = buffer.tobytes()

class MyServer(BaseHTTPRequestHandler):
    def do_GET(self):

        global t_last
        t_now = time.time()

        if t_last is not None:
            t_delta = t_now - t_last
            if t_delta > 4:
                with open(f'log_out/Missed_{t_now}', 'w') as f:
                    pass
                print('missed')
        t_last = t_now


        self.send_response(200)
        self.send_header('Content-type', 'image/jpeg')
        self.end_headers()
        data = json.dumps({'wifi period': '60', 'poll period': '1', '#':'1'})
        self.wfile.write(pic3)



if __name__ == "__main__":
    webServer = HTTPServer((hostName, serverPort), MyServer)
    print("Server started http://%s:%s" % (hostName, serverPort))

    try:
        webServer.serve_forever()
    except KeyboardInterrupt:
        pass

    webServer.server_close()
    print("Server stopped.")