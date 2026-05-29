#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <Servo.h>

// =========================
// WiFi + MQTT
// =========================
const char WIFI_SSID[] = "YourWifi";
const char WIFI_PASS[] = "Passwords";

const char MQTT_BROKER[] = "...";  // broker.hivemq.com
const int MQTT_PORT = 1883;

const char STATUS_TOPIC[]  = "dat/smart_turbine/status";
const char COMMAND_TOPIC[] = "dat/smart_turbine/command";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
Servo turbineServo;

// =========================
// Pins
// =========================
const int potPin       = A0;
const int rainPin      = A1;
const int vibrationPin = 2;
const int trigPin      = 3;
const int echoPin      = 4;
const int servoPin     = 5;

// =========================
// Thresholds
// =========================
const int rainThreshold        = 400;
const int wildlifeDistanceLimit = 15;
const int servoHome            = 90;

// =========================
// System state
// =========================
String systemMode  = "AUTO";
String lastCommand = "NONE";

bool wildlifeAlert   = false;
bool stormAlert      = false;
bool vibrationAlert  = false;
bool ultrasonicFault = false;
bool wifiWasConnected = false;
bool riskActive      = false;   // track status risk to avoid spam IFTTT

float lastValidDistance = 100.0;
int   ultrasonicFailCount = 0;
const int maxUltrasonicFails = 5;

int currentServoAngle = servoHome;

// =========================
// Timing
// =========================
unsigned long lastStatusTime   = 0;
unsigned long lastWiFiAttempt  = 0;
unsigned long lastMQTTAttempt  = 0;
unsigned long lastCommandTime  = 0;
unsigned long lastIFTTTTime    = 0; 

const unsigned long statusInterval   = 500;
const unsigned long reconnectInterval = 5000;
const unsigned long commandTimeout   = 10000;
const unsigned long iftttCooldown    = 60000;

// =========================
// IFTTT Config 
// =========================
const char IFTTT_KEY[]   = "YourKey";
const char IFTTT_EVENT[] = "turbine_alert";
const char IFTTT_HOST[]  = "maker.ifttt.com";

// =========================
// Setup
// =========================
void setup() {
  Serial.begin(9600);
  delay(2000);

  setupPins();
  setupServo();
  setupMQTT();
  connectWiFi();

  Serial.println("Smart Wind Turbine MQTT System Ready");
}

// =========================
// Main loop
// =========================
void loop() {
  maintainWiFi();
  maintainMQTT();
  handleCommunicationFallback();

  if (mqttClient.connected()) {
    mqttClient.loop();
  }

  readSensors();
  updateMode();
  controlTurbine();

  if (millis() - lastStatusTime >= statusInterval) {
    publishStatus();
    lastStatusTime = millis();
  }
}

// =========================
// Setup helpers
// =========================
void setupPins() {
  pinMode(vibrationPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void setupServo() {
  turbineServo.attach(servoPin);
  moveServo(servoHome);
  delay(1000);
}

void setupMQTT() {
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(handleMQTTCommand);
  mqttClient.setBufferSize(512);
}

// =========================
// Connecting WiFi
// =========================
void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected! IP: ");
    Serial.println(WiFi.localIP());
    wifiWasConnected = true;
  } else {
    Serial.println("WiFi failed. Continuing in AUTO mode...");
  }
}

