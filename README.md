# Tusk: Revamping an old weaponised RFID reader tool

This repo contains the hardware and software upgrades made to the original [Tastic RFID Thief project](https://bishopfox.com/resources/rfid-hacking-intro-to-tastic-rfid-thief), which was created by Francis Brown at Bishop Fox.

The core functionality of the project remains the same: a tool that allows red teamers to capture RFID access card credentials from several feet away. Additionally, the same readers are supported:

- HID Maxiprox 5375 (125kHz HID ProxII cards)
- HID Indala ASR-620 (125kHz Indala cards)
- HID iCLASS R90 (13.56mHz iClass cards)

## Legacy Operating Mode vs Future Operating Mode

The project currently operates in "legacy mode", which is based on receiving card credentials from the `data0` and `data1` output pins on the Maxiprox reader. 

This will eventually change and data will be sampled directly from the reader's microcontroller (pin `PD7` in the image below). The card sampling code is currently in another repo: https://github.com/TeamWalrus/tusk-sampler , and needs a few kinks ironed out. If you are interested in helping out, please check it out.

![MC68HC705C8ACFNE-pinout](/images/MC68HC705C8ACFNE-pinout.jpg)

Link for the MC68HC705C8ACFNE datasheet: https://www.allaboutcircuits.com/electronic-components/datasheet/MC68HC705C8ACFNE--NXP-Semiconductors/

## Overview of the upgrades

| Component                  | Description                                                             |
|:-------------------------- |:----------------------------------------------------------------------- |
| Microcontroller Unit (MCU) | Replace the Arduino with an ESP32                                       |
| Batteries                  | Replace the AA with 27100's for the reader and 18650's for the MCU      |
| Wireless Connectivity      | The ESP32 has built-in WiFI capability                                  |
| Web App                    | Simple react web interface                                              |
| ~~Capability~~             | ~~Decode Gallagher 125kHz proximity cards (Cardax proprietary format)~~ |
| ~~Enclosure~~              | ~~3D printed enclosure for the PCB\*~~                                  |

\* One of the main issues with all previous build's I've seen, is the electronic components (microcontroller unit, buck converter and batteries) being housed inside the RFID reader. These components (specifically buck converters) create a lot of 'RF noise' - which interfere with the finely tuned RFID readers. This reduces the effective range of the reader, which can be the difference between capturing access card credentials or not. Housing these components externally, ensures the device operates at it's advertised range capability.

![tusk-pcb](/images/tusk-pcb-v0-3.jpg)

## Overview of the project structure

```
├── firmware                    # 
│   ├── interface               # react frontend web application
│   │   └── src
│   ├── data                    # build path of the react frontend
│   ├── src
│   │   └── main.cpp            # backend webserver + card handling code
│   ├── scripts
│   │   └── build_interface.py  # helper script to deploy react frontend to build path
├── hardware                    # gerber files, BOM, etc
```


## Build instructions: hardware

Gerber files, Bill of Materials (BOM), and Pick and Place files required to get the PCB manufactured can be found in the `/hardware` directory. This is a PCB that houses the batteries for the long range reader, ESP32 microcontroller unit, and sd-card reader. Note - some parts are not in the BOM as they were purchased directly from aliexpress (non-affiliate links below):

- ESP32-DevkitC-v4: https://www.aliexpress.com/item/4000090521976.html
- Mini SD card reader module: https://www.aliexpress.com/item/1865616455.html
- 21700 battery holder: https://www.aliexpress.com/item/1005003204083647.html

The tusk PCB uses 5 x 27100 li-ion batteries slots that will power the long range reader. That means the output voltage is 4.2V x 5 = 21V. This should be within the operating voltage for the long range readers listed above. However, I recommend checking this before hooking up your reader.

Once the PCB is assembled: 

- Connect the power from the PCB -> positive (+) and negative (-) from the block connector to the external reader
- Connect the data lines (`data0` and `data1`) from the block connector to the external reader

**Note**: the ESP32 footprint on the PCB is for an ESP32-DevKitC-V4. Older ESP32-DevKit-V1 will not fit.

![devkitv1-vs-devkitv4](/images/devkitv1-vs-devkitv4.jpg)

![aliexpress-esp32-devkitc-v4](/images/aliexpress-esp32-devkitc-v4.jpg)


## Build instructions: firmware

| Component                | Description                                                 |
|:------------------------ |:----------------------------------------------------------- |
| `/firmware/src/main.cpp` | backend webserver and access card credentials handling code |
| `/firmware/interface/`   | react frontend to display captured access card credentials  |

Change directory into `/firmware` and execute the following commands to flash the ESP32 firmware:

To build and upload the backend code: 

`pio run --target upload`

To build and upload the frontend react webapp: 

`pio run --target uploadfs`


### Tusk web interface

Clients connect to the ESP32 over WiFi and view captured card credentials via a react web application. The default values are as follows:

| Component     | Details                |
|:------------- |:---------------------- |
| WiFi SSID     | `Tusk`                 |
| WiFi Password | `changeme`             |
| URL           | `http://192.168.100.1` |

## Acknowledgments

It's important to acknowledge previous work done on this project:

- Fran Brown: original Tastic RFID Thief: `https://www.bishopfox.com/resources/tools/rfid-hacking/attack-tools/`
- Folks who updated the original Arduino code:
  - ninewires: `https://github.com/ninewires/Prox_Thief/blob/master/Prox_Thief.ino`
  - ShakataGaNai: `https://gist.github.com/ShakataGaNai/4319d3e82a858c9d00c1d80f20da81a3`
- Other interesting projects / upgraded versions of the RFID Thief:
  - Mike Kelly's Wiegotcha: `https://github.com/lixmk/Wiegotcha`
  - Corey Harding's ESP-RFID-Thief: `https://github.com/exploitagency/ESP-RFID-Thief`

Thanks to my mate @megabug for always helping out with my projects :)

Thanks to my employer [Aura Information Security](https://www.aurainfosec.com/) for providing time for me to work on this project.

![aura-logo](https://user-images.githubusercontent.com/27876907/188373880-8157648c-eb94-4054-81c8-7c61692b0367.png)

## Captured access card credentials format

Captured card credentials are written to the sd card in newline-delimited JSON (`cards.jsonl`). For example:

```
{"card_type":"hid","bit_length":26,"facility_code":123,"card_number":123123,"hex":"AAAAAAA","raw":"0010101010010011000101010"}
{"card_type":"gallagher","region_code":4,"bit_length":46,"facility_code":2222,"card_number":1111,"issue_level":3,"hex":"A38A8A4BA3A3A32C","raw":"101000110100010101100010101010010110101000110101000110101000110001011001"}
```