# ESP8266 Wireless Security Lab

A compact **defensive Wi-Fi visibility project** for ESP8266 development boards. The current firmware performs passive nearby-network scanning and serves a local dashboard showing SSID, BSSID, channel, RSSI, and advertised security mode.

> This project is intended for your own lab, classroom, and explicitly authorized wireless-security observation. The current main-branch firmware does **not** disconnect clients, impersonate access points, or collect Wi-Fi credentials.

## Highlights

- Passive 2.4 GHz Wi-Fi scanning
- SSID and BSSID inventory
- Channel and RSSI visibility
- Advertised security-mode identification
- Open-network configuration warning
- Local responsive dashboard hosted by the ESP8266
- PlatformIO build configuration
- Browser-based firmware installer using ESP Web Tools
- GitHub Actions firmware compilation and Pages packaging

## Quick Start

### Browser installer

Once GitHub Pages is enabled for the included Actions workflow, open:

`https://tanzeel0hussain.github.io/ESP8266-PhiSiFi-Pro/`

Connect a supported ESP8266 board with a data-capable USB cable, click the install button, and approve the serial device in the browser. Chrome or Edge on desktop is recommended for Web Serial support.

### Local build

Requirements:

- Python 3
- PlatformIO CLI or PlatformIO IDE
- ESP8266 development board such as NodeMCU v2

```bash
pio run -e nodemcuv2
```

The compiled firmware is generated under:

```text
.pio/build/nodemcuv2/firmware.bin
```

To upload from PlatformIO:

```bash
pio run -e nodemcuv2 -t upload
```

## After Installation

1. Reboot the ESP8266.
2. Connect to Wi-Fi: `ESP8266-Security-Lab`
3. Password: `securitylab`
4. Open `http://192.168.4.1`
5. Use **Scan now** to refresh nearby-network metadata.

## Current Data Displayed

| Field | Purpose |
| --- | --- |
| SSID | Advertised network name |
| BSSID | Access-point MAC address |
| Channel | 2.4 GHz operating channel |
| RSSI | Observed signal strength |
| Security | Advertised encryption/security mode |

No Wi-Fi passwords are requested or stored by the current firmware.

## Repository Structure

```text
.
├── .github/workflows/
│   └── build-and-pages.yml
├── docs/
│   ├── index.html
│   └── manifest.json
├── firmware/
│   └── src/
│       └── main.cpp
├── platformio.ini
├── LICENSE
└── README.md
```

## Web Installer Pipeline

Every push to `main` triggers GitHub Actions to:

1. Compile the ESP8266 firmware with PlatformIO.
2. Produce `firmware.bin`.
3. Copy the binary into the generated installer site.
4. Upload the firmware as a workflow artifact.
5. Deploy the installer site through GitHub Pages when Pages is configured to use GitHub Actions.

The ESP Web Tools manifest is located at `docs/manifest.json`.

## Design Direction

This repository has been repositioned as a **defensive wireless-security lab**. The focus is network visibility, configuration awareness, clean embedded engineering, repeatable builds, and simple firmware delivery.

Planned improvements include:

- Duplicate/rogue SSID observations
- Channel-distribution summary
- Security posture hints
- Optional event logging
- Additional tested ESP8266 boards
- Screenshots and hardware documentation

## Driver Note

USB-UART drivers depend on the board revision and USB bridge chip. Use the official vendor driver for your specific CH340/CH341, CP210x, or other USB-UART adapter instead of bundled third-party driver archives.

## License

MIT License. See [LICENSE](LICENSE).

## Maintainer

[Tanzeel Hussain](https://github.com/Tanzeel0Hussain)
