from flask import Flask, render_template_string, Response, jsonify
from picamera2 import Picamera2
import cv2
import Adafruit_DHT
import time
from datetime import datetime
from collections import deque
import threading

app = Flask(__name__)

# DHT11 setup
DHT_SENSOR = Adafruit_DHT.DHT11
DHT_PIN = 27

# Initialize camera
picam2 = Picamera2()
picam2.preview_configuration.main.size = (640, 480)  # Full size for left side
picam2.preview_configuration.main.format = "RGB888"
picam2.configure("preview")
picam2.start()

# Data storage for graphs (last 30 points = 1 minute at 2-second intervals)
max_data_points = 30
sensor_data = {
    'timestamps': deque(maxlen=max_data_points),
    'temperature': deque(maxlen=max_data_points),
    'humidity': deque(maxlen=max_data_points),
    'soil_moisture': deque(maxlen=max_data_points)
}

# HTML template with camera on left, graphs on right
HTML_PAGE = """
<!DOCTYPE html>
<html>
<head>
    <title>Pi Sensor Dashboard</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body { 
            font-family: Arial, sans-serif; 
            background: #f2f2f2; 
            overflow-x: auto;
            overflow-y: auto;
        }
        .dashboard { 
            display: grid; 
            grid-template-columns: 400px 1fr; 
            gap: 10px; 
            min-height: 100vh; 
            padding: 10px;
        }
        .camera-section { 
            background: white; 
            border: 2px solid #333; 
            border-radius: 8px; 
            padding: 15px; 
            display: flex; 
            flex-direction: column; 
            align-items: center;
            height: fit-content;
            position: sticky;
            top: 10px;
        }
        .graphs-section {
            display: flex;
            flex-direction: column;
            gap: 10px;
            min-height: 100vh;
        }
        .graph-panel { 
            background: white; 
            border: 2px solid #333; 
            border-radius: 8px; 
            padding: 15px; 
            height: 400px;
            display: flex; 
            flex-direction: column; 
        }
        .section-title { 
            margin: 0 0 15px 0; 
            color: #333; 
            font-size: 18px;
            text-align: center;
        }
        .graph-title { 
            margin: 0 0 10px 0; 
            color: #333; 
            font-size: 16px;
            text-align: center;
        }
        .video-feed { 
            width: 320px;
            height: 240px;
            object-fit: contain; 
            border-radius: 4px; 
            border: 1px solid #ddd;
        }
        .chart-container { 
            flex: 1;
            position: relative; 
            min-height: 250px;
        }
        .current-value {
            font-size: 20px;
            font-weight: bold;
            color: #007bff;
            margin-bottom: 8px;
            text-align: center;
        }
        .status {
            font-size: 11px;
            color: #666;
            text-align: center;
            margin-bottom: 8px;
        }
        .graph-header {
            text-align: center;
            margin-bottom: 10px;
        }
        .timestamp {
            font-size: 12px;
            color: #888;
            text-align: center;
            margin-top: 10px;
        }
        
        /* Responsive design */
        @media (max-width: 768px) {
            .dashboard {
                grid-template-columns: 1fr;
                gap: 10px;
            }
            .camera-section {
                position: static;
            }
            .video-feed {
                width: 280px;
                height: 210px;
            }
        }
    </style>
</head>
<body>
    <div class="dashboard">
        <!-- Camera Feed Section -->
        <div class="camera-section">
            <h2 class="section-title">Live Camera Feed</h2>
            <img src="{{ url_for('video_feed') }}" class="video-feed">
            <div class="timestamp" id="camera-timestamp">--</div>
        </div>
        
        <!-- Graphs Section -->
        <div class="graphs-section">
            <!-- Temperature Graph -->
            <div class="graph-panel">
                <div class="graph-header">
                    <h3 class="graph-title">Temperature</h3>
                    <div class="current-value" id="temp-current">-- °C</div>
                </div>
                <div class="chart-container">
                    <canvas id="tempChart"></canvas>
                </div>
            </div>
            
            <!-- Humidity Graph -->
            <div class="graph-panel">
                <div class="graph-header">
                    <h3 class="graph-title">Humidity</h3>
                    <div class="current-value" id="humidity-current">-- %</div>
                </div>
                <div class="chart-container">
                    <canvas id="humidityChart"></canvas>
                </div>
            </div>
            
            <!-- Soil Moisture Graph -->
            <div class="graph-panel">
                <div class="graph-header">
                    <h3 class="graph-title">Soil Moisture</h3>
                    <div class="current-value" id="moisture-current">-- %</div>
                    <div class="status">Sensor not connected</div>
                </div>
                <div class="chart-container">
                    <canvas id="moistureChart"></canvas>
                </div>
            </div>
            
            <!-- Additional Info Panel -->
            <div class="graph-panel">
                <div class="graph-header">
                    <h3 class="graph-title">System Info</h3>
                </div>
                <div style="padding: 20px; text-align: center; color: #666;">
                    <p><strong>Data Points:</strong> Last 30 readings (1 minute)</p>
                    <p><strong>Update Interval:</strong> Every 2 seconds</p>
                    <p><strong>User:</strong> vihaanvp</p>
                    <p><strong>Last Update:</strong> <span id="last-update">--</span></p>
                    <div style="margin-top: 20px; padding: 10px; background: #f8f9fa; border-radius: 4px;">
                        <p style="font-size: 12px; margin: 0;">
                            Ready to add soil moisture sensor.<br>
                            Video content rotated 90° clockwise.
                        </p>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <script>
        // Chart configurations
        const chartConfig = {
            type: 'line',
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    x: {
                        display: true,
                        title: {
                            display: false
                        },
                        ticks: {
                            maxTicksLimit: 6,
                            font: {
                                size: 10
                            }
                        }
                    },
                    y: {
                        display: true,
                        beginAtZero: true,
                        ticks: {
                            font: {
                                size: 10
                            }
                        }
                    }
                },
                plugins: {
                    legend: {
                        display: false
                    }
                },
                animation: {
                    duration: 0
                },
                elements: {
                    point: {
                        radius: 2
                    }
                }
            }
        };

        // Initialize charts
        const tempChart = new Chart(document.getElementById('tempChart'), {
            ...chartConfig,
            data: {
                labels: [],
                datasets: [{
                    label: 'Temperature (°C)',
                    data: [],
                    borderColor: 'rgb(255, 99, 132)',
                    backgroundColor: 'rgba(255, 99, 132, 0.2)',
                    tension: 0.3,
                    fill: true,
                    borderWidth: 2
                }]
            }
        });

        const humidityChart = new Chart(document.getElementById('humidityChart'), {
            ...chartConfig,
            data: {
                labels: [],
                datasets: [{
                    label: 'Humidity (%)',
                    data: [],
                    borderColor: 'rgb(54, 162, 235)',
                    backgroundColor: 'rgba(54, 162, 235, 0.2)',
                    tension: 0.3,
                    fill: true,
                    borderWidth: 2
                }]
            },
            options: {
                ...chartConfig.options,
                scales: {
                    ...chartConfig.options.scales,
                    y: {
                        ...chartConfig.options.scales.y,
                        max: 100
                    }
                }
            }
        });

        const moistureChart = new Chart(document.getElementById('moistureChart'), {
            ...chartConfig,
            data: {
                labels: [],
                datasets: [{
                    label: 'Soil Moisture (%)',
                    data: [],
                    borderColor: 'rgb(75, 192, 192)',
                    backgroundColor: 'rgba(75, 192, 192, 0.2)',
                    tension: 0.3,
                    fill: true,
                    borderWidth: 2
                }]
            },
            options: {
                ...chartConfig.options,
                scales: {
                    ...chartConfig.options.scales,
                    y: {
                        ...chartConfig.options.scales.y,
                        max: 100
                    }
                }
            }
        });

        // Function to update charts
        function updateCharts() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    // Update current values
                    document.getElementById('temp-current').textContent = 
                        data.current.temperature !== null ? data.current.temperature.toFixed(1) + ' °C' : 'N/A';
                    document.getElementById('humidity-current').textContent = 
                        data.current.humidity !== null ? data.current.humidity.toFixed(1) + ' %' : 'N/A';
                    document.getElementById('moisture-current').textContent = 
                        data.current.soil_moisture !== null ? data.current.soil_moisture.toFixed(1) + ' %' : 'N/A';

                    // Update timestamps
                    const now = new Date();
                    document.getElementById('camera-timestamp').textContent = now.toLocaleTimeString();
                    document.getElementById('last-update').textContent = now.toLocaleTimeString();

                    // Prepare labels for better readability (show every 5th timestamp)
                    const displayLabels = data.timestamps.map((time, index) => 
                        index % 5 === 0 ? time : ''
                    );

                    // Update charts
                    tempChart.data.labels = displayLabels;
                    tempChart.data.datasets[0].data = data.temperature;
                    tempChart.update();

                    humidityChart.data.labels = displayLabels;
                    humidityChart.data.datasets[0].data = data.humidity;
                    humidityChart.update();

                    moistureChart.data.labels = displayLabels;
                    moistureChart.data.datasets[0].data = data.soil_moisture;
                    moistureChart.update();
                })
                .catch(error => {
                    console.error('Error fetching data:', error);
                });
        }

        // Update every 2 seconds
        setInterval(updateCharts, 2000);
        updateCharts(); // Initial load
    </script>
</body>
</html>
"""

