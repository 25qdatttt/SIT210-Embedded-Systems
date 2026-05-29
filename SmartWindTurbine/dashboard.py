import json
import time
import queue
import threading
import tkinter as tk
from tkinter import ttk
import paho.mqtt.client as mqtt
import RPi.GPIO as GPIO

# -------------------- MQTT CONFIG --------------------

MQTT_BROKER = "broker.hivemq.com"
MQTT_PORT = 1883

STATUS_TOPIC = "dat/smart_turbine/status"
COMMAND_TOPIC = "dat/smart_turbine/command"

# -------------------- GPIO CONFIG --------------------

LED_PIN = 17
BUZZER_PIN = 27

GPIO.setmode(GPIO.BCM)
GPIO.setup(LED_PIN, GPIO.OUT)
GPIO.setup(BUZZER_PIN, GPIO.OUT)


def alarm_on():
    GPIO.output(LED_PIN, GPIO.HIGH)
    GPIO.output(BUZZER_PIN, GPIO.HIGH)


def alarm_off():
    GPIO.output(LED_PIN, GPIO.LOW)
    GPIO.output(BUZZER_PIN, GPIO.LOW)


# -------------------- GLOBAL STATE --------------------

message_queue = queue.Queue()

last_status_time = 0
mqtt_connected = False
latest_arduino_mqtt = "-"

DATA_TIMEOUT = 15


# -------------------- MQTT CALLBACKS --------------------

def on_connect(client, userdata, flags, rc):
    global mqtt_connected

    if rc == 0:
        mqtt_connected = True
        client.subscribe(STATUS_TOPIC)
        message_queue.put(("log", "Dashboard connected to MQTT broker"))
        message_queue.put(("broker_status", "CONNECTED"))
    else:
        mqtt_connected = False
        message_queue.put(("log", "MQTT connection failed: " + str(rc)))
        message_queue.put(("broker_status", "DISCONNECTED"))


def on_disconnect(client, userdata, rc):
    global mqtt_connected

    mqtt_connected = False
    message_queue.put(("log", "Dashboard disconnected from MQTT broker. Reconnecting..."))
    message_queue.put(("broker_status", "DISCONNECTED"))


def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode()
        data = json.loads(payload)
        message_queue.put(("status", data))

    except Exception as e:
        message_queue.put(("log", "Invalid message: " + str(e)))


# -------------------- MQTT SETUP --------------------

client = mqtt.Client(client_id="Pi_Turbine_Dashboard_" + str(int(time.time())))
client.on_connect = on_connect
client.on_disconnect = on_disconnect
client.on_message = on_message

# Bật auto reconnect của paho
client.reconnect_delay_set(min_delay=1, max_delay=10)


def start_mqtt():
    while True:
        try:
            message_queue.put(("log", "Connecting to MQTT broker..."))
            client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
            client.loop_forever()

        except Exception as e:
            message_queue.put(("log", "MQTT error: " + str(e) + " — retrying in 5s"))
            message_queue.put(("broker_status", "DISCONNECTED"))
            time.sleep(5)


# -------------------- COMMANDS --------------------

def send_command(command):
    if not mqtt_connected:
        log_message("Cannot send command: dashboard is not connected to MQTT broker")
        return

    try:
        client.publish(COMMAND_TOPIC, command)
        last_command_var.set(command)
        log_message("Sent command: " + command)

    except Exception as e:
        log_message("Command failed: " + str(e))


# -------------------- DASHBOARD UPDATE --------------------

def update_alert_label(label, active, alert_text):
    if active:
        label.config(text=alert_text, foreground="red")
    else:
        label.config(text="SAFE", foreground="green")


def update_dashboard(data):
    global last_status_time, latest_arduino_mqtt

    last_status_time = time.time()

    mode_var.set(data.get("mode", "-"))
    servo_var.set(str(data.get("servo", "-")) + "°")
    wind_var.set(str(data.get("wind", "-")))
    rain_var.set(str(data.get("rain", "-")))
    vib_var.set(str(data.get("vibration", "-")))
    dist_var.set(str(data.get("distance", "-")) + " cm")
    wifi_var.set(data.get("wifi", "-"))

    latest_arduino_mqtt = data.get("mqtt", "-")
    arduino_mqtt_var.set(latest_arduino_mqtt)

    last_command_var.set(data.get("lastCommand", "-"))

    wildlife = data.get("wildlife", 0) == 1
    storm = data.get("storm", 0) == 1
    vibration_alert = data.get("vibrationAlert", 0) == 1
    ultrasonic_fault = data.get("ultrasonicFault", 0) == 1

    update_alert_label(wildlife_label, wildlife, "WILDLIFE ALERT")
    update_alert_label(storm_label, storm, "STORM WARNING")
    update_alert_label(vibration_label, vibration_alert, "VIBRATION ALERT")
    update_alert_label(ultra_label, ultrasonic_fault, "ULTRASONIC FAULT")

    if wildlife or storm or vibration_alert or ultrasonic_fault:
        health_var.set("WARNING / CHECK SYSTEM")
        health_label.config(foreground="red")
        alarm_on()
    else:
        health_var.set("NORMAL")
        health_label.config(foreground="green")
        alarm_off()

    connection_var.set("RECEIVING ARDUINO DATA")
    connection_label.config(foreground="green")


