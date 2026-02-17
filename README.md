# ESP32-WebPage (Arduino IDE friendly)

This repo now includes a **single Arduino sketch** for an Inland ESP32 Core board that:
- hosts a dark-mode web UI,
- performs **real Wi-Fi scanning** on the ESP32,
- shows simple risk tags (`OPEN`, `WEP`),
- keeps **Deauth as a harmless UI stub** (no packet injection).

## Use this sketch in Arduino IDE
Sketch path:
- `firmware/Inland_ESP32Core_WebUI/Inland_ESP32Core_WebUI.ino`

This sketch has HTML/CSS/JS embedded directly in `.ino`, so you can upload from Arduino IDE without SPIFFS/LittleFS filesystem upload tools.

## Arduino IDE setup (Inland ESP32 Core)
1. Install **Arduino IDE 2.x**.
2. Open **File -> Preferences** and add this URL to *Additional Boards Manager URLs*:
   - `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Open **Tools -> Board -> Boards Manager**, search `esp32`, install **esp32 by Espressif Systems**.
4. Connect your Inland ESP32 Core board via USB.
5. Select:
   - **Board**: `ESP32 Dev Module`
   - **Port**: your board COM/tty port
   - **Upload Speed**: `921600` (or `115200` if upload is unstable)
6. Open `Inland_ESP32Core_WebUI.ino` and click **Upload**.

If upload fails, hold **BOOT** while clicking Upload, release when "Connecting..." appears.

## First run
- Board starts its own AP:
  - SSID: `Inland-ESP32-Toolkit`
  - Password: `esp32darkmode`
- Connect your phone/laptop to that Wi-Fi.
- Open `http://192.168.4.1/`.
- Click **Scan Nearby APs** to run real scans.

## Optional STA mode
In the sketch:
- `STA_SSID`
- `STA_PASS`

Set these if you also want the board connected to your normal router while still hosting its own AP.
