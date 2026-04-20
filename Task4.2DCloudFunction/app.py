from flask import Flask, request, jsonify, send_from_directory
import requests
import time

app = Flask(__name__)

CLIENT_ID = "dirdKOS3EvQgr4mi2s90sar1RoBp4HMT"
CLIENT_SECRET = "rQqkEF1lS9jLnOfwT2v17V7csMDp63Nlg6v8gLZZJd1dT95HET9nOwFoKgIWOK5R"
THING_ID = "22d47625-dcfc-4b2b-881c-a2f62e3a942e"
PROPERTY_NAME = "selectedRoom"
PROPERTY_ID = ""

access_token = None
expire_time = 0


def get_access_token():
    global access_token, expire_time

    if access_token and time.time() < expire_time:
        return access_token

    url = "https://api2.arduino.cc/iot/v1/clients/token"

    data = {
        "grant_type": "client_credentials",
        "client_id": CLIENT_ID,
        "client_secret": CLIENT_SECRET,
        "audience": "https://api2.arduino.cc/iot"
    }

    headers = {
        "content-type": "application/x-www-form-urlencoded"
    }

    response = requests.post(url, data=data, headers=headers)

    if response.status_code != 200:
        print("Error getting token:")
        print(response.text)
        return None

    result = response.json()
    access_token = result["access_token"]
    expires_in = result["expires_in"]
    expire_time = time.time() + expires_in - 60

    print("Access token generated")
    return access_token


def get_property_id():
    global PROPERTY_ID

    if PROPERTY_ID.strip() != "":
        return PROPERTY_ID

    token = get_access_token()
    if not token:
        return None

    url = f"https://api2.arduino.cc/iot/v2/things/{THING_ID}"
    headers = {
        "Authorization": f"Bearer {token}"
    }

    response = requests.get(url, headers=headers)

    if response.status_code != 200:
        print("Error getting Thing details:")
        print(response.text)
        return None

    result = response.json()

    for prop in result.get("properties", []):
        if prop.get("name") == PROPERTY_NAME:
            PROPERTY_ID = prop.get("id", "")
            print("Property ID found:", PROPERTY_ID)
            return PROPERTY_ID

    print("Property not found")
    return None


@app.route("/")
def home():
    return send_from_directory(".", "index.html")


@app.route("/send", methods=["POST"])
def send_command():
    data = request.get_json()
    command = data.get("command", "").strip()

    if command == "":
        return jsonify({"success": False, "message": "Empty command"}), 400

    token = get_access_token()
    if not token:
        return jsonify({"success": False, "message": "Cannot get token"}), 500

    property_id = get_property_id()
    if not property_id:
        return jsonify({"success": False, "message": "Cannot get property ID"}), 500

    url = f"https://api2.arduino.cc/iot/v2/things/{THING_ID}/properties/{property_id}/publish"

    headers = {
        "Authorization": f"Bearer {token}",
        "Content-Type": "application/json"
    }

    payload = {
        "value": command
    }

    response = requests.put(url, headers=headers, json=payload)

    print("Command sent:", command)
    print("Status code:", response.status_code)
    print("Response:", response.text)

    if response.status_code not in [200, 201]:
        return jsonify({
            "success": False,
            "status_code": response.status_code,
            "response": response.text
        }), response.status_code

    return jsonify({
        "success": True,
        "status_code": response.status_code,
        "response": response.text
    })


if __name__ == "__main__":
    print("Starting server...")
    print("Thing ID:", THING_ID)
    print("Property name:", PROPERTY_NAME)
    app.run(debug=True)