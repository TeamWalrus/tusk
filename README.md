# Tusk: Re-Weaponising long range RFID readers

This repo contains the hardware and software upgrades made to the original [Tastic RFID Thief project](https://bishopfox.com/resources/rfid-hacking-intro-to-tastic-rfid-thief), which was created by Francis Brown at Bishop Fox.

The core functionality of the project remains the same: a tool that allows red teamers to capture RFID access card credentials from several feet away. Additionally, the same readers are supported:

- HID Maxiprox 5375 (125kHz HID ProxII cards)
- HID Indala ASR-620 (125kHz Indala cards)
- HID iCLASS R90 (13.56mHz iClass cards)

## Overview of the upgrades

| Component                  | Description                                                             |
| -------------------------- | ----------------------------------------------------------------------- |
| Microcontroller Unit (MCU) | Replace the Arduino with an ESP32                                       |
| Batteries                  | Replace the AA with 27100's for the reader and 18650's for the MCU      |
| Wireless Connectivity      | The ESP32 has built-in WiFI capability                                  |
| Web App                    | Simple react web interface                                              |
| ~~Capability~~             | ~~Decode Gallagher 125kHz proximity cards (Cardax proprietary format)~~ |
| ~~Enclosure~~              | ~~3D printed enclosure for the PCB\*~~                                  |

\* One of the main issues with all previous build's I've seen, is the electronic components (microcontroller unit, buck converter and batteries) being housed inside the RFID reader. These components (specifically buck converters) create a lot of 'RF noise' - which interfere with the finely tuned RFID readers. This reduces the effective range of the reader, which can be the difference between capturing access card credentials or not. Housing these components externally, ensures the device operates at it's advertised range capability.

## Acknowledgments

Before going into the details of the updates, it's important to acknowledge previous work done on this project.

- Fran Brown: original Tastic RFID Thief (https://www.bishopfox.com/resources/tools/rfid-hacking/attack-tools/).
- Folks who updated the original Arduino code
  - ninewires: https://github.com/ninewires/Prox_Thief/blob/master/Prox_Thief.ino
  - ShakataGaNai: https://gist.github.com/ShakataGaNai/4319d3e82a858c9d00c1d80f20da81a3
- Other interesting projects / upgraded versions of the RFID Thief
  - Mike Kelly's Wiegotcha: https://github.com/lixmk/Wiegotcha
  - Corey Harding's ESP-RFID-Tool: https://github.com/rfidtool/ESP-RFID-Tool

Thanks to my employer [Aura Information Security](https://www.aurainfosec.com/) for providing time for me to work on this project.

![aura-logo](https://user-images.githubusercontent.com/27876907/188373880-8157648c-eb94-4054-81c8-7c61692b0367.png)

# Hardware

Gerber files, BOM, and Pick and Place files required to get the PCB manufactured. This is a PCB that houses the batteries for the long range reader, ESP32 microcontroller unit, and sd-card reader.

The tusk PCB has 5 x 27100 li-ion batteries slots that will power the long range reader. That means the output voltage is 4.2V x 5 = 21V. This should be within the operating voltage of the long range readers listed above. However, I recommend checking this before hooking up your reader.

# Firmware & Software

Clients connect to the ESP32 over WiFi and view captured card credentials via a react web application.

## Overview of the project structure

```
├── firmware
│   ├── interface               # react frontend web application
│   │   └── src
│   ├── data                    # build path of the react frontend
│   ├── src
│   │   └── main.cpp            # backend webserver
│   ├── scripts
│   │   └── build_interface.py  # helper script to build ahd deploy react webserver to /data/ folder
```

## Captured card data format

Captured card credentials are written to the sd card in newline-delimited JSON (`cards.jsonl`). For example:

```
{"bit_length":26,"facility_code":123,"card_number":123123,"hex":"AAAAAAA","raw":"0010101010010011000101010"}
{"bit_length":26,"facility_code":555,"card_number":678678,"hex":"FFFFFFF","raw":"0000000000000000000000000"}
```

# Enclosure

3D printed enclosure files