// =========================
// WiFi maintenance - NON-BLOCKING
// =========================
void maintainWiFi() {
  bool currentlyConnected = (WiFi.status() == WL_CONNECTED);

  if (wifiWasConnected && !currentlyConnected) {
    Serial.println("WiFi lost! Resetting...");
    WiFi.disconnect();
    WiFi.end();
    mqttClient.disconnect();
    lastMQTTAttempt = 0;
    delay(500);
  }

  wifiWasConnected = currentlyConnected;
  if (currentlyConnected) return;

  if (millis() - lastWiFiAttempt >= reconnectInterval) {
    lastWiFiAttempt = millis();
    Serial.println("Retrying WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }
}

// =========================
// MQTT maintenance - NON-BLOCKING
// =========================
void maintainMQTT() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (mqttClient.connected()) return;

  if (millis() - lastMQTTAttempt >= reconnectInterval) {
    lastMQTTAttempt = millis();

    String clientId = "Nano33IoT_Turbine_";
    clientId += String(random(1000, 9999));

    Serial.println("Connecting to MQTT...");

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("MQTT connected!");
      mqttClient.subscribe(COMMAND_TOPIC);
    } else {
      int rc = mqttClient.state();
      Serial.print("MQTT failed, rc=");
      Serial.print(rc);
      switch (rc) {
        case -4: Serial.println(" (TIMEOUT)"); break;
        case -3: Serial.println(" (LOST)"); break;
        case -2: Serial.println(" (FAILED - broker unreachable)"); break;
        case -1: Serial.println(" (DISCONNECTED)"); break;
        case  1: Serial.println(" (BAD PROTOCOL)"); break;
        case  2: Serial.println(" (BAD CLIENT ID)"); break;
        case  3: Serial.println(" (UNAVAILABLE)"); break;
        case  4: Serial.println(" (BAD CREDENTIALS)"); break;
        case  5: Serial.println(" (UNAUTHORIZED)"); break;
        default: Serial.println(" (UNKNOWN)"); break;
      }
    }
  }
}

// =========================
// IFTTT Webhook
// =========================
void sendIFTTT(String value1, String value2, String value3) {
  if (WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastIFTTTTime < iftttCooldown) return;

  WiFiClient httpClient;

  String body = "{\"value1\":\"" + value1 + "\","
                + "\"value2\":\"" + value2 + "\","
                + "\"value3\":\"" + value3 + "\"}";

  String path = "/trigger/";
  path += IFTTT_EVENT;
  path += "/with/key/";
  path += IFTTT_KEY;

  if (httpClient.connect(IFTTT_HOST, 80)) {
    httpClient.println("POST " + path + " HTTP/1.1");
    httpClient.println("Host: " + String(IFTTT_HOST));
    httpClient.println("Content-Type: application/json");
    httpClient.println("Content-Length: " + String(body.length()));
    httpClient.println("Connection: close");
    httpClient.println();
    httpClient.println(body);

    lastIFTTTTime = millis();
    Serial.println("IFTTT alert sent: " + value1);

    httpClient.stop();
  } else {
    Serial.println("IFTTT connection failed");
  }
}

// =========================
// Fault tolerance
// =========================
void handleCommunicationFallback() {
  bool communicationLost =
    WiFi.status() != WL_CONNECTED ||
    !mqttClient.connected();

  if (communicationLost && systemMode == "MANUAL") {
    if (millis() - lastCommandTime > commandTimeout) {
      systemMode = "AUTO";
      lastCommand = "AUTO_FALLBACK";
      Serial.println("Communication lost. Returning to AUTO mode.");
    }
  }
}

// =========================
// MQTT command handling
// =========================
void handleMQTTCommand(char* topic, byte* payload, unsigned int length) {
  String command = "";
  for (unsigned int i = 0; i < length; i++) {
    command += (char)payload[i];
  }

  command.trim();
  command.toUpperCase();
  if (command.length() == 0) return;

  lastCommand = command;
  lastCommandTime = millis();

  Serial.print("MQTT command received: ");
  Serial.println(command);

  if (command == "AUTO") {
    systemMode = "AUTO";
    riskActive = false;
  }
  else if (command == "CENTER") {
    moveServo(servoHome);
    systemMode = "AUTO";
    riskActive = false;
  }
  else if (command.startsWith("SERVO:")) {
    int angle = command.substring(6).toInt();
    systemMode = "MANUAL";
    moveServo(angle);
  }
}

// =========================
// Sensor reading
// =========================
void readSensors() {
  int rainValue      = analogRead(rainPin);
  int vibrationValue = digitalRead(vibrationPin);
  float distance     = readDistanceSafe();

  wildlifeAlert  = false;
  stormAlert     = false;
  vibrationAlert = false;

  if (!ultrasonicFault && distance < wildlifeDistanceLimit) {
    wildlifeAlert = true;
  }
  if (rainValue < rainThreshold) {
    stormAlert = true;
  }
  if (vibrationValue == HIGH) {
    vibrationAlert = true;
  }
}

