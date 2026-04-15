#include <WiFiNINA.h>
#include <PubSubClient.h>

char ssid[] = "8Uganda";
char pass[] = "Lmamlmam2025";

const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;

WiFiClient wifiClient;
PubSubClient client(wifiClient);

const int led1 = 3;
const int led2 = 4;
const int echoPin = 5;
const int trigPin = 6;

String lastState = "";

void connectWiFi() {
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(2000);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message received on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(message);
  Serial.println();

  if (String(topic) == "ES/Wave") {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
  }

  if (String(topic) == "ES/Pat") {
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
  }
}

void connectMQTT() {
  while (!client.connected()) {
    if (client.connect("DatClient")) {
      client.subscribe("ES/Wave");
      client.subscribe("ES/Pat");
    } else {
      delay(2000);
    }
  }
}

long readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return -1;

  return duration * 0.034 / 2;
}

void setup() {
  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);

  connectWiFi();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }

  client.loop();

  long d = readDistance();
  String currentState = "";

  if (d > 0 && d < 10) {
    currentState = "PAT";
  }
  else if (d >= 10 && d <= 25) {
    currentState = "WAVE";
  }

  if (currentState != lastState) {
    if (currentState == "PAT") {
      client.publish("ES/Pat", "Nguyen Quoc Dat");
    }
    else if (currentState == "WAVE") {
      client.publish("ES/Wave", "Nguyen Quoc Dat");
    }

    lastState = currentState;
  }

  delay(300);
}
