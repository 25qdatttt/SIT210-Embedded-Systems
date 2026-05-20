import tkinter as tk
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)

LED_PINS = {
    "Living Room": 17,
    "Bathroom": 27,
    "Closet": 22
}

# Setup GPIO
for pin in LED_PINS.values():
    GPIO.setup(pin, GPIO.OUT)
    GPIO.output(pin, GPIO.LOW)

# Function to toggle each LED independently
def toggle_led(room):
    pin = LED_PINS[room]

    if led_vars[room].get() == 1:
        GPIO.output(pin, GPIO.HIGH)
    else:
        GPIO.output(pin, GPIO.LOW)

# Exit function
def exit_app():
    for pin in LED_PINS.values():
        GPIO.output(pin, GPIO.LOW)

    GPIO.cleanup()
    window.destroy()

# GUI window
window = tk.Tk()
window.title("Light Control - Linda")
window.geometry("320x260")

tk.Label(
    window,
    text="Select lights:",
    font=("Arial", 13, "bold")
).pack(pady=15)

# Store variables for each checkbox
led_vars = {}

# Create independent checkboxes
for room in LED_PINS.keys():

    led_vars[room] = tk.IntVar()

    tk.Checkbutton(
        window,
        text=room,
        variable=led_vars[room],
        font=("Arial", 12),
        command=lambda r=room: toggle_led(r)
    ).pack(anchor="w", padx=70, pady=5)

# Exit button
tk.Button(
    window,
    text="Exit",
    command=exit_app,
    bg="red",
    fg="white",
    font=("Arial", 11)
).pack(pady=20)

window.mainloop()
