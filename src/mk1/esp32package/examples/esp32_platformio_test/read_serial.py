import serial
import time
import sys

try:
    with serial.Serial('COM3', 115200, timeout=1) as ser:
        ser.setDTR(False)
        time.sleep(0.1)
        ser.setDTR(True)
        print("Listening to COM3...")
        # wait a bit for ESP32 to boot
        time.sleep(1)
        end_time = time.time() + 45
        while time.time() < end_time:
            line = ser.readline()
            if line:
                print(line.decode('utf-8', errors='ignore').strip())
except Exception as e:
    print(f"Error: {e}")
