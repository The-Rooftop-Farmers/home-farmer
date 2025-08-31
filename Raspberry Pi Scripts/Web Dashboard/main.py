from flask import Flask, Response, render_template, jsonify, url_for
from datetime import datetime
import threading, time, json, atexit
import serial
import cv2
from picamera2 import Picamera2

# --- Serial setup (for Arduino comm) ---
SERIAL_PORT = '/dev/ttyACM0'
ser = serial.Serial(SERIAL_PORT, 9600, timeout=1)
soil_moisture_value = None  # Updated from Arduino

# --- DHT setup ---
try:
    import Adafruit_DHT
    DHT = Adafruit_DHT.DHT11
    DHT_PIN = 27
    HAVE_DHT = True
except Exception:
    HAVE_DHT = False

app = Flask(__name__)
port_num = 4000

# --- Camera setup ---
picam2 = Picamera2()
picam2.preview_configuration.main.size = (720, 1280)
picam2.preview_configuration.main.format = "RGB888"
picam2.configure("preview")
picam2.start()

def frame_generator():
    while True:
        frame = picam2.capture_array()
        frame = cv2.rotate(frame, cv2.ROTATE_90_CLOCKWISE)
        ok, buf = cv2.imencode(".jpg", frame, [cv2.IMWRITE_JPEG_QUALITY, 80])
        if not ok:
            continue
        yield (b"--frame\r\nContent-Type: image/jpeg\r\n\r\n" + buf.tobytes() + b"\r\n")

# --- Sensor read core ---
def read_sensors():
    temperature = None
    humidity = None
    if HAVE_DHT:
        h, t = Adafruit_DHT.read_retry(DHT, DHT_PIN)
        if h is not None and t is not None:
            humidity, temperature = float(h), float(t)
    soil = soil_moisture_value
    return {
        "timestamp": datetime.utcnow().isoformat() + "Z",
        "temperature": temperature,
        "humidity": humidity,
        "soil_moisture": soil
    }

# --- Minimal cache ---
SENSOR_HISTORY = []
HIST_LIMIT = 120

def sensor_sampler():
    while True:
        data = read_sensors()
        SENSOR_HISTORY.append(data)
        if len(SENSOR_HISTORY) > HIST_LIMIT:
            SENSOR_HISTORY[:len(SENSOR_HISTORY)-HIST_LIMIT] = []
        time.sleep(1)

threading.Thread(target=sensor_sampler, daemon=True).start()

# --- Serial bridge ---
def serial_bridge():
    global soil_moisture_value
    while True:
        if ser.in_waiting:
            try:
                line = ser.readline().decode('utf-8').strip()
                if line.startswith("MOISTURE:"):
                    val = int(line.split(":")[1])
                    soil_moisture_value = val
            except Exception:
                pass
        if HAVE_DHT:
            h, t = Adafruit_DHT.read_retry(DHT, DHT_PIN)
            if h is not None and t is not None:
                msg = "DHT:{},{}\n".format(int(t), int(h))
                try:
                    ser.write(msg.encode('utf-8'))
                except:
                    pass
        time.sleep(1)

threading.Thread(target=serial_bridge, daemon=True).start()

# --- Routes ---
@app.route("/")
def index():
    return render_template("index.html")

@app.route("/video")
def video():
    return Response(frame_generator(), mimetype="multipart/x-mixed-replace; boundary=frame")

@app.route("/sensors")
def sensors():
    return jsonify(read_sensors())

@app.route("/sensors/history")
def sensors_history():
    return app.response_class(
        response=json.dumps(SENSOR_HISTORY[-60:], separators=(",", ":")),
        status=200, mimetype="application/json"
    )

@atexit.register
def _cleanup():
    try:
        picam2.stop()
    except Exception:
        pass

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=port_num, debug=False, threaded=True)
