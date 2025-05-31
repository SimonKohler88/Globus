from flask import Flask, Response, jsonify, render_template
# from video_server.video_preprocessor import process_videos, get_video_data
# from PIL import Image
# import numpy as np
import time
import os

import cv2

import globus_video_utils.video_processor as video_processor

gif_path = os.path.join(os.path.dirname(__file__), 'gifs')

vids = video_processor.VideoPlayer(gif_path)
vids.play()

app = Flask(__name__, static_url_path='/static')

FRAME_SIZE = (120, 256)

t_last = None

# process the video files
# video_data = process_videos("movies", FRAME_SIZE)

# pic = "Earth_relief_120x256.jpg"
pic_bmp = "Earth_relief_120x256.bmp"
# pic2 = "Earth_relief_120x256.jpeg"


# img = cv2.imread(pic_bmp, 0)  # works, but ts greyscale
img = cv2.imread(pic_bmp)
# img = cv2.cvtColor(img, cv2.IMREAD_COLOR_RGB)
# esp_jpeg decompressor in ESP only takes YCrCb Color Space
# img = cv2.cvtColor(img, cv2.COLOR_RGB2YCrCb)
# img = cv2.cvtColor(img, cv2.COLOR_BGR2YCrCb)
# Compress, 90% quality
encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 90]
_, buffer = cv2.imencode(".jpg", img, encode_param)
pic3 = buffer.tobytes()


# plt.imshow(img, cmap='gray')
# we don't need more than the framerate
#     encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 50]
#     _, buffer = cv2.imencode(".jpg", frame, encode_param)
#     video_jpegs.append((frame_ms, buffer.tobytes()))
# #
# @app.route('/channel_info')
# def get_channel_lengths():
#     lengths = [len(audio) for audio, frames in video_data]
#     return jsonify(lengths)
#
#
# @app.route('/audio/<int:channel_index>/<int:start>/<int:length>')
# def get_audio(channel_index, start, length):
#     audio, frames = video_data[channel_index % len(video_data)]
#     end = start + length
#     if end > len(audio):
#         end = len(audio)
#     if start > end:
#         start = end
#     if start == end:
#         # return nothing - we've got no more data to give
#         return Response(b'', mimetype='audio/x-raw')
#     slice = audio[start:end]
#     return Response(slice, mimetype='audio/x-raw')
#

# @app.route('/frame/<int:channel_index>/<int:ms>')
# def get_frame(channel_index, ms):
#     audio, frames = video_data[channel_index % len(video_data)]
#     # use binary search to find the closest frame
#     start = 0
#     end = len(frames) - 1
#     while start <= end:
#         mid = (start + end) // 2
#         if frames[mid][0] == ms:
#             return Response(frames[mid][1], mimetype='image/jpeg')
#         elif frames[mid][0] < ms:
#             start = mid + 1
#         else:
#             end = mid - 1
#     # we may not find the exact frame, so return the closest frame
#     if end < 0:
#         end = 0
#     elif start >= len(frames):
#         start = len(frames) - 1
#     return Response(frames[start][1], mimetype='image/jpeg')

@app.route('/')
def home():
    return render_template('index.html')


# DEV_MODE = 0  # static pic
DEV_MODE = 1  # video


@app.route('/frame/<int:delta_t_ms>')
def get_frame(delta_t_ms):
    global t_last
    t_now = time.time()
    if t_last is not None:
        t_delta = t_now - t_last
        if t_delta > 4:
            with open(f'log_out/Missed_{t_now}', 'w') as f:
                pass
            print('missed')
    t_last = t_now

    if DEV_MODE == 0:
        return Response(pic3, mimetype='image/jpeg')
    else:
        pic = vids.get_frame(1)
        return Response(pic, mimetype='image/jpeg')


if __name__ == '__main__':
    # app.run(host='0.0.0.0', port=8123)
    # app.run(host = '192.168.3.1', port=1234)
    app.run(host='192.168.0.22', port=80)
    # app.run(host = '192.168.137.1', port=8123)
