import speech_recognition as sr
from bleak import BleakClient
import asyncio

ADDRESS = "..."

CHAR_UUID = "..."

recognizer = sr.Recognizer()

async def send_command(command):

    async with BleakClient(ADDRESS) as client:

        print("Connected to Arduino")

        await client.write_gatt_char(
            CHAR_UUID,
            command.encode()
        )

        print("Sent:", command)

while True:

    with sr.Microphone() as source:

        print("\nListening...")

        recognizer.adjust_for_ambient_noise(
            source,
            duration=1
        )

        audio = recognizer.listen(source)

    try:

        text = recognizer.recognize_google(audio)

        text = text.lower()

        print("You said:", text)

        # Bathroom
        if "bathroom" in text:

            asyncio.run(
                send_command("BATHROOM_ON")
            )

        # Hallway
        elif "hallway" in text:

            asyncio.run(
                send_command("HALLWAY_ON")
            )

        # Turn off
        elif "off" in text:

            asyncio.run(
                send_command("ALL_OFF")
            )

        else:

            print("Unknown command")

    except Exception as e:

        print("Error:", e)
