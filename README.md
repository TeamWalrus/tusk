# Tusk

This repo contains the hardware and software related to upgrades being made to the Tastic RFID Thief project. 


## Hardware

PCB that houses the batteries for the long range reader, ESP32 microcontroller, and sd-card reader.

## WiFi

### Firmware & Software

- `/interface/` is the react frontend
- `/data/` is build path of the react frontend
- `/src/main.cpp`is the backend (webserver)
- `/scripts/build_interface.py`is a helper script to build / deploy react webserver to `/data/` folder -> gzips the files 


//TODO:

- add search / filter feature for card data
- use websockets to update card data in less janky way (rather than read entire file everytime?)
- Add code to handle card data coming from reader
- Switch from writing card data to littlefs in frontend website partition to external sd card
- Remove dummy data in `main.cpp`


## Bluetooth

### Firmware

ESP32 firmware that handles card data coming from the reader and sends it over Bluetooth.

Use PlatformIO to build & flash firmware

//TODO: Add functions to write card data to sd-card as well


### Software

Basic python client to connect to tusk via bluetooth 
