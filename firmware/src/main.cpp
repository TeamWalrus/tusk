// vim: ts=2 sw=2 et

#include "ArduinoJson.h"
#include "AsyncJson.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>

// wifi config
// variables to save values from HTML form
String ssid;
String password;
String channel;
String hidessid;

// file paths on SD card to save wifi values permanently
const char *ssidPath = "/ssid.txt";
const char *passwordPath = "/password.txt";
const char *channelPath = "/channel.txt";
const char *hidessidPath = "/hidessid.txt";
const char *jsoncarddataPath = "/cards.jsonl";

IPAddress local_ip(192, 168, 100, 1);
IPAddress gateway(192, 168, 100, 1);
IPAddress subnet(255, 255, 255, 0);

// define CS pin for the SD card module
const int sd_cs = 5;

// general device settings
bool isCapturing = true;
String version = "0.1";

// read file from SD Card
String readSDFileLF(const char *path) {
  File file = SD.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("[-] Failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
  file.close();
}

// write file to SD Card
void writeSDFile(const char *path, const char *message) {
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("[-] Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("[+] File written");
  } else {
    Serial.println("[-] File write failed");
  }
  file.close();
}

// card reader config and variables

// max number of bits
#define MAX_BITS 100
// time to wait for another weigand pulse
#define WEIGAND_WAIT_TIME 3000

// stores all of the data bits
volatile unsigned char databits[MAX_BITS];
volatile unsigned int bitCount = 0;
// stores the last written card's data bits
unsigned char lastWrittenDatabits[MAX_BITS];
unsigned int lastWrittenBitCount = 0;

// goes low when data is currently being captured
volatile unsigned char flagDone;

// countdown until we assume there are no more bits
volatile unsigned int weigandCounter;

// card type
String cardType = "";

// decoded facility code
unsigned long facilityCode = 0;
// decoded card code
unsigned long cardCode = 0;

// breaking up card value into 2 chunks to create 10 char HEX value
volatile unsigned long bitHolder1 = 0;
volatile unsigned long bitHolder2 = 0;
unsigned long cardChunk1 = 0;
unsigned long cardChunk2 = 0;

// Define reader input pins
// card reader DATA0
#define DATA0 32
// card reader DATA1
#define DATA1 33

// process interupts
// interrupt that happens when INT0 goes low (0 bit)
void ISR_INT0() {
  bitCount++;
  flagDone = 0;

  if (bitCount < 23) {
    bitHolder1 = bitHolder1 << 1;
  } else {
    bitHolder2 = bitHolder2 << 1;
  }
  weigandCounter = WEIGAND_WAIT_TIME;
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

  weigandCounter = WEIGAND_WAIT_TIME;
}

// print bits to serial (for debugging only)
void printBits() {
  // ignore data caused by noise
  if (bitCount >= 26) {
    Serial.print("bit length: ");
    Serial.println(bitCount);
    Serial.println("facility code: ");
    Serial.println(facilityCode);
    Serial.println("card number: ");
    Serial.println(cardCode);
    Serial.printf("Hex: %lx%06lx\n", cardChunk1, cardChunk2);
  }
}

// process hid cards
unsigned long decodeHIDFacilityCode(unsigned int start, unsigned int end) {
  unsigned long HIDFacilityCode = 0;
  unsigned int i;
  for (i = start; i < end; i++) {
    HIDFacilityCode = (HIDFacilityCode << 1) | databits[i];
  }
  return HIDFacilityCode;
}

unsigned long decodeHIDCardCode(unsigned int start, unsigned int end) {
  unsigned long HIDCardCode = 0;
  unsigned int i;
  for (i = start; i < end; i++) {
    HIDCardCode = (HIDCardCode << 1) | databits[i];
  }
  return HIDCardCode;
}

void getCardNumAndSiteCode() {
  // bits to be decoded differently depending on card format length
  // see http://www.pagemac.com/projects/rfid/hid_data_formats for more info
  // also specifically: www.brivo.com/app/static_data/js/calculate.js
  switch (bitCount) {
    // standard 26 bit format
  case 26:
    cardType = "hid";
    facilityCode = decodeHIDFacilityCode(1, 9);
    cardCode = decodeHIDCardCode(9, 25);
    break;

    // 33 bit HID Generic
  case 33:
    cardType = "hid";
    facilityCode = decodeHIDFacilityCode(1, 8);
    cardCode = decodeHIDCardCode(8, 32);
    break;

    // 34 bit HID Generic
  case 34:
    cardType = "hid";
    facilityCode = decodeHIDFacilityCode(1, 17);
    cardCode = decodeHIDCardCode(17, 33);
    break;

    // 35 bit HID Corporate 1000 format
  case 35:
    cardType = "hid";
    facilityCode = decodeHIDFacilityCode(2, 14);
    cardCode = decodeHIDCardCode(14, 34);
    break;
  }
  return;
}

// card value processing functions
// function to append the card value (bitHolder1 and bitHolder2) to the
// necessary array then translate that to the two chunks for the card value that
// will be output
void setCardChunkBits(unsigned int cardChunk1Offset,
                      unsigned int bitHolderOffset,
                      unsigned int cardChunk2Offset) {
  for (int i = 19; i >= 0; i--) {
    if (i == 13 || i == cardChunk1Offset) {
      bitWrite(cardChunk1, i, 1);
    } else if (i > cardChunk1Offset) {
      bitWrite(cardChunk1, i, 0);
    } else {
      bitWrite(cardChunk1, i, bitRead(bitHolder1, i + bitHolderOffset));
    }
    if (i < bitHolderOffset) {
      bitWrite(cardChunk2, i + cardChunk2Offset, bitRead(bitHolder1, i));
    }
    if (i < cardChunk2Offset) {
      bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
    }
  }
}

void getCardValues() {
  switch (bitCount) {
    // Example of full card value
    // |>   preamble   <| |>   Actual card value   <|
    // 000000100000000001 11 111000100000100100111000
    // |> write to chunk1 <| |>  write to chunk2   <|
  case 26:
    setCardChunkBits(2, 20, 4);
    break;

  case 27:
    setCardChunkBits(3, 19, 5);
    break;

  case 28:
    setCardChunkBits(4, 18, 6);
    break;

  case 29:
    setCardChunkBits(5, 17, 7);
    break;

  case 30:
    setCardChunkBits(6, 16, 8);
    break;

  case 31:
    setCardChunkBits(7, 15, 9);
    break;

  case 32:
    setCardChunkBits(8, 14, 10);
    break;

  case 33:
    setCardChunkBits(9, 13, 11);
    break;

  case 34:
    setCardChunkBits(10, 12, 12);
    break;

  case 35:
    setCardChunkBits(11, 11, 13);
    break;

  case 36:
    setCardChunkBits(12, 10, 14);
    break;

  case 37:
    setCardChunkBits(13, 9, 15);
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

// write to SD card
void writeSD() {
  File SDFile = SD.open("/cards.jsonl", FILE_APPEND);
  if (!SDFile) {
    Serial.println("[-] SD Card: Failed to open file");
    return;
  }
  String hex =
      String(cardChunk1, HEX) + prefixPad(String(cardChunk2, HEX), '0', 6);
  String raw;
  for (int i = 19; i >= 0; i--) {
    raw += (bitRead(cardChunk1, i));
  }
  for (int i = 23; i >= 0; i--) {
    raw += (bitRead(cardChunk2, i));
  }
  DynamicJsonDocument doc(1024);
  doc["card_type"] = cardType;
  doc["bit_length"] = bitCount;
  doc["facility_code"] = facilityCode;
  doc["card_number"] = cardCode;
  doc["hex"] = hex;
  doc["raw"] = raw;
  Serial.println("[+] New Card Read: ");
  serializeJsonPretty(doc, Serial);

  if (serializeJson(doc, SDFile) == 0) {
    Serial.println("[-] SD Card: Failed to write json card data to file");
  } else {
    SDFile.print("\n");
    SDFile.close();
    Serial.println("\n[+] SD Card: Data Written to SD Card");
  }
}

// cleanup and get ready for the next card
void cleanup() {
  cardType = "";
  bitCount = 0;
  facilityCode = 0;
  cardCode = 0;
  bitHolder1 = 0;
  bitHolder2 = 0;
  cardChunk1 = 0;
  cardChunk2 = 0;
}

// webserver setup and config
AsyncWebServer server(80);

void logRequest(const String &url) {
  Serial.println("[*] Webserver: Requested url: " + url);
  Serial.println("[*] Webserver: Serving gzipped file: " + url + ".gz");
}

String getUrlExtension(const String &url) {
  String extension = url.substring(url.lastIndexOf('.') + 1);

  if (extension == "html" || extension == "htm")
    return "text/html";
  else if (extension == "css")
    return "text/css";
  else if (extension == "js")
    return "text/javascript";
  else if (extension == "json")
    return "application/json";
  else if (extension == "jpg" || extension == "jpeg")
    return "image/jpeg";
  else if (extension == "png")
    return "image/png";
  else if (extension == "svg")
    return "image/svg+xml";
  else
    return "text/plain";
}

void handleGzippedFile(AsyncWebServerRequest *request, const String &url,
                       const String &contentType) {
  logRequest(url);
  AsyncWebServerResponse *response =
      request->beginResponse(LittleFS, url + ".gz", contentType);
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
}

void sendJsonResponse(AsyncWebServerRequest *request,
                      DynamicJsonDocument &json) {
  AsyncResponseStream *response =
      request->beginResponseStream("application/json");
  serializeJson(json, *response);
  request->send(response);
}

void handleGeneralSettingsGet(AsyncWebServerRequest *request) {
  DynamicJsonDocument json(200);
  json["capturing"] = isCapturing;
  json["version"] = version;
  sendJsonResponse(request, json);
}

void handleGeneralSettingsPost(AsyncWebServerRequest *request) {
  int params = request->params();
  for (int i = 0; i < params; i++) {
    AsyncWebParameter *p = request->getParam(i);
    if (p->isPost()) {
      if (p->name() == "capturing") {
        String newCapturingState = p->value().c_str();
        if (newCapturingState == "true") {
          isCapturing = true;
        } else if (newCapturingState == "false") {
          isCapturing = false;
        }
      }
      Serial.printf("[+] Webserver: FormData - [%s]: %s\n", p->name().c_str(),
                    p->value().c_str());
    }
  }
  request->send(200, "text/plain", "General settings updated");
}

void handleJsonFileResponse(AsyncWebServerRequest *request,
                            const String &path) {
  AsyncResponseStream *response =
      request->beginResponseStream("application/json");
  DynamicJsonDocument json(512);

  if (path == "littlefsinfo") {
    json["totalBytes"] = LittleFS.totalBytes();
    json["usedBytes"] = LittleFS.usedBytes();
  } else if (path == "sdcardinfo") {
    json["totalBytes"] = SD.totalBytes();
    json["usedBytes"] = SD.usedBytes();
  }

  serializeJson(json, *response);
  request->send(response);
}

void handleCardDataGet(AsyncWebServerRequest *request) {
  File SDFile = SD.open("/cards.jsonl", FILE_READ);
  String cardData;

  if (SDFile) {
    while (SDFile.available()) {
      cardData = SDFile.readString();
    }
    SDFile.close();
  } else {
    Serial.println("[-] SD Card: error opening json data");
  }

  AsyncWebServerResponse *response =
      request->beginResponse(200, "application/x-ndjson", cardData);
  request->send(response);
}

void handleCardDataPost(AsyncWebServerRequest *request) {
  writeSDFile(jsoncarddataPath, "");
  lastWrittenBitCount = 0;
  for (unsigned char i = 0; i < MAX_BITS; i++) {
    lastWrittenDatabits[i] = 0;
  }

  AsyncWebServerResponse *response =
      request->beginResponse(200, "text/plain", "All card data deleted!");
  request->send(response);
}

void handleWiFiConfigGet(AsyncWebServerRequest *request) {
  AsyncResponseStream *response =
      request->beginResponseStream("application/json");
  DynamicJsonDocument json(512);
  json["ssid"] = ssid;
  json["password"] = password;
  json["channel"] = channel;
  json["hidessid"] = hidessid;
  serializeJson(json, *response);
  request->send(response);
}

void handleWifiConfigPost(AsyncWebServerRequest *request) {
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
      Serial.printf("[+] Webserver: FormData - [%s]: %s\n", p->name().c_str(),
                    p->value().c_str());
    }
    request->send(200, "text/plain", "WiFi config updated. Rebooting now");
  }
}

void handleReboot(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response =
      request->beginResponse(200, "text/plain", "Rebooting device");
  request->send(response);
  delay(5000);
  Serial.println("[*] Rebooting...");
  ESP.restart();
}

void setupWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleGzippedFile(request, "/index.html", "text/html");
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleGzippedFile(request, "/favicon.ico", "image/png");
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleGzippedFile(request, "/index.html", "text/html");
  });

  server.on("/assets/*", HTTP_GET, [](AsyncWebServerRequest *request) {
    String url = request->url();
    String contentType = getUrlExtension(url);

    handleGzippedFile(request, url.c_str(), contentType.c_str());
  });

  server.on("/api/device/littlefsinfo", HTTP_GET,
            [](AsyncWebServerRequest *request) {
              handleJsonFileResponse(request, "littlefsinfo");
            });

  server.on("/api/device/sdcardinfo", HTTP_GET,
            [](AsyncWebServerRequest *request) {
              handleJsonFileResponse(request, "sdcardinfo");
            });

  server.on("/api/device/settings/general", HTTP_GET, handleGeneralSettingsGet);
  server.on("/api/device/settings/general", HTTP_POST,
            handleGeneralSettingsPost);

  server.on("/api/carddata", HTTP_GET, handleCardDataGet);
  server.on("/api/carddata", HTTP_POST, handleCardDataPost);

  server.on("/api/device/wificonfig", HTTP_GET, handleWiFiConfigGet);
  server.on("/api/device/wificonfig", HTTP_POST, handleWifiConfigPost);

  server.on("/api/device/reboot", HTTP_POST, handleReboot);

  server.onNotFound([](AsyncWebServerRequest *request) { request->send(404); });
}

