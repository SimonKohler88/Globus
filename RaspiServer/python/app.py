from flask import Flask, Response, jsonify, render_template, request, redirect, flash
from werkzeug.utils import secure_filename
# from video_server.video_preprocessor import process_videos, get_video_data
# from PIL import Image
# import numpy as np
import time
import os
import shutil

print(f'Current WD: {os.getcwd()}')

from GlobusServer import GlobusServer
import speeddriver

UPLOAD_FOLDER = os.path.join(os.path.dirname(__file__), 'temp_upload')
app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# globus = None
with app.app_context():
    print('init scripts')
    global globus, speed
    globus = GlobusServer()
    speed = speeddriver.SpeedDriver()
    app.logger.info("before_first_request")


@app.route('/')
def homepage():
    return render_template("HTML_fuer_Webserver.html")


#
@app.route('/motor_control/<cmd>', methods=['GET'])
def motor_control(cmd):
    if cmd in ['on', 'off']:
        # pass
        if cmd == "on":
            speed.enable()
        if cmd == "off":
            speed.disable()
    else:
        try:
            val = int(cmd)
            print(f'received speed value: {val}')
            speed.set_speed(val)
        except ValueError:
            # do nothing
            pass
    return f"mot_ctrl {cmd}"


@app.route('/motor_speed', methods=['GET'])
def get_speed():
    return str(speed.read_speed())


@app.route('/motor_target_speed', methods=['GET'])
def get_target_speed():
    return str(speed.read_target_speed())


@app.route('/frame/<int:delta_t_ms>')
def get_frame(delta_t_ms):
    pic = globus.get_frame(delta_t_ms)
    return Response(pic, mimetype='image/jpeg')


def __allowed_file(filename):
    nm, ext = os.path.splitext(filename)
    if ext.lower() in ['.jpg', '.jpeg', '.png']:
        return True
    return False


def __clear_upload_folder():
    if os.path.exists(UPLOAD_FOLDER):
        shutil.rmtree(UPLOAD_FOLDER)
    os.mkdir(UPLOAD_FOLDER)


@app.route('/upload', methods=['POST'])
def upload_file():
    print(request.form)
    if 'file' in request.files:
        file = request.files['file']
        filename = secure_filename(file.filename)
        print(filename)

        if __allowed_file(filename):
            __clear_upload_folder()
            abs_file_path = os.path.join(app.config['UPLOAD_FOLDER'], filename)
            file.save(abs_file_path)
            globus.set_image(abs_file_path)

    return "done"


@app.route('/show_globe_video', methods=['POST'])
def show_globe_video():
    if 'option' in request.form:
        option = request.form['option']
        print(option)
        available_gifs = globus.get_list_of_available_gifs()
        if option in available_gifs:
            globus.set_video(option)
            globus.return_to_video()
    # return render_template("HTML_fuer_Webserver.html")
    return "done"


if __name__ == '__main__':
    # to run on Local PC, set LOCAL = True in speeddriver

    # app.run(host='127.0.0.1')
    app.run(host='192.168.0.92', port=5000)
    # app.run(host = '192.168.137.1', port=8123)
