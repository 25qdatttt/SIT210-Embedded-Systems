#include <WiFiNINA.h>
#include <ThingSpeak.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>

#define DHTPIN 2
#define DHTTYPE DHT22

char wifiName[] = "8Uganda";
char wifiPassword[] = "Lmamlmam2025";

unsigned long myChannel = 3301549;
const char* myAPI = "1SSHBGTZFP0I8E0G";

WiFiClient wifiClient;
DHT dhtSensor(DHTPIN, DHTTYPE);
BH1750 lightMeter;

void startSensors() {
  dhtSensor.begin();
  Wire.begin();
  lightMeter.begin();
}

void connectWifi() {
  WiFi.begin(wifiName, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting");
    delay(1000);
  }

  Serial.println("Connected");
}

void show(float temp, float lightLv) {
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" C");

  Serial.print("Light: ");
  Serial.print(lightLv);
  Serial.println(" lux");
}

void upload(float temp, float lightLv) {
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, lightLv);
  ThingSpeak.writeFields(myChannel, myAPI);
}

void setup() {
  Serial.begin(9600);

  startSensors();
  connectWifi();
  ThingSpeak.begin(wifiClient);
}

void loop() {
  float temp = dhtSensor.readTemperature();
  float lightLv = lightMeter.readLightLevel();

  if (isnan(temp)) {
    Serial.println("Failed to read DHT22");
  } else {
    show(temp, lightLv);
    upload(temp, lightLv);
  }
 
  delay(30000);
}
