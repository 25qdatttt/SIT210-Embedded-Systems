# Task 4.2 Cloud Function

In this project, Python (Flask), Arduino IoT Cloud and the basic HTML interface are used to control three LEDs connected to an Arduino Nano 33 IoT. The LEDs make up all the rooms in a house, such as the living room, bathroom, and closet. The system enables the user to switch on or off each of the lights using a web page.

The user clicks a button on the web interface, and a request is sent to the Flask backend. The backend handles the command, and forwards the command to Arduino IoT Cloud through updating a property. This update is sent to the Arduino device and alter the state of the LEDs.

The project demonstrates the interplay between a web interface, a backend system, a cloud platform, and an embedded device to create a simple IoT-based smart home system.
