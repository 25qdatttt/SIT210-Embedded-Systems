#include "thingProperties.h"

const int livingPin = 3;
const int bathroomPin = 4;
const int closetPin = 5;

void setup() {
  Serial.begin(9600);
  delay(1500);

  pinMode(livingPin, OUTPUT);
  pinMode(bathroomPin, OUTPUT);
  pinMode(closetPin, OUTPUT);

  digitalWrite(livingPin, LOW);
  digitalWrite(bathroomPin, LOW);
  digitalWrite(closetPin, LOW);

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  Serial.println("System ready.");
}

void loop() {
  ArduinoCloud.update();
}

void onSelectedRoomChange() {
  selectedRoom.trim();

  Serial.print("Received command: ");
  Serial.println(selectedRoom);

  if (selectedRoom == "living_on") {
    digitalWrite(livingPin, HIGH);
    Serial.println("Living room LED ON");
  }
  else if (selectedRoom == "living_off") {
    digitalWrite(livingPin, LOW);
    Serial.println("Living room LED OFF");
  }
  else if (selectedRoom == "bathroom_on") {
    digitalWrite(bathroomPin, HIGH);
    Serial.println("Bathroom LED ON");
  }
  else if (selectedRoom == "bathroom_off") {
    digitalWrite(bathroomPin, LOW);
    Serial.println("Bathroom LED OFF");
  }
  else if (selectedRoom == "closet_on") {
    digitalWrite(closetPin, HIGH);
    Serial.println("Closet LED ON");
  }
  else if (selectedRoom == "closet_off") {
    digitalWrite(closetPin, LOW);
    Serial.println("Closet LED OFF");
  }
  else {
    Serial.println("Unknown command");
  }
}