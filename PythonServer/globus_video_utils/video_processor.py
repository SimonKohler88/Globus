#!/usr/bin/python3
# -*- coding: utf-8 -*-


import os
import cv2
import numpy as np
import shutil
import PIL


class VideoPlayer:
    """
    Represents a video player capable of managing and playing video files.

    This class allows for the initialization with a directory path containing
    videos, serving as its video source. It provides functionality for managing
    playback, including loading, playing, pausing, and stopping videos.

    :ivar video_directory_path: Path to the directory containing video files.
    :type video_directory_path: str
    :ivar current_video: The video currently loaded for playback.
    :type current_video: Optional[str]
    :ivar is_playing: Indicates whether a video is currently playing.
    :type is_playing: bool
    """

    def __init__(self, video_directory_path):
        # current idx of active video to get frames from
        self.__current_video_idx = 0
        # whether we increase frame idx on "get frame" or not
        self.__is_playing = False
        # remember frame index
        self.__current_frame_idx = 0
        # save against concurent access
        self.__lock = False
        self.__locked_video_idx = 0
        self.__locked_frame_index = 0

        vid_lst = os.listdir(video_directory_path)

        # list of Video -objects
        self.__videos: list[Video] = [Video(os.path.join(video_directory_path, v)) for v in vid_lst]

    def change_video(self, video_name_str, initial_frame):
        """

        :param video_name_str:
        :param initial_frame: None, int: if None, take Same as before. int: explicit start index
        :return:
        """
        names = self.get_available_video_names()
        if video_name_str in names:
            idx = names.index(video_name_str)
            self.__locked_frame_index = self.__current_frame_idx
            self.__locked_video_idx = self.__current_video_idx

            self.__lock = True
            self.__current_frame_idx = initial_frame if initial_frame is not None else self.__current_frame_idx
            self.__current_video_idx = idx
            self.__lock = False

    def get_available_video_names(self):
        """
        Returns a list of available video Names processed
        :return:
        """
        return [v.name for v in self.__videos]

    def get_frame(self, next_frame):
        """

        :param next_frame: bool, True for next frame, False for current frame
        :return: bytes, ready to send by Flask
        """
        if self.__lock:
            return self.__videos[self.__locked_video_idx].get_frame(self.__locked_frame_index)
        else:
            frame, self.__current_frame_idx = self.__videos[self.__current_video_idx].get_frame_circular(
                self.__current_frame_idx)
            if next_frame > 0 and self.__is_playing:
                self.__current_frame_idx += 1

            return frame

    def play(self):
        """
        increase frame index on every "get_frame" call
        :return:
        """
        self.__is_playing = True

    def pause(self):
        """
        Dont increase frame index on every "get_frame" call
        :return:
        """
        self.__is_playing = False


class Video:
    """
    Represents a video object.

    This class preprocesses given .gif ready for esp.

    :param: video_path: absolute Path to the video.
    """

    def __init__(self, video_path_abs):

        self.__frames = []
        self.__name = 'No Vid'
        self.__total_frames = 0
        self.__load_video(video_path_abs)

    @property
    def name(self):
        """
        Returns the name of the video, without the extension and without the path
        :return:
        """
        return self.__name

    def __load_video(self, video_path):
        """
         * opens given .gif
         * converts all images to YcrCb color space
         * compressing to jpeg 90%
         * saves as bytes, ready for esp

        :param video_path: absolute path to video
        :return: nix
        """
        cap = cv2.VideoCapture(video_path)
        self.__name, _ = os.path.splitext(os.path.basename(video_path))
        print(f"Video Preprocessing: {self.__name}")
        self.__total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
        print(f'Frames: {self.__total_frames}')

        # # ********* testing part
        # test_directory = os.path.join(os.path.dirname(__file__), 'test', self.__name)
        # if os.path.exists(test_directory):
        #     shutil.rmtree(test_directory)
        # if not os.path.exists('test'):
        #     os.mkdir('test')
        # os.mkdir(test_directory)
        # # ********************

        encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 80]

        for i in range(self.__total_frames):
            frame_exists, frame = cap.read()
            if frame_exists:
                # img = cv2.imread(frame)
                img = frame
                # img = cv2.cvtColor(img, cv2.COLOR_BGR2YCrCb)

                # Compress, 90% quality
                _, buffer = cv2.imencode(".jpg", img, encode_param)
                self.__frames.append(buffer.tobytes())

                # ********************* testing
                # self.__frames.append(buffer)
                # test_file_name = f'{self.__name}_{i}.jpg'
                #
                # cv2.imwrite(os.path.join(test_directory, test_file_name), img, [int(cv2.IMWRITE_JPEG_QUALITY), 90])
                # *********************
                print('.', end='')
        print('done')

    def get_frame_circular(self, idx):
        """
        wraps idx around "total frames" if > than total frames
        :param idx: frame to get
        :return: frame, circular wraped around index
        """
        if idx >= self.__total_frames:
            idx_n = idx - (idx // self.__total_frames) * self.__total_frames
            pass
        else:
            idx_n = idx

        return self.__frames[idx_n], idx_n

    def get_frame(self, idx):
        """
        returns frame at index idx, raises IndexError if idx is out of range
        :param idx:
        :return:
        """
        if idx < self.__total_frames:
            return self.__frames[idx]
        raise IndexError("Index out of range")


if __name__ == '__main__':
    directory = os.path.join(os.path.dirname(os.path.dirname(__file__)), 'gifs')
    vid = VideoPlayer(directory)
    print(vid.get_available_video_names())
    vid.play()

    for i in range(10):
        pic = vid.get_frame(1)
        print(f'-------- {i:03} --------')
        print(type(pic), len(pic))
        # open_jpeg = cv2.imdecode(pic, cv2.IMREAD_COLOR)
        open_jpeg = cv2.imdecode(pic, cv2.IMREAD_UNCHANGED)

        # open_jpeg = cv2.cvtColor(open_jpeg, cv2.COLOR_YCrCb2RGB)
        # open_jpeg = cv2.cvtColor(open_jpeg, cv2.COLOR_BGR2RGB)
        print(open_jpeg.shape)
        cv2.imshow('frame', open_jpeg)
        cv2.waitKey(0)
