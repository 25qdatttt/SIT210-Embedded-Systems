#include <Wire.h>
#include <BH1750.h>
#include <ArduinoBLE.h>

BH1750 lightMeter;

// LED pins
const int bathroomLED = 2;
const int hallwayLED  = 3;
const int fanLED      = 4;

// Light threshold
float threshold = 200.0;

// BLE Service
BLEService lightService(
  "..."
);

// BLE Characteristic
BLEStringCharacteristic commandChar(
  "...",
  BLERead | BLEWrite,
  50
);

// Setup Functions
void setupLEDs() {

  pinMode(bathroomLED, OUTPUT);
  pinMode(hallwayLED, OUTPUT);
  pinMode(fanLED, OUTPUT);
}

void setupSensor() {

  Wire.begin();
  lightMeter.begin();

  Serial.println("BH1750 ready");
}

void setupBLE() {

  if (!BLE.begin()) {

    Serial.println("BLE failed!");

    while (1);
  }

  Serial.println("BLE started");

  BLE.setLocalName("Nano33IoT");

  BLE.setAdvertisedService(lightService);

  lightService.addCharacteristic(commandChar);

  BLE.addService(lightService);

  commandChar.writeValue("");

  BLE.advertise();

  Serial.println("BLE Device Ready");
}

// Lighting Functions
void handleBathroom() {

  digitalWrite(bathroomLED, HIGH);

  digitalWrite(fanLED, HIGH);

  digitalWrite(hallwayLED, LOW);

  Serial.println("Bathroom ON");
}

void handleHallway() {

  digitalWrite(hallwayLED, HIGH);

  digitalWrite(bathroomLED, LOW);

  digitalWrite(fanLED, LOW);

  Serial.println("Hallway ON");
}

void turnOffAll() {

  digitalWrite(bathroomLED, LOW);

  digitalWrite(hallwayLED, LOW);

  digitalWrite(fanLED, LOW);

  Serial.println("All OFF");
}

// Main Setup
void setup() {

  Serial.begin(9600);

  delay(2000);

  Serial.println("Starting...");

  setupLEDs();

  setupSensor();

  setupBLE();
}

// Main Loop
void loop() {

  BLEDevice central = BLE.central();

  if (central) {

    Serial.println("Connected to Raspberry Pi");

    while (central.connected()) {

      if (commandChar.written()) {

        String command = commandChar.value();

        Serial.print("Received: ");

        Serial.println(command);

        float lux = lightMeter.readLightLevel();

        Serial.print("Light: ");

        Serial.println(lux);

        // Bathroom
        if (command == "BATHROOM_ON") {

          if (lux < threshold) {

            handleBathroom();
          }
        }

        // Hallway
        else if (command == "HALLWAY_ON") {

          if (lux < threshold) {

            handleHallway();
          }
        }

        // All OFF
        else if (command == "ALL_OFF") {

          turnOffAll();
        }
      }
    }

    Serial.println("Disconnected");
  }
}
