# Home Farmer Robot

> **Note:** This file has been modified using AI.

The **Home Farmer Robot** is a semi-automatic plant care robot designed to monitor and maintain optimal growing conditions for small-scale indoor farming. Built for the **World Robot Olympiad (WRO)** competition, this project uses an Arduino-controlled gantry system with sensors and a Raspberry Pi web dashboard to automate irrigation, seeding, and environmental monitoring.

> 🌐 **Live Website:** [the-rooftop-farmers.github.io/home-farmer](https://the-rooftop-farmers.github.io/home-farmer)
>
> 📦 **Required Library:** [Operate Library](https://github.com/The-Rooftop-Farmers/Operate-Lib/releases) — install via Arduino IDE (Sketch → Include Library → Add .ZIP Library)

---

## Features

- **3-Axis Gantry System** — X/Y/Z stepper motors with limit-switch homing and position limits (via [Operate Library](https://github.com/The-Rooftop-Farmers/Operate-Lib))
- **Automated Seeding** — Picks seeds from a designated area and plants at 4 predefined positions (or custom coordinates via rotary encoder)
- **Smart Watering** — Measures soil moisture at each plant; waters only when below threshold (40%), re-checks until target (70%) reached
- **Environmental Monitoring** — DHT11 temperature/humidity + soil moisture sensor (analog)
- **Real-Time Clock** — DS1307 RTC for scheduled auto-watering (3 min interval, configurable)
- **LCD Menu Interface** — 20×4 I²C LCD with rotary encoder + button navigation
- **Raspberry Pi Web Dashboard** — Live camera feed (Pi Camera 2), sensor charts (Chart.js), REST API (`/sensors`, `/sensors/history`, `/video`)
- **Alexa/Serial Integration** — Accepts `SEED`, `WATER`, `RESET`, `DHT:` commands over serial
- **Auto-Watering Mode** — Background timer triggers watering cycle independently of menu

---

## Hardware

| Component | Qty | Notes |
|-----------|-----|-------|
| Arduino Mega 2560 (or Uno with port expanders) | 1 | Main controller |
| NEMA 17 Stepper Motors | 3 | X, Y, Z axes |
| A4988 / DRV8825 Stepper Drivers | 3 | One per motor |
| Limit Switches (NO) | 3 | Homing for X, Y, Z |
| Soil Moisture Sensor (capacitive/resistive) | 1 | Analog, connected to A3 |
| DHT11 Temperature/Humidity Sensor | 1 | Digital, pin 27 on Pi (also read via serial from Arduino) |
| DS1307 RTC Module | 1 | I²C (0x68) |
| 20×4 I²C LCD (0x27) | 1 | 4-row display |
| Rotary Encoder (KY-040) | 1 | CLK=A2, DT=A1, SW=A0 |
| Water Pump (5V/12V) + Relay/Transistor | 1 | Pin 12 (Arduino) |
| Servo (SG90/MG996R) | 1 | Seed picker, pin 13 |
| Raspberry Pi 4 + Pi Camera Module 2 | 1 | Web dashboard + camera |
| 3D Printed Frame & Gantry | 1 | See `3D Printing/` |

---

## Pinout (Arduino)

| Function | Pin | Notes |
|----------|-----|-------|
| Stepper X – Step | 2 | `mot_x.setPin(2, 5)` |
| Stepper X – Dir | 5 | |
| Stepper Y – Step | 3 | `mot_y.setPin(3, 6)` |
| Stepper Y – Dir | 6 | |
| Stepper Z – Step | 4 | `mot_z.setPin(4, 7)` |
| Stepper Z – Dir | 7 | |
| Limit Switch X | 9 | `INPUT_PULLUP` |
| Limit Switch Y | 10 | `INPUT_PULLUP` |
| Limit Switch Z | 11 | `INPUT_PULLUP` |
| Water Pump Relay | 12 | `OUTPUT`, active HIGH |
| Servo (Seed Picker) | 13 | `Servo.attach(13)` |
| Soil Moisture (Analog) | A3 | 0–1023 → mapped to 0–100% |
| Rotary Encoder CLK | A2 | |
| Rotary Encoder DT | A1 | |
| Rotary Encoder SW | A0 | `INPUT_PULLUP` (button) |
| I²C SDA/SCL | 20/21 | LCD (0x27), RTC (0x68) |

---

## Software Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      ARDUINO (main.ino)                      │
│  ┌──────────────┐ ┌──────────────┐ ┌────────────────────┐  │
│  │ Gantry Control│ │ Sensor Read  │ │ Menu State Machine │  │
│  │ (Operate lib) │ │ (DHT, Moist, │ │ (20 screens, rotary │  │
│  │  Homing, Limits)│ │  RTC)      │ │  encoder + button) │  │
│  └──────┬───────┘ └──────┬───────┘ └─────────┬──────────┘  │
│         │                │                   │             │
│         └────────────────┼───────────────────┘             │
│                          ▼                                  │
│                   Serial (9600) ◄──────────────────────┐    │
└──────────────────────────────────────────────────────────│────┘
                                                           │ USB
                                                           ▼
┌──────────────────────────────────────────────────────────────────┐
│                    RASPBERRY PI (Flask + PiCamera2)               │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────────┐  │
│  │ Serial Bridge│  │ Sensor Sampler│ │   Web Routes (Flask)    │  │
│  │ (MOISTURE:←  │  │ (1 Hz, DHT +  │  │  /          → index.html│  │
│  │  DHT:→)      │  │  Arduino)    │  │  /video     → MJPEG     │  │
│  └─────────────┘  └─────────────┘  │  /sensors   → JSON now    │  │
│                                    │  /sensors/history → JSON  │  │
│                                    └─────────────────────────┘  │
└──────────────────────────────────────────────────────────────────┘
```

**Key Data Flow:**
1. Arduino reads soil moisture (A3) → sends `MOISTURE:<val>\n` every 1 s
2. Pi reads serial, parses moisture, updates global `soil_moisture_value`
3. Pi reads DHT11 locally → sends `DHT:<temp>,<hum>\n` back to Arduino (Arduino displays on LCD)
4. Flask serves `/sensors` (current), `/sensors/history` (last 60 samples), `/video` (MJPEG stream)

---

## Getting Started

### 1. Arduino Firmware

**Dependencies (Arduino IDE → Library Manager):**
- `Operate` — [download ZIP](https://github.com/The-Rooftop-Farmers/Operate-Lib/releases) → Sketch → Include Library → Add .ZIP Library
- `LiquidCrystal_I2C`
- `uRTCLib`
- `DHT sensor library` (Adafruit)
- `Servo` (built-in)

**Upload:**
1. Open `main/main.ino`
2. Select board (Arduino Mega 2560) and port
3. Upload

**Calibration (adjust in code):**
- `xStepsPerCm`, `yStepsPerCm` in `UnitConversion()` — measure your gantry travel per step
- `xOffset`, `halfSquareWidth` — physical offsets
- Soil moisture `map()` ranges (lines 350, 448) — measure dry/wet raw values
- Motor speeds (`setSpeed`), homing speeds, position limits — match your hardware

### 2. Raspberry Pi Dashboard

**Requirements:**
- Raspberry Pi OS (64-bit recommended)
- Python 3.9+
- Pi Camera Module 2 (or USB webcam — modify `main.py`)

**Install:**
```bash
cd "Raspberry Pi Scripts/Web Dashboard"
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
# requirements.txt: flask, pyserial, opencv-python, picamera2, Adafruit-DHT
```

**Run:**
```bash
python main.py
# Serves on http://<pi-ip>:4000
```

**Systemd Service (auto-start):**
```ini
# /etc/systemd/system/home-farmer-dashboard.service
[Unit]
Description=Home Farmer Web Dashboard
After=network.target

[Service]
WorkingDirectory=/home/pi/HomeFarmer/Raspberry Pi Scripts/Web Dashboard
ExecStart=/home/pi/HomeFarmer/Raspberry Pi Scripts/Web Dashboard/.venv/bin/python main.py
Restart=always
User=pi

[Install]
WantedBy=multi-user.target
```
```bash
sudo systemctl enable --now home-farmer-dashboard
```

### 3. 3D Printing

STL files in `3D Printing/2024 (v2.4)/Home Farmer All 3D Printed Parts.stl`. Print with:
- Material: PLA or PETG
- Infill: 30–40% (structural parts), 15% (non-structural)
- Layer height: 0.2 mm
- Supports: As needed for overhangs

---

## Menu System (LCD)

| Screen | Function |
|--------|----------|
| 0/1 | Home — time, date, temp, humidity, menu entry |
| 3 | Info — version, authors, mentor |
| 4 | Main Menu — Plant Seed / Water Plants / Parameters / Exit |
| 5–6 | Plant Seed confirmation |
| 7–11 | Water Plants / Parameters / Exit navigation |
| 12–14 | Seed planting confirmation (Yes/No) |
| 15 | Seeding in progress (homes → seeds 4 positions → homes) |
| 16–18 | Watering confirmation (Yes/No) |
| 19 | Watering in progress (homes → checks 4 positions → homes) |
| 20 | Parameters — live temp/humidity/moisture display |

**Navigation:** Rotary encoder scrolls, button selects. `>` indicator shows cursor.

---

## Serial Commands (for Alexa / Home Assistant / Custom Integrations)

Send via USB serial (9600 baud, newline-terminated):

| Command | Effect |
|---------|--------|
| `SEED` | Homes all axes, runs seeding sequence (screen 15) |
| `WATER` | Homes all axes, runs watering sequence (screen 19) |
| `RESET` | Software reset (watchdog) |
| `DHT:<temp>,<hum>` | Pi → Arduino: updates displayed temp/humidity |

Example (Python):
```python
import serial
ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
ser.write(b'SEED\n')
```

---

## Project Structure

```
Home Farmer/
├── main/
│   └── main.ino              # Arduino firmware (1200+ lines)
├── Raspberry Pi Scripts/
│   └── Web Dashboard/
│       ├── main.py           # Flask + PiCamera2 + serial bridge
│       ├── requirements.txt
│       ├── templates/
│       │   └── index.html    # Dashboard UI (Chart.js)
│       └── static/
│           ├── style.css
│           └── script.js     # Live sensor charts
├── 3D Printing/
│   └── 2024 (v2.4)/
│       └── Home Farmer All 3D Printed Parts.stl
├── Old Code/                 # Early prototypes (reference only)
├── Project Reports/          # WRO competition reports (PDF)
└── README.md                 # This file
```

---

## Troubleshooting

| Symptom | Likely Cause | Fix |
|---------|--------------|-----|
| Motors don't home | Limit switch wiring / `INPUT_PULLUP` logic | Check switches: NO → pin reads LOW when triggered |
| Soil moisture always 0/100 | `map()` range wrong | Measure raw `analogRead(A3)` in air vs water; update `map(min, max, 0, 100)` |
| LCD shows garbage | I²C address wrong | Run I²C scanner; common addresses: 0x27, 0x3F |
| Pi dashboard shows no camera | `picamera2` not configured | `sudo raspi-config` → Interface Options → Camera → Enable |
| Serial `MOISTURE:` not received | Baud mismatch / port wrong | Confirm 9600 baud; `ls /dev/ttyACM*` or `ttyUSB*` |
| Auto-watering doesn't trigger | `autowater` flag false | Set `bool autowater = true;` in code or via menu (not yet exposed) |

---

## Competition History

- **WRO 2024 (v2.4)** — Virtual & Regional rounds — Project reports in `Project Reports/2024/` and `2025/`
- **Team:** The Rooftop Farmers
- **Students:** Vihaan Parlikar, Yogeshwar Deshmukh
- **Mentor:** Malhar Ashtaputre

---

## License

MIT License — see [LICENSE](LICENSE).

---

## Acknowledgements

- [Operate Library](https://github.com/The-Rooftop-Farmers/Operate-Lib) — Stepper motor control with homing/limits
- Original Foliate project (inspiration for modular design)
- World Robot Olympiad community