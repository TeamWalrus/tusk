// vim: ts=2 sw=2 et

#include "ArduinoJson.h"
#include "AsyncJson.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>

// Uncomment for debugging / verbose messages
// #define VERBOSE

// WiFi config
// Variables to save values from HTML form
String ssid;
String password;
String channel;
String hidessid;

// File paths on SD card to save wifi values permanently
const char *ssidPath = "/ssid.txt";
const char *passwordPath = "/password.txt";
const char *channelPath = "/channel.txt";
const char *hidessidPath = "/hidessid.txt";
const char *jsoncarddataPath = "/cards.jsonl";

IPAddress local_ip(192, 168, 100, 1);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);

// Define CS pin for the SD card module
const int sd_cs = 5;

// Webserver config /
AsyncWebServer server(80);

// Read File from SD Card
String readSDFile(const char *path) {
  File file = SD.open(path);
  if (!file || file.isDirectory()) {
#ifdef VERBOSE
    Serial.println("[-] Failed to open file for reading");
#endif
    return String();
  }

  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}

// Write file to SD Card
void writeSDFile(const char *path, const char *message) {
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
#ifdef VERBOSE
    Serial.println("[-] Failed to open file for writing");
#endif
    return;
  }
  if (file.print(message)) {
#ifdef VERBOSE
    Serial.println("[+] File written");
#endif
  } else {
#ifdef VERBOSE
    Serial.println("[-] File write failed");
#endif
  }
}

/* #####----- Card Reader Config -----##### */
// Card Reader variables

// max number of bits
#define MAX_BITS 100
// time to wait for another weigand pulse
#define WEIGAND_WAIT_TIME 3000

// stores all of the data bits
unsigned char databits[MAX_BITS];
volatile unsigned int bitCount = 0;
// stores the last written card's data bits
unsigned char lastWrittenDatabits[MAX_BITS];
unsigned int lastWrittenBitCount = 0;

// goes low when data is currently being captured
unsigned char flagDone;

// countdown until we assume there are no more bits
unsigned int weigand_counter;

// decoded facility code
volatile unsigned long facilityCode = 0;
// decoded card code
volatile unsigned long cardCode = 0;

// Breaking up card value into 2 chunks to create 10 char HEX value
volatile unsigned long bitHolder1 = 0;
volatile unsigned long bitHolder2 = 0;
volatile unsigned long cardChunk1 = 0;
volatile unsigned long cardChunk2 = 0;

// Define reader input pins
// card reader DATA0
#define DATA0 32
// card reader DATA1
#define DATA1 33

/* #####----- Process interupts -----##### */
// interrupt that happens when INT0 goes low (0 bit)
void ISR_INT0() {
  bitCount++;
  flagDone = 0;

  if (bitCount < 23) {
    bitHolder1 = bitHolder1 << 1;
  } else {
    bitHolder2 = bitHolder2 << 1;
  }
  weigand_counter = WEIGAND_WAIT_TIME;
}

// interrupt that happens when INT1 goes low (1 bit)
void ISR_INT1() {
  if (bitCount < MAX_BITS) {
    databits[bitCount] = 1;
    bitCount++;
  }
  flagDone = 0;

  if (bitCount < 23) {
    bitHolder1 = bitHolder1 << 1;
    bitHolder1 |= 1;
  } else {
    bitHolder2 = bitHolder2 << 1;
    bitHolder2 |= 1;
  }

  weigand_counter = WEIGAND_WAIT_TIME;
}

/* #####----- Print bits function -----##### */
void printBits() {
  if (bitCount >= 26) { // ignore data caused by noise
    Serial.print("bit length: ");
    Serial.println(bitCount);
    Serial.println("facility code: ");
    Serial.println(facilityCode);
    Serial.println("card number: ");
    Serial.println(cardCode);
    Serial.printf("Hex: %lx%06lx\n", cardChunk1, cardChunk2);
  }
}

