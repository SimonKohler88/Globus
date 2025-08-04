import time
import os

import cv2
import globus_video_utils.video_processor as video_processor
import globus_video_utils.image_processor as img_proc

GIF_PATH = os.path.join(os.path.dirname(__file__), 'gifs')

USE_STATIC_PIC = True


class GlobusServer:
    def __init__(self):
        this_path = os.path.dirname(__file__)
        # Testing: send same static picture
        pic_bmp = os.path.join(this_path, "Earth_relief_120x256.bmp")
        print(f'Static Pic Path: {pic_bmp}')
        if not os.path.exists(pic_bmp):
            print("Not existing")

        #
        if USE_STATIC_PIC:
            img = cv2.imread(pic_bmp)
            # img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
            # Compress, 80% quality
            encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 90]
            _, buffer = cv2.imencode(".jpg", img, encode_param)
            self.__pic3 = buffer.tobytes()
            print(len(self.__pic3))

        self.__vids = video_processor.VideoPlayer(GIF_PATH)
        self.__vids.play()

        self.__image_to_show = None
        self.__is_image_showing = False

    def get_list_of_available_gifs(self):
        return self.__vids.get_available_video_names()

    def set_video(self, gif_name):
        """gif_name: str, name of the gif, no extension, no path"""
        if gif_name in self.get_list_of_available_gifs():
            self.__vids.change_video(gif_name, None)

    def get_frame(self, time_used_prev_frame):
        # called from esp
        if self.__is_image_showing:
            if self.__image_to_show is None:
                return self.__pic3
            else:
                return self.__image_to_show

        if USE_STATIC_PIC:
            return self.__pic3
        return self.__vids.get_frame(time_used_prev_frame)

    def set_image(self, image):
        pic = img_proc.scale_crop_image(image)
        encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 80]
        _, buffer = cv2.imencode(".jpg", pic, encode_param)
        self.__pic3 = buffer.tobytes()

        self.__is_image_showing = True

    def return_to_video(self):
        self.__is_image_showing = False