def read_sensors():
    """Read sensor values and add to data storage"""
    while True:
        try:
            # Read DHT11 values
            humidity, temperature = Adafruit_DHT.read_retry(DHT_SENSOR, DHT_PIN)
            
            # Soil moisture sensor placeholder - ready for future implementation
            soil_moisture = None  # Will be replaced when sensor is connected
            
            # Add timestamp
            timestamp = datetime.now().strftime("%H:%M:%S")
            
            # Store data
            sensor_data['timestamps'].append(timestamp)
            sensor_data['temperature'].append(temperature)
            sensor_data['humidity'].append(humidity)
            sensor_data['soil_moisture'].append(soil_moisture)
            
            print(f"Data logged: {timestamp} - Temp: {temperature}°C, Humidity: {humidity}%, Moisture: {soil_moisture}")
            
        except Exception as e:
            print(f"Error reading sensors: {e}")
            # Add None values to maintain time consistency
            timestamp = datetime.now().strftime("%H:%M:%S")
            sensor_data['timestamps'].append(timestamp)
            sensor_data['temperature'].append(None)
            sensor_data['humidity'].append(None)
            sensor_data['soil_moisture'].append(None)
        
        time.sleep(2)  # 2-second interval

def generate_frames():
    while True:
        try:
            frame = picam2.capture_array()
            # Rotate frame 90 degrees clockwise - this rotates the VIDEO CONTENT itself
            rotated_frame = cv2.rotate(frame, cv2.ROTATE_90_CLOCKWISE)
            ret, buffer = cv2.imencode('.jpg', rotated_frame)
            frame_bytes = buffer.tobytes()
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame_bytes + b'\r\n')
        except Exception as e:
            print(f"Camera error: {e}")
            # Provide a simple error frame
            yield (b'--frame\r\n'
                   b'Content-Type: text/plain\r\n\r\nCamera Error\r\n')