/* #####----- Process card format function -----##### */
void getCardNumAndSiteCode() {
  unsigned char i;

  // bits to be decoded differently depending on card format length
  // see http://www.pagemac.com/projects/rfid/hid_data_formats for more info
  // also specifically: www.brivo.com/app/static_data/js/calculate.js
  switch (bitCount) {

  ///////////////////////////////////////
  // standard 26 bit format
  // facility code = bits 2 to 9
  case 26:
    for (i = 1; i < 9; i++) {
      facilityCode <<= 1;
      facilityCode |= databits[i];
    }

    // card code = bits 10 to 23
    for (i = 9; i < 25; i++) {
      cardCode <<= 1;
      cardCode |= databits[i];
    }
    break;

  ///////////////////////////////////////
  // 33 bit HID Generic
  case 33:
    for (i = 1; i < 8; i++) {
      facilityCode <<= 1;
      facilityCode |= databits[i];
    }

    // card code
    for (i = 8; i < 32; i++) {
      cardCode <<= 1;
      cardCode |= databits[i];
    }
    break;

  ///////////////////////////////////////
  // 34 bit HID Generic
  case 34:
    for (i = 1; i < 17; i++) {
      facilityCode <<= 1;
      facilityCode |= databits[i];
    }

    // card code
    for (i = 17; i < 33; i++) {
      cardCode <<= 1;
      cardCode |= databits[i];
    }
    break;

  ///////////////////////////////////////
  // 35 bit HID Corporate 1000 format
  // facility code = bits 2 to 14
  case 35:
    for (i = 2; i < 14; i++) {
      facilityCode <<= 1;
      facilityCode |= databits[i];
    }

    // card code = bits 15 to 34
    for (i = 14; i < 34; i++) {
      cardCode <<= 1;
      cardCode |= databits[i];
    }
    break;
  }
  return;
}

/* #####----- Card value processing -----##### */
// Function to append the card value (bitHolder1 and bitHolder2) to the
// necessary array then translate that to the two chunks for the card value that
// will be output
void getCardValues() {
  switch (bitCount) {
    // Example of full card value
    // |>   preamble   <| |>   Actual card value   <|
    // 000000100000000001 11 111000100000100100111000
    // |> write to chunk1 <| |>  write to chunk2   <|

  case 26:
    for (int i = 19; i >= 0; i--) {
      if (i == 13 || i == 2) {
        bitWrite(cardChunk1, i, 1);
      } else if (i > 2) {
        bitWrite(cardChunk1, i, 0);
      } else {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 20));
      }
      if (i < 20) {
        bitWrite(cardChunk2, i + 4, bitRead(bitHolder1, i));
      }
      if (i < 4) {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 27:
    for (int i = 19; i >= 0; i--) {
      if (i == 13 || i == 3) {
        bitWrite(cardChunk1, i, 1);
      } else if (i > 3) {
        bitWrite(cardChunk1, i, 0);
      } else {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 19));
      }
      if (i < 19) {
        bitWrite(cardChunk2, i + 5, bitRead(bitHolder1, i));
      }
      if (i < 5) {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 28:
    for (int i = 19; i >= 0; i--) {
      if (i == 13 || i == 4) {
        bitWrite(cardChunk1, i, 1);
      } else if (i > 4) {
        bitWrite(cardChunk1, i, 0);
      } else {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 18));
      }
      if (i < 18) {
        bitWrite(cardChunk2, i + 6, bitRead(bitHolder1, i));
      }
      if (i < 6) {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 29:
    for (int i = 19; i >= 0; i--) {
      if (i == 13 || i == 5) {
        bitWrite(cardChunk1, i, 1);
      } else if (i > 5) {
        bitWrite(cardChunk1, i, 0);
      } else {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 17));
      }
      if (i < 17) {
        bitWrite(cardChunk2, i + 7, bitRead(bitHolder1, i));
      }
      if (i < 7) {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 30:
    for (int i = 19; i >= 0; i--) {
      if (i == 13 || i == 6) {
        bitWrite(cardChunk1, i, 1);
      } else if (i > 6) {
        bitWrite(cardChunk1, i, 0);
      } else {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 16));
      }
      if (i < 16) {
        bitWrite(cardChunk2, i + 8, bitRead(bitHolder1, i));
      }
      if (i < 8) {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 31:
    for (int i = 19; i >= 0; i--) {
      if (i == 13 || i == 7) {
        bitWrite(cardChunk1, i, 1);
      } else if (i > 7) {
        bitWrite(cardChunk1, i, 0);
      } else {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 15));
      }
      if (i < 15) {
        bitWrite(cardChunk2, i + 9, bitRead(bitHolder1, i));
      }
      if (i < 9) {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 32:
    for (int i = 19; i >= 0; i--) {
      if (i == 13 || i == 8) {
        bitWrite(cardChunk1, i, 1);
      } else if (i > 8) {
        bitWrite(cardChunk1, i, 0);
      } else {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 14));
      }
      if (i < 14) {
        bitWrite(cardChunk2, i + 10, bitRead(bitHolder1, i));
      }
      if (i < 10) {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 33:
    for (int i = 19; i >= 0; i--) {
      if (i == 13 || i == 9) {
        bitWrite(cardChunk1, i, 1);
      } else if (i > 9) {
        bitWrite(cardChunk1, i, 0);
      } else {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 13));
      }
      if (i < 13) {
        bitWrite(cardChunk2, i + 11, bitRead(bitHolder1, i));
      }
      if (i < 11) {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 34:
    for (int i = 19; i >= 0; i--) {
      if (i == 13 || i == 10) {
        bitWrite(cardChunk1, i, 1);
      } else if (i > 10) {
        bitWrite(cardChunk1, i, 0);
      } else {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 12));
      }
      if (i < 12) {
        bitWrite(cardChunk2, i + 12, bitRead(bitHolder1, i));
      }
      if (i < 12) {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 35:
    for (int i = 19; i >= 0; i--) {
      if (i == 13 || i == 11) {
        bitWrite(cardChunk1, i, 1);
      } else if (i > 11) {
        bitWrite(cardChunk1, i, 0);
      } else {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 11));
      }
      if (i < 11) {
        bitWrite(cardChunk2, i + 13, bitRead(bitHolder1, i));
      }
      if (i < 13) {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 36:
    for (int i = 19; i >= 0; i--) {
      if (i == 13 || i == 12) {
        bitWrite(cardChunk1, i, 1);
      } else if (i > 12) {
        bitWrite(cardChunk1, i, 0);
      } else {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 10));
      }
      if (i < 10) {
        bitWrite(cardChunk2, i + 14, bitRead(bitHolder1, i));
      }
      if (i < 14) {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 37:
    for (int i = 19; i >= 0; i--) {
      if (i == 13) {
        bitWrite(cardChunk1, i, 0);
      } else {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 9));
      }
      if (i < 9) {
        bitWrite(cardChunk2, i + 15, bitRead(bitHolder1, i));
      }
      if (i < 15) {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;
  }
  return;
}

