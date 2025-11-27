from flask import Flask, Response, jsonify, render_template, request, redirect, flash
from werkzeug.utils import secure_filename
# from video_server.video_preprocessor import process_videos, get_video_data
# from PIL import Image
# import numpy as np
import time
import os
import shutil
import subprocess
from datetime import datetime
import re

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
    return jsonify({
        'speed': speed.read_speed(),
        'target_speed': speed.read_target_speed(),
        'enabled': speed.is_motor_enabled()
    })


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

    if 'file' in request.files:
        file = request.files['file']
        filename = secure_filename(file.filename)
        print(filename)

        if __allowed_file(filename):
            __clear_upload_folder()
            abs_file_path = os.path.join(app.config['UPLOAD_FOLDER'], filename)
            file.save(abs_file_path)
            print('Setting gif from Image')
            globus.set_gif_from_image(abs_file_path)
            # globus.set_image(abs_file_path)

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


@app.route('/device_time', methods=['POST'])
def receive_device_time():
    data = request.get_json()
    if data and 'time' in data:
        device_time_str = data['time']
        print(f'Device time received: {device_time_str}')

        # Robustly parse incoming time strings from different formats (ISO, JS Date, etc.)
        def _parse_device_time(ts: str) -> datetime:
            # 1) Try ISO 8601 (optionally ending with Z)
            try:
                return datetime.fromisoformat(ts.replace('Z', '+00:00'))
            except Exception:
                pass

            # 2) Try python-dateutil if available (handles many variants)
            try:
                from dateutil import parser as _du_parser  # type: ignore
                return _du_parser.parse(ts)
            except Exception:
                pass

            # 3) Try to normalize common JS Date.toString() format like:
            #    "Thu Nov 27 2025 12:07:05 GMT+0100 (Mitteleuropäische Normalzeit)"
            #    Remove the parenthetical part and optional leading 'GMT'
            try:
                # Remove parenthetical timezone name if present
                ts_norm = re.sub(r"\s*\([^\)]*\)\s*", " ", ts).strip()
                # Replace 'GMT+0100' -> '+0100' (keep offset)
                ts_norm = ts_norm.replace('GMT', '')
                # Collapse multiple spaces
                ts_norm = re.sub(r"\s+", " ", ts_norm)
                # Expected now: 'Thu Nov 27 2025 12:07:05 +0100'
                return datetime.strptime(ts_norm, '%a %b %d %Y %H:%M:%S %z')
            except Exception:
                pass

            # 4) As a last resort, try without weekday (some browsers omit it)
            try:
                # Example: 'Nov 27 2025 12:07:05 +0100'
                return datetime.strptime(ts_norm, '%b %d %Y %H:%M:%S %z')  # type: ignore[name-defined]
            except Exception:
                pass

            raise ValueError(f'Unsupported time format: {ts}')

        try:
            device_time = _parse_device_time(device_time_str)
        except Exception as e:
            msg = f'Error parsing device time: {e}'
            print(msg)
            return jsonify({'status': 'error', 'message': msg}), 400

        # Get current system time
        system_time = datetime.now(device_time.tzinfo)

        # Calculate time difference in seconds
        time_diff = abs((device_time - system_time).total_seconds())

        print(f'System time: {system_time}')
        print(f'Time difference: {time_diff:.2f} seconds')

        # Check if difference is greater than 1 hour (3600 seconds)
        if time_diff > 3600:
            print(f'Time discrepancy detected: {time_diff / 3600:.2f} hours')

            # Format time for the date command: "YYYY-MM-DD HH:MM:SS"
            time_str = device_time.strftime('%Y-%m-%d %H:%M:%S')

            if not speeddriver.LOCAL:
                try:
                    # Set system time using sudo date command
                    # Note: The Flask app needs to be run with appropriate permissions
                    # or you need to configure sudoers to allow date command without password

                    result = subprocess.run(
                        ['sudo', 'date', '-s', time_str],
                        capture_output=True,
                        text=True,
                        check=True
                    )
                    print(f'System time updated successfully to: {time_str}')
                    return jsonify(
                        {'status': 'success', 'message': 'Time synchronized', 'time_diff_seconds': time_diff}), 200

                except subprocess.CalledProcessError as e:
                    print(f'Error setting system time: {e.stderr}')
                    return jsonify({'status': 'error', 'message': f'Failed to set system time: {e.stderr}'}), 500
            else:
                print('Local mode, not setting system time')
        else:
            print('Time difference within acceptable range (< 1 hour)')
            return jsonify(
                {'status': 'success', 'message': 'Time difference acceptable', 'time_diff_seconds': time_diff}), 200

        # Note: Any parsing errors are handled earlier

    return jsonify({'status': 'error', 'message': 'No time data received'}), 400


if __name__ == '__main__':
    # to run on Local PC, set LOCAL = True in speeddriver
    app.run(host='127.0.0.1')
    # app.run(host = '192.168.137.1', port=8123)
