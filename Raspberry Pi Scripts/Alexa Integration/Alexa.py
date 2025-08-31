import os
import time
import json
from flask import Flask, request, jsonify
import serial

# ---- Config ----
SERIAL_PORT = os.getenv("HF_SERIAL_PORT", "/dev/ttyACM0")  # or /dev/ttyUSB0
BAUDRATE = int(os.getenv("HF_BAUD", "9600"))

# ---- Serial ----
arduino = None
def ensure_serial():
    global arduino
    if arduino is None or not arduino.is_open:
        arduino = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
        time.sleep(2)  # let Arduino reset

# ---- DHT11/22 GPIO 27 ----
try:
    import Adafruit_DHT
    DHT = Adafruit_DHT.DHT11
    DHT_PIN = 27
    HAVE_DHT = True
except Exception:
    HAVE_DHT = False

def get_dht_stats():
    if not HAVE_DHT:
        return None, None
    humidity, temperature = Adafruit_DHT.read_retry(DHT, DHT_PIN)
    if humidity is None or temperature is None:
        return None, None
    return temperature, humidity

# ---- Alexa helpers (no SDK) ----
def plain_text_response(text, end_session=True):
    return {
        "version": "1.0",
        "response": {
            "outputSpeech": {"type": "PlainText", "text": text},
            "shouldEndSession": end_session
        }
    }

def reprompt_response(text):
    return {
        "version": "1.0",
        "response": {
            "outputSpeech": {"type": "PlainText", "text": text},
            "reprompt": {"outputSpeech": {"type": "PlainText", "text": text}},
            "shouldEndSession": False
        }
    }

# ---- Flask app ----
app = Flask(__name__)

@app.route("/", methods=["POST"])
def alexa_entry():
    # Parse the incoming Alexa request
    body = request.get_json(force=True, silent=False)

    req = body.get("request", {})
    req_type = req.get("type", "")

    # LaunchRequest
    if req_type == "LaunchRequest":
        return jsonify(reprompt_response(
            "Welcome to Home Farmer. Give your command."
        ))

    # IntentRequest
    if req_type == "IntentRequest":
        intent = req.get("intent", {}) or {}
        name = intent.get("name", "")

        # SeedIntent
        if name == "SeedIntent":
            try:
                ensure_serial()
                arduino.write(b"SEED\n")
                return jsonify(plain_text_response("Home Farmer is now seeding."))
            except Exception as e:
                return jsonify(plain_text_response(f"Unable to start seeding: {e}"))

        # WaterIntent
        if name == "WaterIntent":
            try:
                ensure_serial()
                arduino.write(b"WATER\n")
                return jsonify(plain_text_response("Home Farmer is now watering the plants."))
            except Exception as e:
                return jsonify(plain_text_response(f"Unable to start watering: {e}"))

        # GetStatsIntent
        if name == "GetStatusIntent":
            temperature, humidity = get_dht_stats()
            if temperature is None or humidity is None:
                return jsonify(plain_text_response("Sorry, I could not read the temperature or humidity sensor right now."))
            return jsonify(plain_text_response(f"The temperature is {temperature:.1f} degrees Celsius and the humidity is {humidity:.1f} percent."))

        # RebootIntent
        if name == "RebootIntent":
            try:
                ensure_serial()
                arduino.write(b"RESET\n")
                return jsonify(plain_text_response("Reboot command sent to the Arduino."))
            except Exception as e:
                return jsonify(plain_text_response(f"Unable to send reboot command: {e}"))

        # Unknown intent
        return jsonify(plain_text_response("Sorry, Home Farmer didn't understand that command."))

    # SessionEndedRequest or anything else
    return jsonify(plain_text_response("Goodbye."))

if __name__ == "__main__":
    # Optional: allow overriding port/host via env
    port = int(os.getenv("PORT", "5000"))
    app.run(host="0.0.0.0", port=port, debug=False)