def update_broker_status(status):
    if status == "CONNECTED":
        broker_var.set("CONNECTED")
        broker_label.config(foreground="green")
        log_message("Broker status: CONNECTED")
    else:
        broker_var.set("DISCONNECTED")
        broker_label.config(foreground="red")


def process_queue():
    while not message_queue.empty():
        msg_type, content = message_queue.get()

        if msg_type == "status":
            update_dashboard(content)

        elif msg_type == "log":
            log_message(content)

        elif msg_type == "broker_status":
            update_broker_status(content)

    if not mqtt_connected:
        connection_var.set("DASHBOARD MQTT DISCONNECTED")
        connection_label.config(foreground="red")

    elif last_status_time == 0:
        connection_var.set("BROKER CONNECTED - WAITING FOR ARDUINO DATA")
        connection_label.config(foreground="orange")

    elif time.time() - last_status_time > DATA_TIMEOUT:
        connection_var.set("NO RECENT ARDUINO DATA")
        connection_label.config(foreground="orange")
        alarm_on()

    root.after(200, process_queue)


def log_message(message):
    timestamp = time.strftime("%H:%M:%S")
    log_box.insert(tk.END, "[" + timestamp + "] " + message + "\n")
    log_box.see(tk.END)


# -------------------- BUTTON FUNCTIONS --------------------

def set_auto():
    send_command("AUTO")


# SAFE bị bỏ — CENTER thay thế, quay về 90° rồi về AUTO
def center_servo():
    servo_scale.set(90)
    send_command("CENTER")


def servo_slider_changed(value):
    angle = int(float(value))
    servo_slider_value.set(str(angle) + "°")


def send_servo_angle():
    angle = int(servo_scale.get())
    send_command("SERVO:" + str(angle))


def quick_left():
    servo_scale.set(0)
    send_command("SERVO:0")


def quick_diag_left():
    servo_scale.set(45)
    send_command("SERVO:45")


def quick_center():
    servo_scale.set(90)
    send_command("CENTER")


def quick_diag_right():
    servo_scale.set(135)
    send_command("SERVO:135")


def quick_right():
    servo_scale.set(180)
    send_command("SERVO:180")


# -------------------- TKINTER UI --------------------

root = tk.Tk()
root.title("Smart Wind Turbine Monitoring Dashboard")
root.geometry("920x720")

style = ttk.Style()
style.configure("Title.TLabel", font=("Arial", 20, "bold"))
style.configure("Header.TLabel", font=("Arial", 13, "bold"))
style.configure("Value.TLabel", font=("Arial", 12))

connection_var = tk.StringVar(value="CONNECTING...")
broker_var = tk.StringVar(value="-")

mode_var = tk.StringVar(value="-")
servo_var = tk.StringVar(value="-")
wind_var = tk.StringVar(value="-")
rain_var = tk.StringVar(value="-")
vib_var = tk.StringVar(value="-")
dist_var = tk.StringVar(value="-")
wifi_var = tk.StringVar(value="-")
arduino_mqtt_var = tk.StringVar(value="-")
last_command_var = tk.StringVar(value="-")
health_var = tk.StringVar(value="-")
servo_slider_value = tk.StringVar(value="90°")

ttk.Label(
    root,
    text="Smart Wind Turbine Monitoring Dashboard",
    style="Title.TLabel"
).pack(pady=15)

connection_label = ttk.Label(root, textvariable=connection_var, style="Header.TLabel")
connection_label.pack()

broker_frame = ttk.Frame(root)
broker_frame.pack(pady=5)

ttk.Label(
    broker_frame,
    text="Dashboard Broker:",
    style="Header.TLabel"
).grid(row=0, column=0, padx=5)

broker_label = ttk.Label(
    broker_frame,
    textvariable=broker_var,
    style="Header.TLabel"
)
broker_label.grid(row=0, column=1, padx=5)

