# Task 3.2 – MQTT

In this project, a simple gesture control system is implemented using an Arduino Nano 33 IoT, an ultrasonic sensor, and two LEDs. The system measures the distance between the user's hand and the sensor to identify basic gestures. A short distance is interpreted as a "pat", while a medium distance is considered a "wave". The LEDs represent lights that can be switched on or off based on these gestures.

The program connects to a WiFi network and communicates with an MQTT broker (broker.emqx.io) using a publish–subscribe model. When a gesture is detected, the Arduino sends a message containing the user's name to either the ES/Pat or ES/Wave topic. At the same time, it subscribes to these topics and reacts to received messages by controlling the LEDs.
