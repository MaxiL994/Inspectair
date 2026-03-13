"""
Auto-Upload Script: USB wenn verbunden, sonst OTA.
Wird von PlatformIO beim Konfigurieren ausgeführt (pre: script).
"""
Import("env")

import serial.tools.list_ports

OTA_HOST = "inspectair.local"

# Bekannte ESP32-S3 USB VID/PIDs
esp_vids = [0x303A, 0x10C4, 0x1A86]

usb_found = False
for port in serial.tools.list_ports.comports():
    if port.vid in esp_vids:
        print(f">>> USB erkannt: {port.device} ({port.description})")
        env.Replace(UPLOAD_PORT=port.device)
        usb_found = True
        break

if not usb_found:
    print(f">>> Kein USB erkannt -> OTA Upload an {OTA_HOST}")
    env.Replace(
        UPLOAD_PROTOCOL="espota",
        UPLOAD_PORT=OTA_HOST,
        UPLOAD_FLAGS=["--port=3232"]
    )