main_frame = ttk.Frame(root)
main_frame.pack(fill="both", expand=True, padx=20, pady=10)

left_frame = ttk.LabelFrame(main_frame, text="Live System Status")
left_frame.grid(row=0, column=0, sticky="nsew", padx=10, pady=10)

right_frame = ttk.LabelFrame(main_frame, text="Manual Turbine Control")
right_frame.grid(row=0, column=1, sticky="nsew", padx=10, pady=10)

main_frame.columnconfigure(0, weight=1)
main_frame.columnconfigure(1, weight=1)

status_items = [
    ("Mode", mode_var),
    ("Servo Angle", servo_var),
    ("Wind Value", wind_var),
    ("Rain Value", rain_var),
    ("Vibration", vib_var),
    ("Distance", dist_var),
    ("Arduino WiFi", wifi_var),
    ("Arduino MQTT", arduino_mqtt_var),
    ("Last Command", last_command_var),
    ("System Health", health_var),
]

for i, (label_text, var) in enumerate(status_items):
    ttk.Label(
        left_frame,
        text=label_text + ":",
        style="Header.TLabel"
    ).grid(row=i, column=0, sticky="w", padx=10, pady=6)

    label = ttk.Label(
        left_frame,
        textvariable=var,
        style="Value.TLabel"
    )
    label.grid(row=i, column=1, sticky="w", padx=10, pady=6)

    if label_text == "System Health":
        health_label = label

alert_frame = ttk.LabelFrame(left_frame, text="Alerts")
alert_frame.grid(row=10, column=0, columnspan=2, sticky="ew", padx=10, pady=15)

wildlife_label = ttk.Label(alert_frame, text="SAFE", foreground="green", font=("Arial", 12, "bold"))
storm_label = ttk.Label(alert_frame, text="SAFE", foreground="green", font=("Arial", 12, "bold"))
vibration_label = ttk.Label(alert_frame, text="SAFE", foreground="green", font=("Arial", 12, "bold"))
ultra_label = ttk.Label(alert_frame, text="SAFE", foreground="green", font=("Arial", 12, "bold"))

wildlife_label.pack(anchor="w", padx=10, pady=3)
storm_label.pack(anchor="w", padx=10, pady=3)
vibration_label.pack(anchor="w", padx=10, pady=3)
ultra_label.pack(anchor="w", padx=10, pady=3)

# Bỏ nút SAFE MODE, thay bằng CENTER
ttk.Button(right_frame, text="AUTO MODE", command=set_auto).pack(fill="x", padx=20, pady=8)
ttk.Button(right_frame, text="CENTER (90°)", command=center_servo).pack(fill="x", padx=20, pady=8)

ttk.Label(right_frame, text="Servo Direction Control", style="Header.TLabel").pack(pady=15)

servo_scale = tk.Scale(
    right_frame,
    from_=0,
    to=180,
    orient="horizontal",
    length=330,
    command=servo_slider_changed
)
servo_scale.set(90)
servo_scale.pack(pady=5)

ttk.Label(right_frame, textvariable=servo_slider_value, font=("Arial", 14, "bold")).pack()

ttk.Button(
    right_frame,
    text="SEND SERVO ANGLE",
    command=send_servo_angle
).pack(fill="x", padx=20, pady=10)

quick_frame = ttk.LabelFrame(right_frame, text="Quick Directions")
quick_frame.pack(fill="x", padx=20, pady=10)

ttk.Button(quick_frame, text="Left 0°",    command=quick_left).grid(row=0, column=0, padx=5, pady=5)
ttk.Button(quick_frame, text="45°",        command=quick_diag_left).grid(row=0, column=1, padx=5, pady=5)
ttk.Button(quick_frame, text="90°",        command=quick_center).grid(row=0, column=2, padx=5, pady=5)
ttk.Button(quick_frame, text="135°",       command=quick_diag_right).grid(row=0, column=3, padx=5, pady=5)
ttk.Button(quick_frame, text="Right 180°", command=quick_right).grid(row=0, column=4, padx=5, pady=5)

log_frame = ttk.LabelFrame(root, text="System Log")
log_frame.pack(fill="both", expand=True, padx=20, pady=10)

log_box = tk.Text(log_frame, height=8)
log_box.pack(fill="both", expand=True, padx=10, pady=10)

mqtt_thread = threading.Thread(target=start_mqtt, daemon=True)
mqtt_thread.start()

try:
    root.after(200, process_queue)
    root.mainloop()

finally:
    alarm_off()
    GPIO.cleanup()
