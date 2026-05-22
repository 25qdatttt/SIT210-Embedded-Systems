#include <WiFiNINA.h>
#include <ThingSpeak.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11   // nếu dùng DHT22 thì đổi thành DHT22

DHT dhtSensor(DHTPIN, DHTTYPE);

char wifiName[] = "qdattt";
char wifiPassword[] = "25092006";

unsigned long myChannel = 3392155;
const char* myAPI = "5LUNSX4JN8C2AJMK";

WiFiClient wifiClient;

const int lightPin = A0;

void startSensors() {
  dhtSensor.begin();
}

void connectWifi() {
  WiFi.begin(wifiName, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting");
    delay(1000);
  }

  Serial.println("Connected");
}

void show(float temp, float humidity, int lightValue) {
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Light: ");
  Serial.println(lightValue);
}

void upload(float temp, float humidity, int lightValue) {
  ThingSpeak.setField(1, temp);
  ThingSpeak.setField(2, lightValue);
  ThingSpeak.setField(3, humidity);

  int status = ThingSpeak.writeFields(myChannel, myAPI);

  Serial.print("ThingSpeak status: ");
  Serial.println(status);
}

void setup() {
  Serial.begin(9600);

  startSensors();
  connectWifi();
  ThingSpeak.begin(wifiClient);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }

  float temp = dhtSensor.readTemperature();
  float humidity = dhtSensor.readHumidity();
  int lightValue = analogRead(lightPin);

  if (isnan(temp) || isnan(humidity)) {
    Serial.println("Failed to read DHT sensor");
  } else {
    show(temp, humidity, lightValue);
    upload(temp, humidity, lightValue);
  }

  delay(30000);
}
