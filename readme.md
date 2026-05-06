🌍 English | [中文](https://github.com/teatall/13.3inch_e-Paper_E-Frame/blob/main/readme.zh.md)

# ESP32-S3 13.3" Color E-Paper Digital Photo Frame (V1.1.1)

This is an open-source digital photo frame project based on the [ESP32-S3 and a 13.3-inch Full-Color E-Paper display](https://www.waveshare.net/wiki/13.3inch_e-Paper_HAT+_(E)_Manual). The project not only supports cyclic playback of images from a local SD card but also features a complete built-in Web management backend. It supports wireless uploading, online cropping, progress display, and automatic system time synchronization. This project was developed with the assistance of Google Gemini.

## 🚀 Core Features

* **Large Full-Color Display**: Supports the Waveshare 13.3-inch E Ink Spectra 6 (E6) full-color e-paper display with a resolution of 1600 × 1200.
* **Wireless Web Management**: No need to repeatedly plug and unplug the SD card. Upload, crop (default portrait mode), delete images, and configure system settings directly via a web browser.
* **High-Speed Streaming Uploads**: Optimized large-file receiving engine with a real-time frontend progress bar for a seamless upload experience.
* **Secure WiFi Provisioning**: Supports initialization or network switching via a `wifi.txt` file on the SD card. Upon successful configuration, credentials are automatically stored in NVS, and the plaintext file is immediately deleted ("burn after reading").
* **Automatic Time Sync**: The web interface automatically synchronizes the browser's local time to the ESP32 upon loading, ensuring SD card files have accurate modification timestamps.
* **Interactive Audio**: Built-in DAC driver (ES8311) provides a volume-optimized "tick" feedback sound for button interactions.
* **Low Power Management**: Optimized deep sleep current, featuring battery voltage detection and automatic low-battery sleep prompts.

## 🛠️ Hardware Requirements

1. **Controller**: ESP32-S3-ePaper-13.3E6 driver board (onboard 16MB PSRAM / 32MB Flash).
2. **Display**: Waveshare 13.3inch e-Paper (E).
3. **Storage**: Micro SD Card (FAT32 format recommended).
4. **Audio**: Onboard ES8311 DAC + NS4150B power amplifier (supports 8Ω speaker interface).
5. **Power Supply**: 3.7V Lithium battery or Type-C power.

## 📂 Directory Structure

| File Name | Description |
| :--- | :--- |
| `ESP32.ino` | Main program handling sleep/wake, button logic, and WiFi routing. |
| `webpage.h` | Core Web Backend: Contains HTML/JS, image processing, time sync, and upload engine. |
| `audio_driver.h` | ES8311 Audio Driver: Optimized volume register (0xDF) for clearer feedback sounds. |
| `epd_render.h` | Screen Rendering Engine: Handles BMP decoding, color dithering, and dual-IC regional transmission. |
| `usb_msc_driver.h` | USB MSC Driver: Supports direct SD card file management via USB. |
| `images.h` | Hex arrays for system status icons (low battery, no WiFi, etc.). |
| `fonts.h` / `font24.cpp` | System font library for displaying info like IP addresses on the screen. |

## 🔧 Development Environment Setup

1. **Arduino IDE**: Install the ESP32 board support package (version 2.0.x or above recommended).
2. **Board Selection**: `ESP32S3 Dev Module`.
3. **Key Compile Settings**:
   * **USB CDC On Boot**: Enabled *(See detailed tools section below)*
   * **Flash Size**: 32MB
   * **PSRAM**: OPI PSRAM
   * **Partition Scheme**: 16MB Flash (Sufficient to run the large-capacity Web Server)

## 📖 Usage Guide

### 1. Initial Network Provisioning (Out-of-the-box)
Create a `wifi.txt` file in the root directory of the SD card. The format should be as follows (two lines: SSID on the first, password on the second):
```text
YourWiFiSSID
YourWiFiPassword
```
Insert the card and power on the device. Double-click the **BOOT button** to enter Management Mode. The device will automatically read and save the WiFi credentials to the internal non-volatile storage (NVS), and then immediately delete the plaintext file from the SD card for security.

### 2. Button Interactions
* **Single Click**: Force refresh and switch to the next image.
* **Double Click**: Enter Management Mode (starts the Web Server and displays the current IP address on the screen).
* **Long Press (5 seconds)**: Clear the screen and power off (enters deep sleep state).

### 3. Web Management Operations
Enter the IP address displayed on the device into your browser to:
* **Upload Photos**: Click or drag-and-drop images. The system defaults to the **Portrait (3:4)** cropping aspect ratio. Click "Save" and watch the green progress bar at the top as the streaming upload completes.
* **Photo Gallery**: View existing files on the SD card. Supports reverse chronological sorting, thumbnail previews, and one-click secure deletion.
* **System Settings**: Real-time monitoring of remaining battery, WiFi signal strength, and storage space. Adjust the automatic image switching interval (multiple options ranging from 10 minutes to 24 hours).

## ⚠️ Precautions

* **Invisible Character Errors**: If you copy HTML/JS code from external sources into the `.h` files, be sure to use a plain text editor to check for and remove any `NBSP` (non-breaking spaces) at the beginning of lines. Otherwise, the Arduino compiler will throw invisible character errors.
* **Screen Protection Mechanism**: Keeping a color e-paper display powered under high voltage for extended periods can damage the film. This program forces the screen and driver board into sleep mode immediately after each refresh. It is recommended to perform a full screen refresh at least once every 24 hours to prevent ghosting and aging.

## ⚙️ Arduino IDE Tools Core Settings

To ensure the firmware compiles and runs correctly, please configure the following parameters exactly as shown in the Arduino IDE `Tools` menu:

* **Board**: `ESP32S3 Dev Module`
* **USB CDC On Boot**: `Disabled` 
    > 📝 **Note**: This driver board features an advanced dual-chip design with a CH334 USB HUB and a CH343 UART chip. The physical Type-C port handles both hardware UART and native USB communication simultaneously. Keeping this `Disabled` ensures that serial debug logs are stably output via the hardware chip, while allowing the native USB interface to focus solely on the USB Mass Storage (U-Disk) mode.
* **Flash Size**: `32MB (256Mb)` 
* **Partition Scheme**: `Huge APP (3MB No OTA/1MB SPIFFS)`
    > 📝 **Note**: Because the firmware embeds the complete Web management backend code and high-resolution status icon libraries, the compiled binary is quite large. Standard default partitions will result in errors. The 3MB space provided by the Huge APP scheme is perfectly sufficient to accommodate this program.
* **PSRAM**: `OPI PSRAM`
    > ⚠️ **CRITICAL**: The 13.3-inch full-color e-paper display has a high resolution of 1600x1200. BMP image decoding and color dithering rendering require massive amounts of RAM. If you forget to enable PSRAM, the device will crash and reboot due to an Out of Memory (OOM) error when processing images.

## 📄 License

MIT License

---
**Special Thanks**: Waveshare for providing the hardware and low-level screen driver support.
