# dashboard_plus.py
# System deps (recommended on Pi): sudo apt install -y python3-flask python3-opencv python3-picamera2
# Optional (if using DHT11):      sudo pip3 install Adafruit_DHT

from flask import Flask, Response, render_template_string, jsonify
from datetime import datetime
import threading, time, json, atexit

import cv2
from picamera2 import Picamera2

# Try to enable real DHT11 reads; otherwise we’ll return None for T/H
try:
    import Adafruit_DHT
    DHT = Adafruit_DHT.DHT11
    DHT_PIN = 27
    HAVE_DHT = True
except Exception:
    HAVE_DHT = False

app = Flask(__name__)
port_num = 4000

# --- Camera setup (pick tall frame so after 90° CW it’s landscape 1280x720) ---
picam2 = Picamera2()
picam2.preview_configuration.main.size = (720, 1280)     # (width,height) BEFORE rotation
picam2.preview_configuration.main.format = "RGB888"
picam2.configure("preview")
picam2.start()

def frame_generator():
    while True:
        frame = picam2.capture_array()
        # Rotate 90° clockwise for horizontal video
        frame = cv2.rotate(frame, cv2.ROTATE_90_CLOCKWISE)
        # (Optional) draw a tiny timestamp overlay:
        # cv2.putText(frame, datetime.now().strftime("%H:%M:%S"), (10,30),
        #            cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255,255,255), 2, cv2.LINE_AA)
        ok, buf = cv2.imencode(".jpg", frame, [cv2.IMWRITE_JPEG_QUALITY, 80])
        if not ok:
            continue
        yield (b"--frame\r\nContent-Type: image/jpeg\r\n\r\n" + buf.tobytes() + b"\r\n")

# --- Sensor read (real DHT11 if available, else placeholders) ---
def read_sensors():
    temperature = None
    humidity = None
    if HAVE_DHT:
        h, t = Adafruit_DHT.read_retry(DHT, DHT_PIN)
        if h is not None and t is not None:
            humidity, temperature = float(h), float(t)
    # Soil moisture placeholder (replace with real ADC/GPIO read later)
    soil = None
    return {
        "timestamp": datetime.utcnow().isoformat() + "Z",
        "temperature": temperature,  # °C
        "humidity": humidity,        # %
        "soil_moisture": soil        # %
    }

# --- Minimal cache to smooth charts if DHT is flaky ---
SENSOR_HISTORY = []
HIST_LIMIT = 120  # two minutes @ 1 Hz below (we’ll poll every 2s from the browser)

def sensor_sampler():
    while True:
        data = read_sensors()
        SENSOR_HISTORY.append(data)
        if len(SENSOR_HISTORY) > HIST_LIMIT:
            SENSOR_HISTORY[:len(SENSOR_HISTORY)-HIST_LIMIT] = []
        time.sleep(1)

threading.Thread(target=sensor_sampler, daemon=True).start()

