# Tusk

This repo contains the hardware and software related to upgrades being made to the Tastic RFID Thief project.

## Hardware

PCB that houses the batteries for the long range reader, ESP32 microcontroller, and sd-card reader.

## WiFi

### Firmware & Software

#### Overview of the project structure:

```
├── wifi
│   ├── interface               # react frontend web application
│   │   └── src
│   ├── data                    # build path of the react frontend
│   ├── src                     # backend webserver
│   │   └── main.cpp
│   ├── scripts
│   │   └── build_interface.py  # helper script to build ahd deploy react webserver to /data/ folder
```

#### Sample cards.jsonl file for testing (write to sd card)

Dummy data in `cards.jsonl` file:

```
{"bit_length":462,"facility_code":123,"card_number":123123,"hex":"AAAAAAA","raw":"0010101010010011000101010"}
{"bit_length":418,"facility_code":555,"card_number":678678,"hex":"FFFFFFF","raw":"0000000000000000000000000"}
```

## Bluetooth

### Firmware

ESP32 firmware that handles card data coming from the reader and sends it over Bluetooth.

Use PlatformIO to build & upload firmware

#### TODO:

- Add functions to write card data to sd-card as backup

### Software

Python client to connect to tusk via bluetooth
