import tkinter as tk
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

LED_PINS = {
    "Living Room": 18,
    "Bathroom": 27,
    "Closet": 22
}

# Setup pins
for pin in LED_PINS.values():
    GPIO.setup(pin, GPIO.OUT)
    GPIO.output(pin, GPIO.LOW)

# PWM setup for Living Room
pwm = GPIO.PWM(LED_PINS["Living Room"], 100)
pwm.start(0)

current_room = "Living Room"

def control_led(room):
    global current_room
    current_room = room

    for name, pin in LED_PINS.items():
        if name == "Living Room":
            continue
        GPIO.output(pin, GPIO.HIGH if name == room else GPIO.LOW)

    if room == "Living Room":
        duty = brightness_slider.get()
        pwm.ChangeDutyCycle(duty)
    else:
        pwm.ChangeDutyCycle(0)


def update_brightness(value):
    duty = float(value)
    brightness_label.config(text="Brightness: " + str(int(duty)) + "%")

    if current_room == "Living Room":
        pwm.ChangeDutyCycle(duty)


def exit_app():
    pwm.stop()
    for pin in LED_PINS.values():
        GPIO.output(pin, GPIO.LOW)
    GPIO.cleanup()
    window.destroy()


# GUI
window = tk.Tk()
window.title("Light Control - Linda")
window.geometry("360x340")

tk.Label(
    window,
    text="Select a room to control the light:",
    font=("Arial", 13, "bold")
).pack(pady=15)

selected = tk.StringVar(value="Living Room")

for room in LED_PINS.keys():
    tk.Radiobutton(
        window,
        text=room,
        variable=selected,
        value=room,
        font=("Arial", 12),
        command=lambda r=room: control_led(r)
    ).pack(anchor="w", padx=80, pady=4)

tk.Label(
    window,
    text="Living Room Brightness",
    font=("Arial", 12, "bold")
).pack(pady=10)

brightness_label = tk.Label(window, text="Brightness: 50%")
brightness_label.pack()

brightness_slider = tk.Scale(
    window,
    from_=0,
    to=100,
    orient="horizontal",
    length=220,
    command=update_brightness
)
brightness_slider.set(50)
brightness_slider.pack(pady=10)

tk.Button(
    window,
    text="Exit",
    command=exit_app,
    bg="red",
    fg="white",
    font=("Arial", 11, "bold"),
    width=10
).pack(pady=15)

# Start default
control_led("Living Room")

window.mainloop()