# --- UI (modern card layout + Chart.js) ---
HTML = """
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Pi Cam Dashboard</title>
<link rel="preconnect" href="https://cdn.jsdelivr.net">
<style>
  :root {
    --bg:#0f172a; --fg:#e5e7eb; --card:#111827; --accent:#60a5fa; --muted:#94a3b8;
  }
  *{box-sizing:border-box}
  body{margin:0;font-family:system-ui,-apple-system,Segoe UI,Roboto,Ubuntu; background:var(--bg); color:var(--fg);}
  .wrap{max-width:1200px;margin:0 auto;padding:18px;display:grid;grid-template-columns:1.2fr 1fr;gap:18px}
  .card{background:var(--card);border-radius:18px;padding:16px;box-shadow:0 10px 30px rgba(0,0,0,0.25)}
  h1{font-size:22px;margin:0 0 10px 0;color:var(--fg);font-weight:650}
  .video{width:100%;height:auto;border-radius:12px;display:block}
  .grid2{display:grid;grid-template-columns:1fr 1fr;gap:14px}
  .stat{background:#0b1220;border-radius:14px;padding:14px;text-align:center}
  .stat .label{color:var(--muted);font-size:12px;margin-bottom:6px}
  .stat .value{font-size:22px;font-weight:700}
  canvas{width:100% !important;height:260px !important}
  @media (max-width: 900px){ .wrap{grid-template-columns:1fr;}}
</style>
</head>
<body>
  <div class="wrap">
    <div class="card">
      <h1>Live Camera (Rotated 90° CW, Landscape)</h1>
      <img class="video" src="{{ url_for('video') }}" alt="camera">
    </div>
    <div class="card">
      <h1>Now</h1>
      <div class="grid2">
        <div class="stat">
          <div class="label">Temperature</div>
          <div class="value" id="tNow">—</div>
        </div>
        <div class="stat">
          <div class="label">Humidity</div>
          <div class="value" id="hNow">—</div>
        </div>
        <div class="stat">
          <div class="label">Soil Moisture</div>
          <div class="value" id="sNow">—</div>
        </div>
        <div class="stat">
          <div class="label">Last Update</div>
          <div class="value" id="timeNow">—</div>
        </div>
      </div>
    </div>
    <div class="card">
      <h1>Temperature (°C)</h1>
      <canvas id="tChart"></canvas>
    </div>
    <div class="card">
      <h1>Humidity (%)</h1>
      <canvas id="hChart"></canvas>
    </div>
  </div>

<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<script>
  const fmt = ts => new Date(ts).toLocaleTimeString();
  const tCtx = document.getElementById('tChart').getContext('2d');
  const hCtx = document.getElementById('hChart').getContext('2d');

  const tData = {labels: [], datasets: [{label: '°C', data: []}]};
  const hData = {labels: [], datasets: [{label: '%', data: []}]};

  const baseOpts = {
    responsive:true,
    plugins:{legend:{display:false}},
    scales:{x:{ticks:{color:'#94a3b8'}}, y:{ticks:{color:'#94a3b8'}}}
  };
  const tChart = new Chart(tCtx, {type:'line', data:tData, options:baseOpts});
  const hChart = new Chart(hCtx, {type:'line', data:hData, options:baseOpts});

  function pushPoint(chart, labels, dataArr, label, val) {
    labels.push(label);
    dataArr.push(val);
    if (labels.length > 60) { labels.shift(); dataArr.shift(); }
    chart.update('none');
  }

  async function pull() {
    const r = await fetch('/sensors');
    const s = await r.json();

    document.getElementById('tNow').textContent =
      s.temperature !== null ? (s.temperature.toFixed ? s.temperature.toFixed(1) : s.temperature) + ' °C' : 'N/A';
    document.getElementById('hNow').textContent =
      s.humidity !== null ? (s.humidity.toFixed ? s.humidity.toFixed(1) : s.humidity) + ' %' : 'N/A';
    document.getElementById('sNow').textContent =
      s.soil_moisture !== null ? s.soil_moisture + ' %' : 'N/A';
    document.getElementById('timeNow').textContent = fmt(s.timestamp);

    const label = new Date(s.timestamp).toLocaleTimeString();
    if (s.temperature !== null && !isNaN(s.temperature))
      pushPoint(tChart, tData.labels, tData.datasets[0].data, label, s.temperature);
    if (s.humidity !== null && !isNaN(s.humidity))
      pushPoint(hChart, hData.labels, hData.datasets[0].data, label, s.humidity);
  }

  // warm start with history
  (async () => {
    const r = await fetch('/sensors/history');
    const hist = await r.json();
    hist.forEach(s => {
      const label = new Date(s.timestamp).toLocaleTimeString();
      if (s.temperature !== null) pushPoint(tChart, tData.labels, tData.datasets[0].data, label, s.temperature);
      if (s.humidity !== null)    pushPoint(hChart, hData.labels, hData.datasets[0].data, label, s.humidity);
    });
    await pull();
    setInterval(pull, 2000);
  })();
</script>
</body>
</html>
"""

@app.route("/")
def index():
    return render_template_string(HTML)

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
    # Use 0.0.0.0 to reach it from your LAN: http://<pi-ip>:5000
    app.run(host="0.0.0.0", port=port_num, debug=False, threaded=True)
