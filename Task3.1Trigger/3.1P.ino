#include <WiFiNINA.h>
#include <Wire.h>
#include <BH1750.h>

#define LIGHT_THRESHOLD 1000.0

char ssid[] = "8Uganda";
char pass[] = "Lmamlmam2025";
String key = "j8TDPfI9QSrtLgPMj3hRV";

BH1750 sensor;
bool lightOn = false;

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(2000);
  }
  Serial.println(" Done!");
}

void notify(String event, float lux) {
  WiFiClient client;
  String request = "/trigger/" + event + "/with/key/"
                 + key + "?value1=" + lux;

  if (client.connect("maker.ifttt.com", 80)) {
    client.println("GET " + request + " HTTP/1.1");
    client.println("Host: maker.ifttt.com");
    client.println("Connection: close");
    client.println();
    Serial.println("Notification sent: " + event);
    delay(5000);
  } else {
    Serial.println("Failed to reach IFTTT");
  }

  client.stop();
}

void checkLight(float lux) {
  if (!lightOn && lux > LIGHT_THRESHOLD) {
    lightOn = true;
    notify("sunlight_on", lux);
  }

  if (lightOn && lux <= LIGHT_THRESHOLD) {
    lightOn = false;
    notify("sunlight_off", lux);
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  sensor.begin();
  connectWiFi();
  lightOn = (sensor.readLightLevel() > LIGHT_THRESHOLD);
}

void loop() {
  float lux = sensor.readLightLevel();

  Serial.print("Light level: ");
  Serial.print(lux);
  Serial.println(" lx");

  checkLight(lux);
  delay(3000);
}