float readDistanceSafe() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0) {
    ultrasonicFailCount++;
    if (ultrasonicFailCount >= maxUltrasonicFails) ultrasonicFault = true;
    return lastValidDistance;
  }

  float distance = duration * 0.034 / 2;

  if (distance < 2 || distance > 400) {
    ultrasonicFailCount++;
    if (ultrasonicFailCount >= maxUltrasonicFails) ultrasonicFault = true;
    return lastValidDistance;
  }

  ultrasonicFailCount = 0;
  ultrasonicFault = false;
  lastValidDistance = distance;
  return distance;
}

// =========================
// Mode logic + IFTTT trigger
// =========================
void updateMode() {
  bool seriousRisk = stormAlert || vibrationAlert || wildlifeAlert || ultrasonicFault;

  if (systemMode == "AUTO" && seriousRisk) {
    moveServo(servoHome);
    Serial.println("Risk detected! Centering servo.");

    // Chỉ gửi IFTTT khi risk mới xuất hiện (không spam)
    if (!riskActive) {
      riskActive = true;

      String alerts = "";
      if (wildlifeAlert)   alerts += "Wildlife ";
      if (stormAlert)      alerts += "Storm ";
      if (vibrationAlert)  alerts += "Vibration ";
      if (ultrasonicFault) alerts += "UltrasonicFault";

      sendIFTTT(
        alerts,
        "Mode: AUTO | Servo: CENTER",
        "Dist: " + String(lastValidDistance, 1) + "cm"
      );
    }
  }

  if (!seriousRisk) {
    riskActive = false;
  }
}

// =========================
// Turbine control
// =========================
void controlTurbine() {
  // Nếu có risk → servo đã center trong updateMode, không override
  if (systemMode == "AUTO" && !riskActive) {
    int windValue = analogRead(potPin);
    int angle = map(windValue, 0, 1023, 0, 180);
    moveServo(angle);
  }
}

void moveServo(int angle) {
  angle = constrain(angle, 0, 180);
  if (angle != currentServoAngle) {
    turbineServo.write(angle);
    currentServoAngle = angle;
  }
}

// =========================
// Publish status
// =========================
void publishStatus() {
  int windValue      = analogRead(potPin);
  int rainValue      = analogRead(rainPin);
  int vibrationValue = digitalRead(vibrationPin);

  String json = "{";
  json += "\"mode\":\""           + systemMode + "\",";
  json += "\"servo\":"            + String(currentServoAngle) + ",";
  json += "\"wind\":"             + String(windValue) + ",";
  json += "\"rain\":"             + String(rainValue) + ",";
  json += "\"vibration\":"        + String(vibrationValue) + ",";
  json += "\"distance\":"         + String(lastValidDistance, 1) + ",";
  json += "\"wildlife\":"         + String(wildlifeAlert   ? 1 : 0) + ",";
  json += "\"storm\":"            + String(stormAlert      ? 1 : 0) + ",";
  json += "\"vibrationAlert\":"   + String(vibrationAlert  ? 1 : 0) + ",";
  json += "\"ultrasonicFault\":"  + String(ultrasonicFault ? 1 : 0) + ",";
  json += "\"ultrasonicFailCount\":" + String(ultrasonicFailCount) + ",";
  json += "\"wifi\":\""  + String(WiFi.status() == WL_CONNECTED ? "CONNECTED" : "DISCONNECTED") + "\",";
  json += "\"mqtt\":\""  + String(mqttClient.connected()        ? "CONNECTED" : "DISCONNECTED") + "\",";
  json += "\"lastCommand\":\"" + lastCommand + "\"";
  json += "}";

  Serial.println(json);

  if (mqttClient.connected()) {
    mqttClient.publish(STATUS_TOPIC, json.c_str());
  }
}
