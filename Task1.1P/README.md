# Task 1.1P – Switching ON Lights

## System Description

This project is a simulation of a simple smart lighting system for an assisted living home. The system contains an Arduino board, two LEDs, and a push button. The LEDs symbolize the porch light and the hallway light of a house.

When the push button is pressed, the porch light is triggered to turn on for 30 seconds in order to shine in the entrance area. After that, the hallway light is switched on for 60 seconds to aid the resident to safely enter the house. This system is an example of how embedded systems can be used to automate small tasks in order to improve safety and convenience.

## Code Structure

The program is coded using a modular programming approach, where the code is broken into several small functions. This makes the program easier to understand and modify.

The setupPins() function sets up the pins for the LEDs and the push button. The checkButton() function continuously checks whether the button is pressed. When the button is pressed, the program triggers the lighting sequence.

The turnPorch() function controls the porch light and keeps it on for 30 seconds. The turnHallway() function controls the hallway light and keeps it on for 60 seconds. Separating these tasks into different functions keeps the code cleaner and makes it easier to maintain if the system needs to be expanded in the future.