String prefixPad(const String &in, const char c, const size_t len) {
  String out = in;
  while (out.length() < len) {
    out = c + out;
  }
  return out;
}

/* #####----- Write to SD card -----##### */
void writeSD() {
  File SDFile = SD.open("/cards.jsonl", FILE_APPEND);
  if (SDFile) {
    if (bitCount >= 26) { // ignore data caused by noise
      DynamicJsonDocument doc(512);
      doc["bit_length"] = bitCount;
      doc["facility_code"] = facilityCode;
      doc["card_number"] = cardCode;
      String hex =
          String(cardChunk1, HEX) + prefixPad(String(cardChunk2, HEX), '0', 6);
      doc["hex"] = hex;
      String raw;
      for (int i = 19; i >= 0; i--) {
        raw = (bitRead(cardChunk1, i));
      }
      for (int i = 23; i >= 0; i--) {
        raw += (bitRead(cardChunk2, i));
      }
      doc["raw"] = raw;
#ifdef VERBOSE
      Serial.println("[+] New Card Read:");
      serializeJsonPretty(doc, Serial);
#endif
      if (serializeJson(doc, SDFile) == 0) {
#ifdef VERBOSE
        Serial.println("[-] SD Card: Failed to write json card data to file");
#endif
      }
      SDFile.print("\n");
      SDFile.close();
#ifdef VERBOSE
      Serial.println("[+] SD Card: Data Written to SD Card");
#endif
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize SD card
  pinMode(sd_cs, OUTPUT);

  delay(3000);
  if (!SD.begin(sd_cs)) {
#ifdef VERBOSE
    Serial.println("[-] SD Card: An error occurred while initializing");
    Serial.println("[-] SD Card: Fix & Power Cycle");
#endif
  } else {
#ifdef VERBOSE
    Serial.println("[+] SD Card: Initialized successfully");
#endif
  }

  // Initialize LittleFS
  delay(3000);
  if (!LittleFS.begin()) {
#ifdef VERBOSE
    Serial.println("[-] LittleFS: An error occurred while mounting");
#endif
  } else {
#ifdef VERBOSE
    Serial.println("[+] LittleFS: Mounted successfully");
#endif
  }

  // Check if ssid.txt file exists on SD card
  delay(3000);
  if (!SD.exists(ssidPath)) {
#ifdef VERBOSE
    Serial.println("[-] WiFi Config: ssid.txt file not found");
#endif
    // If file doesn't exist, create wifi config files
    writeSDFile(ssidPath, "Tusk");
    writeSDFile(passwordPath, "12345678");
    writeSDFile(channelPath, "1");
    writeSDFile(hidessidPath, "0");
#ifdef VERBOSE
    Serial.println("[+] WiFi Config: wifi config files created");
    Serial.println("[*] WiFi Config: Rebooting...");
#endif
    delay(3000);
    ESP.restart();
  } else {
#ifdef VERBOSE
    Serial.println("[+] WiFi Config: Found ssid.txt and password.txt");
#endif
  }

  ssid = readSDFile(ssidPath);
  password = readSDFile(passwordPath);
  channel = readSDFile(channelPath);
  hidessid = readSDFile(hidessidPath);

  // Initialize wifi
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid.c_str(), password.c_str(), channel.toInt(),
              hidessid.toInt());

#ifdef VERBOSE
  Serial.println("\n[*] WiFi: Creating ESP32 Access Point");
  Serial.print("[+] WiFi: Access Point created with IP Gateway: ");
  Serial.println(local_ip);
#endif

  // set tx/rx pins for card reader
  pinMode(DATA0, INPUT); // DATA0 (INT0)
  pinMode(DATA1, INPUT); // DATA1 (INT1)

  // binds the ISR functions to the falling edge of INT0 and INT1
  attachInterrupt(DATA0, ISR_INT0, FALLING);
  attachInterrupt(DATA1, ISR_INT1, FALLING);
  weigand_counter = WEIGAND_WAIT_TIME;

  // Check for cards.jsonl on SD card
  delay(3000);
  if (!SD.exists("/cards.jsonl")) {
#ifdef VERBOSE
    Serial.println("[-] SD Card: File cards.jsonl not found");
    Serial.println(
        "[*] SD Card: Created cards.jsonl and performing software reset");
#endif
    // If file doesn't exist, create it
    writeSDFile(jsoncarddataPath, "");
#ifdef VERBOSE
    Serial.println("[+] SD Card: File cards.jsonl created");
    Serial.println("[*] SD Card: Rebooting...");
#endif
    delay(3000);
    ESP.restart();
  } else {
#ifdef VERBOSE
    Serial.println("[+] SD Card: Found cards.jsonl");
#endif
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef VERBOSE
    Serial.println("[*] Webserver: Serving file:  /index.html.gz");
#endif
    AsyncWebServerResponse *response =
        request->beginResponse(LittleFS, "/index.html.gz", "text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef VERBOSE
    Serial.println("[*] Webserver: Serving file:  /index.html.gz");
#endif
    AsyncWebServerResponse *response =
        request->beginResponse(LittleFS, "/index.html.gz", "text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/assets/*", HTTP_GET, [](AsyncWebServerRequest *request) {
    String url = request->url();
    String urlgz = url + ".gz";
#ifdef VERBOSE
    Serial.println("[*] Webserver: Requested url file: " + url);
    Serial.println("[*] Webserver: Serving gzipped file: " + url);
#endif
    String contentType = url.substring(url.length() - 3);
    if (contentType == "tml" || contentType == "htm")
      contentType = "text/html";
    else if (contentType == "css")
      contentType = "text/css";
    else if (contentType == ".js")
      contentType = "text/javascript";
    else if (contentType == "son")
      contentType = "application/json";
    else if (contentType == "jpg" || contentType == "peg")
      contentType = "image/jpeg";
    else if (contentType == "png")
      contentType = "image/png";
    else if (contentType == "svg")
      contentType = "image/svg+xml";
    else
      contentType = "text/plain";

    AsyncWebServerResponse *response =
        request->beginResponse(LittleFS, urlgz, contentType);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/api/littlefsinfo", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response =
        request->beginResponseStream("application/json");
    DynamicJsonDocument json(512);
    json["totalBytes"] = LittleFS.totalBytes();
    json["usedBytes"] = LittleFS.usedBytes();
    serializeJson(json, *response);
    request->send(response);
  });

  server.on("/api/sdcardinfo", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response =
        request->beginResponseStream("application/json");
    DynamicJsonDocument json(512);
    json["totalBytes"] = SD.totalBytes();
    json["usedBytes"] = SD.usedBytes();
    serializeJson(json, *response);
    request->send(response);
  });

  server.on("/api/carddata", HTTP_GET, [](AsyncWebServerRequest *request) {
    File SDFile = SD.open("/cards.jsonl", FILE_READ);
    String cardData;
    if (SDFile) {
      while (SDFile.available()) {
        cardData = SDFile.readString();
      }
      SDFile.close();
    } else {
#ifdef VERBOSE
      Serial.println("[-] SD Card: error opening json data");
#endif
    }
    // Should we rather be streaming the json data?
    // AsyncResponseStream *response =
    // request->beginResponseStream("application/json");
    AsyncWebServerResponse *response =
        request->beginResponse(200, "application/x-ndjson", cardData);
    request->send(response);
  });

  server.on("/api/delete/carddata", HTTP_GET,
            [](AsyncWebServerRequest *request) {
              writeSDFile(jsoncarddataPath, "");
              request->send(200, "text/plain", "All card data deleted!");

              lastWrittenBitCount = 0;
              for (unsigned char i = 0; i < MAX_BITS; i++) {
                lastWrittenDatabits[i] = 0;
              }
            });

  server.on(
      "/wificonfig/update", HTTP_POST, [](AsyncWebServerRequest *request) {
        int params = request->params();
        for (int i = 0; i < params; i++) {
          AsyncWebParameter *p = request->getParam(i);
          if (p->isPost()) {
            if (p->name() == "ssid") {
              ssid = p->value().c_str();
              writeSDFile(ssidPath, ssid.c_str());
            }
            if (p->name() == "password") {
              password = p->value().c_str();
              writeSDFile(passwordPath, password.c_str());
            }
            if (p->name() == "channel") {
              channel = p->value().c_str();
              writeSDFile(channelPath, channel.c_str());
            }
            if (p->name() == "hidessid") {
              hidessid = p->value().c_str();
              if (hidessid == "on") {
                writeSDFile(hidessidPath, "1");
              } else {
                writeSDFile(hidessidPath, "0");
              }
            }
#ifdef VERBOSE
            Serial.printf("POST[%s]: %s\n", p->name().c_str(),
                          p->value().c_str());
#endif
          }
        }
        request->send(200, "text/plain",
                      "WiFi configuration updated... device will now restart!");
        delay(3000);
        ESP.restart();
      });

  server.onNotFound([](AsyncWebServerRequest *request) { request->send(404); });

  server.begin();
#ifdef VERBOSE
  Serial.println("[+] Webserver: Started");
#endif
  Serial.println("[*] Tusk is running");

  for (unsigned char i = 0; i < MAX_BITS; i++) {
    lastWrittenDatabits[i] = 0;
  }
}

void loop() {
  if (!flagDone) {
    if (--weigand_counter == 0)
      flagDone = 1;
  }

  // if there are bits and the weigand counter went out
  if (bitCount > 0 && flagDone) {
    unsigned char i;

    // check if the newly read card's bits are the same as the previously
    // written card's bits
    bool same = true;
    if (bitCount == lastWrittenBitCount) {
      for (i = 0; i < bitCount; i++) {
        if (databits[i] != lastWrittenDatabits[i]) {
          same = false;
          break;
        }
      }
    } else {
      same = false;
    }

    if (!same) {
      getCardValues();
      getCardNumAndSiteCode();

#ifdef VERBOSE
      printBits();
#endif

      writeSD();

      lastWrittenBitCount = bitCount;
      for (i = 0; i < bitCount; i++) {
        lastWrittenDatabits[i] = databits[i];
      }
    }

    // cleanup and get ready for the next card
    bitCount = 0;
    facilityCode = 0;
    cardCode = 0;
    bitHolder1 = 0;
    bitHolder2 = 0;
    cardChunk1 = 0;
    cardChunk2 = 0;

    for (i = 0; i < MAX_BITS; i++) {
      databits[i] = 0;
    }
  }
}
