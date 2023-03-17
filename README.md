# Tusk

This repo contains the hardware and software related to upgrades being made to the Tastic RFID Thief project. 


## Hardware

PCB that houses the batteries for the long range reader, ESP32 microcontroller, and sd-card reader.

## Firmware

ESP32 firmware that handles card data coming from the reader and sends it over Bluetooth.

Use PlatformIO to build & flash firmware

//TODO: Add functions to write card data to sd-card as well


## Software

Currently a python client exists to connect to the ESP32 via Bluetooth and receive data

//TODO: Create a mobile client?
