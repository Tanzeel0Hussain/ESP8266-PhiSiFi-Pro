# 💀 ESP8266 PhiSiFi - HACKER EDITION 💀

```text
################################################
#                                              #
#       ESP8266 PHISIFI - V2.0 UPGRADE         #
#            Created by Tanzeel Hussain        #
#                                              #
################################################
```

This is a powerful **WiFi Security Testing Tool** for ESP8266. It performs **Deauthentication Attacks** and hosts an **Evil Twin Captive Portal** to capture WiFi credentials for educational and security research purposes.

---

## ⚡ Features
- **Hacker Vibe UI**: Modern dark-mode admin panel with cyan accents.
- **Phishing Portal**: Polished "Emergency Firmware Recovery" page for effective testing.
- **Auto Scanner**: Automatically scans for nearby WiFi networks.
- **Target Selection**: Pick your target from the web interface.
- **Real-time Logs**: Monitor status and captured keys in the admin console.

---

## 📥 Installation Guide (Windows)

### 1. Driver Installation
Your ESP8266 (NodeMCU/Wemos) needs a driver to connect to your computer.
- Locate the file `CH34x_Install_Windows_v3_4.zip` in this folder.
- Extract the ZIP file.
- Run `CH341SER.EXE` and click **INSTALL**.
- Once done, your board will be detected as a COM Port.

### 2. Arduino IDE Setup
1. Download and Install **Arduino IDE**.
2. Go to `File` -> `Preferences`.
3. In **Additional Board Manager URLs**, paste this link:
   `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
4. Go to `Tools` -> `Board` -> `Boards Manager`.
5. Search for `esp8266` and **Install** the latest version.

### 3. Board Settings (Important!)
To ensure the code works perfectly, use these settings under the `Tools` menu:
- **Board**: `Generic ESP8266 Module` (or `NodeMCU 1.0 (ESP-12E Module)`)
- **Flash Mode**: `DIO`
- **Flash Size**: `4MB (FS:2MB)` or `4MB (FS:1MB)`
- **Upload Speed**: `115200`
- **Port**: Select the COM port shown in Device Manager.

---

## 🚀 How to Use

1. **Upload**: Open `ESP8266_PhiSiFi.ino` in Arduino IDE and click **Upload**.
2. **Connect**: On your phone or laptop, connect to the WiFi named **"WiFi_Recovery_System"** (Default Password: `administrator`).
3. **Control Panel**: Open your browser and go to: `http://192.168.4.1/admin`
4. **Attack**:
   - **Step 1**: Scan and **SELECT** a target WiFi network.
   - **Step 2**: Click **DEAUTH_TARGET** to disconnect users from that network.
   - **Step 3**: Click **START_EVIL_TWIN** to launch the phishing page.
5. **Capture**: When the victim enters the password on the fake recovery page, it will appear in your **PhiSiFi Console**.

---

## ⚠️ Disclaimer
This tool is for **educational purposes only**. Do not use it on networks you do not own. Use responsibly.

---
**Maintained by**: [Tanzeel Hussain]
