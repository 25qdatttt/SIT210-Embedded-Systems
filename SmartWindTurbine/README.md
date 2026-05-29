# Smart Wind Turbine — Source Code

## Files

| File | Description |
|---|---|
| `sketch_turbine.ino` | Arduino Nano 33 IoT firmware |
| `dashboard.py` | Raspberry Pi monitoring dashboard |

---

## Setup

### 1. Arduino (`sketch_turbine.ino`)

Install these libraries in Arduino IDE Library Manager:
- `WiFiNINA`
- `PubSubClient`
- `Servo`

Update your credentials before uploading:
```cpp
const char WIFI_SSID[] = "your_wifi_name";
const char WIFI_PASS[] = "your_wifi_password";
const char IFTTT_KEY[] = "your_ifttt_key";
```

### 2. Raspberry Pi (`dashboard.py`)

```bash
pip install paho-mqtt requests
python3 dashboard.py
```

---

## MQTT Topics

| Topic | Direction |
|---|---|
| `dat/smart_turbine/status` | Arduino → Pi |
| `dat/smart_turbine/command` | Pi → Arduino |

## Commands

| Command | Description |
|---|---|
| `AUTO` | Switch to AUTO mode |
| `CENTER` | Return servo to 90° |
| `SERVO:90` | Set specific angle (0–180) |
