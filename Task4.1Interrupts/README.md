# Task 4.1P - Handling Interrupts

In this project, a hardware interrupt-based automatic lighting system is built using the Arduino Nano 33 IoT, a PIR motion sensor, and a BH1750 light sensor. The system automatically switches on two LEDs when motion is detected at the door and it is dark, simulating a porch light that switches on when Linda comes home at night.

The program is based on a modular programming approach in which the code is divided into small, targeted functions. The handleSwitch() function processes the slider switch interrupt and allows the LEDs to be overridden manually. The handlePIR() function processes the PIR motion interrupt and checks the current light level before deciding whether to turn the LEDs on. The handleLight() function monitors ambient brightness and automatically turns the LEDs off when the environment becomes bright. This project demonstrates the use of hardware interrupts and sensor-based logic to build a responsive, energy-aware embedded system.
