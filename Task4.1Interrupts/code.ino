#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;

#define PIR_PIN        2
#define SWITCH_PIN     3
#define LED1_PIN       4
#define LED2_PIN       5
#define DARK_THRESHOLD 50.0
#define MIN_ON_TIME    3000

bool ledsOn         = false;
bool switchOverride = false;
int pirState        = LOW;
float currentLux    = 0;
unsigned long lastStatusTime = 0;
unsigned long ledsOnTime     = 0;

volatile bool pirTriggered    = false;
volatile bool switchTriggered = false;

void pirISR() {
  if (digitalRead(PIR_PIN) == HIGH) {
    pirTriggered = true;
  }
}

void switchISR() {
  static unsigned long lastTime = 0;
  unsigned long now = millis();
  if (now - lastTime > 300) {
    lastTime = now;
    switchTriggered = true;
  }
}

void setLEDs(bool state) {
  ledsOn = state;
  digitalWrite(LED1_PIN, state ? HIGH : LOW);
  digitalWrite(LED2_PIN, state ? HIGH : LOW);
}

void handleSwitch() {
  if (!switchTriggered) return;
  switchTriggered = false;
  bool isOn = digitalRead(SWITCH_PIN) == LOW;
  switchOverride = isOn;
  setLEDs(isOn);
  Serial.println(isOn ? "SWITCH ON (manual override)"
: "SWITCH OFF");
}

void handlePIR() {
  if (switchOverride) {
    pirTriggered = false;
    return;
  }

  if (pirTriggered) {
    pirTriggered = false;
    pirState = HIGH;
    Serial.print("PIR Triggered! Lux: ");
    Serial.println(currentLux);
    if (currentLux < DARK_THRESHOLD) {
      setLEDs(true);
      ledsOnTime = millis();
      Serial.println("Dark + Motion - LEDs ON");
    } else {
      Serial.println("Bright - LEDs stay OFF");
    }
  }

  if (pirState == HIGH && !ledsOn && currentLux < DARK_THRESHOLD) {
    setLEDs(true);
    ledsOnTime = millis();
    Serial.println("Dark again + Motion still - LEDs ON!");
  }

  if (pirState == HIGH && digitalRead(PIR_PIN) == LOW) {
    if (millis() - ledsOnTime > MIN_ON_TIME) {
      pirState = LOW;
      setLEDs(false);
      Serial.println("No Motion - LEDs OFF");
    }
  }
}

void handleLight() {
  if (switchOverride) return;

  if (ledsOn && currentLux >= DARK_THRESHOLD) {
    setLEDs(false);
    Serial.println("Bright - LEDs OFF");
  }
}

void printStatus() {
  if (millis() - lastStatusTime < 3000) return;
  lastStatusTime = millis();
  Serial.println("SYSTEM STATUS");
  Serial.print("  Light : ");
  Serial.print(currentLux);
  Serial.println(currentLux < DARK_THRESHOLD ? " lux -> DARK" : " lux -> BRIGHT");
  Serial.print("  PIR   : ");
  Serial.println(pirState == HIGH ? "Motion" : "No Motion");
  Serial.print("  Switch: ");
  Serial.println(digitalRead(SWITCH_PIN) == LOW ? "RIGHT (ON)" : "LEFT (OFF)");
  Serial.print("  Override: ");
  Serial.println(switchOverride ? "YES" : "NO");
  Serial.print("  LEDs  : ");
  Serial.println(ledsOn ? "ON" : "OFF");
  Serial.println("-----");
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lightMeter.begin();

  pinMode(PIR_PIN,    INPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(LED1_PIN,   OUTPUT);
  pinMode(LED2_PIN,   OUTPUT);

  attachInterrupt(digitalPinToInterrupt(PIR_PIN),    pirISR,    CHANGE);
  attachInterrupt(digitalPinToInterrupt(SWITCH_PIN), switchISR, CHANGE);

  Serial.println("System Ready");
  Serial.println("Switch LEFT = OFF | Switch RIGHT = ON");
  delay(2000);
}

void loop() {
  currentLux = lightMeter.readLightLevel();

  handleSwitch();
  handlePIR();
  handleLight();
  printStatus();

  delay(100);
}