@app.route('/')
def index():
    return render_template_string(HTML_PAGE)

@app.route('/data')
def get_data():
    """API endpoint to get sensor data for charts"""
    current_temp = sensor_data['temperature'][-1] if sensor_data['temperature'] else None
    current_humidity = sensor_data['humidity'][-1] if sensor_data['humidity'] else None
    current_moisture = sensor_data['soil_moisture'][-1] if sensor_data['soil_moisture'] else None
    
    return jsonify({
        'timestamps': list(sensor_data['timestamps']),
        'temperature': list(sensor_data['temperature']),
        'humidity': list(sensor_data['humidity']),
        'soil_moisture': list(sensor_data['soil_moisture']),
        'current': {
            'temperature': current_temp,
            'humidity': current_humidity,
            'soil_moisture': current_moisture
        }
    })

@app.route('/video_feed')
def video_feed():
    return Response(generate_frames(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == "__main__":
    print("Starting Raspberry Pi Sensor Dashboard...")
    print(f"User: vihaanvp")
    print(f"Dashboard will be available at: http://localhost:5000")
    print("Video content will be rotated 90 degrees clockwise")
    print("Page is now scrollable with reduced video size")
    
    # Start sensor reading thread
    sensor_thread = threading.Thread(target=read_sensors, daemon=True)
    sensor_thread.start()
    
    app.run(host="0.0.0.0", port=4000, debug=False)