void setup() {
  Serial.begin(115200);

  // initialize SD card
  pinMode(sd_cs, OUTPUT);
  delay(3000);
  if (!SD.begin(sd_cs)) {
    Serial.println("[-] SD Card: An error occurred while initializing");
    Serial.println("[-] SD Card: Fix & Power Cycle");
  } else {
    Serial.println("[+] SD Card: Initialized successfully");
  }

  // initialize LittleFS
  delay(3000);
  if (!LittleFS.begin()) {
    Serial.println("[-] LittleFS: An error occurred while mounting");
  } else {
    Serial.println("[+] LittleFS: Mounted successfully");
  }

  // Check if ssid.txt file exists on SD card
  delay(3000);
  if (!SD.exists(ssidPath)) {
    Serial.println("[-] WiFi Config: ssid.txt file not found");
    // If file doesn't exist, create wifi config files
    writeSDFile(ssidPath, "Tusk");
    writeSDFile(passwordPath, "12345678");
    writeSDFile(channelPath, "1");
    writeSDFile(hidessidPath, "0");
    Serial.println("[+] WiFi Config: WiFi config files created");
    Serial.println("[*] WiFi Config: Rebooting...");
    delay(3000);
    ESP.restart();
  } else {
    Serial.println("[+] WiFi Config: Found ssid.txt - assuming remaining wifi "
                   "config files exist >.>");
  }

  ssid = readSDFileLF(ssidPath);
  password = readSDFileLF(passwordPath);
  channel = readSDFileLF(channelPath);
  hidessid = readSDFileLF(hidessidPath);

  // initialize wifi
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid.c_str(), password.c_str(), channel.toInt(),
              hidessid.toInt());
  Serial.print("[+] WiFi: Creating access point: ");
  Serial.println(ssid);
  Serial.print("[+] WiFi: Gateway IP address: ");
  Serial.println(local_ip);

  // set tx/rx pins for card reader
  pinMode(DATA0, INPUT); // DATA0 (INT0)
  pinMode(DATA1, INPUT); // DATA1 (INT1)

  // binds the ISR functions to the falling edge of INT0 and INT1
  attachInterrupt(DATA0, ISR_INT0, FALLING);
  attachInterrupt(DATA1, ISR_INT1, FALLING);
  weigandCounter = WEIGAND_WAIT_TIME;

  // check for cards.jsonl on SD card
  delay(3000);
  if (!SD.exists("/cards.jsonl")) {
    Serial.println("[-] SD Card: File cards.jsonl not found");
    Serial.println(
        "[*] SD Card: Created cards.jsonl and performing software reset");
    // if file doesn't exist, create it
    writeSDFile(jsoncarddataPath, "");
    Serial.println("[+] SD Card: File cards.jsonl created");
    Serial.println("[*] SD Card: Rebooting...");
    delay(3000);
    ESP.restart();
  } else {
    Serial.println("[+] SD Card: Found cards.jsonl");
  }

  setupWebServer();

  server.begin();
  Serial.println("[+] Webserver: Started");
  Serial.println("[+] Tusk: is running");

  for (unsigned char i = 0; i < MAX_BITS; i++) {
    lastWrittenDatabits[i] = 0;
  }
}

void loop() {
  if (isCapturing) {

    Serial.println("[+] Tusk: Capturing data");

    if (!flagDone) {
      if (--weigandCounter == 0)
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

        printBits();
        // check if card data is valid
        if (bitCount >= 26 && cardType == "hid" &&
            (facilityCode != 0 || cardCode != 0)) {
          writeSD();

          lastWrittenBitCount = bitCount;
          for (i = 0; i < bitCount; i++) {
            lastWrittenDatabits[i] = databits[i];
          }
        } else {
          Serial.println(
              "[-] Tusk: Invalid card data detected. Skipping writing to SD.");
        }
      }

      // cleanup and get ready for the next card
      cleanup();

      for (i = 0; i < MAX_BITS; i++) {
        databits[i] = 0;
      }
    }
  } else {
    // not capturing data - do nothing
    Serial.println("[-] Tusk: Not capturing data");
    delay(60000);
  }
}