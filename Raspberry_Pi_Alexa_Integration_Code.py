import os
import time
import json
from flask import Flask, request, jsonify
import serial

# ---- Config ----
SERIAL_PORT = os.getenv("HF_SERIAL_PORT", "/dev/ttyACM0")  # or /dev/ttyUSB0 depending on the setup
BAUDRATE = int(os.getenv("HF_BAUD", "9600"))

# ---- Serial ----
arduino = None
def ensure_serial():
    global arduino
    if arduino is None or not arduino.is_open:
        arduino = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
        time.sleep(2) # Arduino Rest Delay

# ---- Alexa helpers ----
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
            "Welcome to Home Farmer. Say water the plants or plant seeds."
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
                return jsonify(plain_text_response("Seeding has started."))
            except Exception as e:
                return jsonify(plain_text_response(f"Unable to start seeding: {e}"))

        # WaterIntent
        if name == "WaterIntent":
            try:
                ensure_serial()
                arduino.write(b"WATER\n")
                return jsonify(plain_text_response("Watering has started."))
            except Exception as e:
                return jsonify(plain_text_response(f"Unable to start watering: {e}"))

        # Unknown intent
        return jsonify(plain_text_response("Sorry, I didn't understand that intent."))

    # SessionEndedRequest or anything else
    return jsonify(plain_text_response("Goodbye."))

if __name__ == "__main__":
    # Allow overriding port via env, but not needed
    port = int(os.getenv("PORT", "5000"))
    app.run(host="0.0.0.0", port=port, debug=False)
