// vim: ts=2 sw=2 et

#include "ArduinoJson.h"
#include "AsyncJson.h"
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>

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

// Write file to SD Card
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

/* #####----- Card Reader Config -----##### */
// Card Reader variables

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

// Breaking up card value into 2 chunks to create 10 char HEX value
volatile unsigned long bitHolder1 = 0;
volatile unsigned long bitHolder2 = 0;
unsigned long cardChunk1 = 0;
unsigned long cardChunk2 = 0;

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

// Gallagher cardholder credential data structure
struct CardholderCredentials
{
  int region_code;
  int facility_code;
  int card_number;
  int issue_level;
};

// Gallagher descramble function to deobfuscate card data
byte descramble(byte arr)
{
  byte lut[] = {
      0x2f, 0x6e, 0xdd, 0xdf, 0x1d, 0x0f, 0xb0, 0x76, 0xad, 0xaf, 0x7f, 0xbb, 0x77, 0x85, 0x11, 0x6d,
      0xf4, 0xd2, 0x84, 0x42, 0xeb, 0xf7, 0x34, 0x55, 0x4a, 0x3a, 0x10, 0x71, 0xe7, 0xa1, 0x62, 0x1a,
      0x3e, 0x4c, 0x14, 0xd3, 0x5e, 0xb2, 0x7d, 0x56, 0xbc, 0x27, 0x82, 0x60, 0xe3, 0xae, 0x1f, 0x9b,
      0xaa, 0x2b, 0x95, 0x49, 0x73, 0xe1, 0x92, 0x79, 0x91, 0x38, 0x6c, 0x19, 0x0e, 0xa9, 0xe2, 0x8d,
      0x66, 0xc7, 0x5a, 0xf5, 0x1c, 0x80, 0x99, 0xbe, 0x4e, 0x41, 0xf0, 0xe8, 0xa6, 0x20, 0xab, 0x87,
      0xc8, 0x1e, 0xa0, 0x59, 0x7b, 0x0c, 0xc3, 0x3c, 0x61, 0xcc, 0x40, 0x9e, 0x06, 0x52, 0x1b, 0x32,
      0x8c, 0x12, 0x93, 0xbf, 0xef, 0x3b, 0x25, 0x0d, 0xc2, 0x88, 0xd1, 0xe0, 0x07, 0x2d, 0x70, 0xc6,
      0x29, 0x6a, 0x4d, 0x47, 0x26, 0xa3, 0xe4, 0x8b, 0xf6, 0x97, 0x2c, 0x5d, 0x3d, 0xd7, 0x96, 0x28,
      0x02, 0x08, 0x30, 0xa7, 0x22, 0xc9, 0x65, 0xf8, 0xb7, 0xb4, 0x8a, 0xca, 0xb9, 0xf2, 0xd0, 0x17,
      0xff, 0x46, 0xfb, 0x9a, 0xba, 0x8f, 0xb6, 0x69, 0x68, 0x8e, 0x21, 0x6f, 0xc4, 0xcb, 0xb3, 0xce,
      0x51, 0xd4, 0x81, 0x00, 0x2e, 0x9c, 0x74, 0x63, 0x45, 0xd9, 0x16, 0x35, 0x5f, 0xed, 0x78, 0x9f,
      0x01, 0x48, 0x04, 0xc1, 0x33, 0xd6, 0x4f, 0x94, 0xde, 0x31, 0x9d, 0x0a, 0xac, 0x18, 0x4b, 0xcd,
      0x98, 0xb8, 0x37, 0xa2, 0x83, 0xec, 0x03, 0xd8, 0xda, 0xe5, 0x7a, 0x6b, 0x53, 0xd5, 0x15, 0xa4,
      0x43, 0xe9, 0x90, 0x67, 0x58, 0xc0, 0xa5, 0xfa, 0x2a, 0xb1, 0x75, 0x50, 0x39, 0x5c, 0xe6, 0xdc,
      0x89, 0xfc, 0xcf, 0xfe, 0xf9, 0x57, 0x54, 0x64, 0xa8, 0xee, 0x23, 0x0b, 0xf1, 0xea, 0xfd, 0xdb,
      0xbd, 0x09, 0xb5, 0x5b, 0x05, 0x86, 0x13, 0xf3, 0x24, 0xc5, 0x3f, 0x44, 0x72, 0x7c, 0x7e, 0x36};

  return lut[arr];
}

// Function to deobfuscate Gallagher cardholder credentials
CardholderCredentials deobfuscate_cardholder_credentials(byte *bytes)
{
  byte *arr = new byte[8];
  for (int i = 0; i < 8; i++)
  {
    arr[i] = descramble(bytes[i]);
  }

  CardholderCredentials credentials;
  Serial.println("trying to deobfuscate cardholder credentials");
  // 4bit region code
  credentials.region_code = (arr[3] & 0x1E) >> 1;
  // 16bit facility code
  credentials.facility_code = ((arr[5] & 0x0F) << 12) | (arr[1] << 4) | ((arr[7] >> 4) & 0x0F);
  // 24bit card number
  credentials.card_number = (arr[0] << 16) | ((arr[4] & 0x1F) << 11) | (arr[2] << 3) | ((arr[3] & 0xE0) >> 5);
  // 4bit issue level
  credentials.issue_level = (arr[7] & 0x0F);

  delete[] arr;

  return credentials;
}

// Function to decode raw Gallagher cardax 125khz card data
void decode_cardax_125khz(String data)
{
  String magic_prefix = "01111111111010";

  int i = data.indexOf(magic_prefix);
  if (i == -1)
  {
    Serial.println("Magic prefix not found - not a valid gallagher cardax card");
    return;
  }

  String b = "";
  data = data.substring(i + 16);
  data = data.substring(0, 9 * 8 + 8);
  if (data.length() != 9 * 8 + 8)
  {
    Serial.println("Invalid data length.");
    return;
  }

  while (b.length() < 64 + 8)
  {
    String n = data.substring(9 * b.length() / 8);
    if (b.length() < 64)
    {
      if (n.charAt(7) == n.charAt(8))
      {
        Serial.println("Invalid data!");
        return;
      }
    }
    b += n.substring(0, 8);
  }

  uint64_t n = strtoull(b.substring(0, 64).c_str(), NULL, 2);

  // This always fails... not sure why
  // byte check_sum = 0x2C;
  // byte xcc[] = {0x7, 0xE, 0x1C, 0x38, 0x70, 0xE0, 0xC7, 0x89};

  // for (int c = 0; c < 8; c++) {
  //   byte ncs = check_sum ^ ((byte*)&n)[c];
  //   check_sum = 0;
  //   for (int i = 0; i < 8; i++) {
  //     if (ncs & (1 << i)) {
  //       check_sum ^= xcc[i];
  //     }
  //   }
  // }

  // if (strtoull(b.substring(0, 64).c_str(), NULL, 2) != check_sum) {
  //   Serial.println("Checksum did not match.");
  //   return;
  // }

  // char hexStr[17];
  // sprintf(hexStr, "%016llX", n);
  // Serial.println(hexStr);

  byte byteArr[8];
  for (int i = 0; i < 8; i++)
  {
    byteArr[i] = (n >> (56 - i * 8)) & 0xFF;
  }

  Serial.print("card data: ");
  for (int i = 0; i < 8; i++)
  {
    Serial.print(byteArr[i], HEX);
  }
  Serial.println();
}

/* #####----- Print bits function -----##### */
void printBits() {
  if (bitCount >= 26) { // ignore data caused by noise
    Serial.print("card type: ");
    Serial.println(cardType);
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
    cardType = "hid";
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
    cardType = "hid";
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
    cardType = "hid";
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
    cardType = "hid";
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
      doc["card_type"] = cardType;
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
      Serial.println("[+] New Card Read: ");
      serializeJsonPretty(doc, Serial);
      if (serializeJson(doc, SDFile) == 0) {
        Serial.println("[-] SD Card: Failed to write json card data to file");
      }
      SDFile.print("\n");
      SDFile.close();
      Serial.println("[+] SD Card: Data Written to SD Card");
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize SD card
  pinMode(sd_cs, OUTPUT);
  delay(3000);
  if (!SD.begin(sd_cs)) {
    Serial.println("[-] SD Card: An error occurred while initializing");
    Serial.println("[-] SD Card: Fix & Power Cycle");
  } else {
    Serial.println("[+] SD Card: Initialized successfully");
  }

  // Initialize LittleFS
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
    Serial.println("[+] WiFi Config: Found ssid.txt - assuming remaining wifi config files exist >.>");
  }

  ssid = readSDFileLF(ssidPath);
  password = readSDFileLF(passwordPath);
  channel = readSDFileLF(channelPath);
  hidessid = readSDFileLF(hidessidPath);

  // Initialize wifi
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

  // Check for cards.jsonl on SD card
  delay(3000);
  if (!SD.exists("/cards.jsonl")) {
    Serial.println("[-] SD Card: File cards.jsonl not found");
    Serial.println(
        "[*] SD Card: Created cards.jsonl and performing software reset");
    // If file doesn't exist, create it
    writeSDFile(jsoncarddataPath, "");
    Serial.println("[+] SD Card: File cards.jsonl created");
    Serial.println("[*] SD Card: Rebooting...");
    delay(3000);
    ESP.restart();
  } else {
    Serial.println("[+] SD Card: Found cards.jsonl");
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response =
        request->beginResponse(LittleFS, "/index.html.gz", "text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response =
        request->beginResponse(LittleFS, "/favicon.ico", "image/png");
    request->send(response); 
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("[*] Webserver: Serving file:  /index.html.gz");
    AsyncWebServerResponse *response =
        request->beginResponse(LittleFS, "/index.html.gz", "text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/assets/*", HTTP_GET, [](AsyncWebServerRequest *request) {
    String url = request->url();
    String urlgz = url + ".gz";
    Serial.println("[*] Webserver: Requested url: " + url);
    Serial.println("[*] Webserver: Serving gzipped file: " + url);
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
      Serial.println("[-] SD Card: error opening json data");
    }
    // Should we rather be streaming the json data?
    // https://github.com/me-no-dev/ESPAsyncWebServer#arduinojson-advanced-response
    AsyncWebServerResponse *response =
        request->beginResponse(200, "application/x-ndjson", cardData);
    request->send(response);
  });

  server.on("/api/delete/carddata", HTTP_POST,
            [](AsyncWebServerRequest *request) {
              writeSDFile(jsoncarddataPath, "");
              lastWrittenBitCount = 0;
              for (unsigned char i = 0; i < MAX_BITS; i++) {
                lastWrittenDatabits[i] = 0;
              }
              AsyncWebServerResponse *response =
                  request->beginResponse(200, "text/plain", "All card data deleted!");
              request->send(response);
            });

  server.on("/api/wificonfig", HTTP_GET,
            [](AsyncWebServerRequest *request) {
              AsyncResponseStream *response =
                  request->beginResponseStream("application/json");
              DynamicJsonDocument json(512);
              json["ssid"] = ssid;
              json["password"] = password;
              json["channel"] = channel;
              json["hidessid"] = hidessid;
              serializeJson(json, *response);
              request->send(response); 
            });

  server.on(
      "/api/wificonfig/update", HTTP_POST, [](AsyncWebServerRequest *request) {
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
        }
        request->send(200, "text/plain", "WiFi config updated. Rebooting now");
      });

  server.on("/api/device/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
      AsyncWebServerResponse *response =
        request->beginResponse(200, "text/plain", "Rebooting device");
      request->send(response);
      delay(5000);
      Serial.println("[*] Rebooting...");
      ESP.restart(); 
    });

  server.onNotFound([](AsyncWebServerRequest *request) { request->send(404); });

  server.begin();
  Serial.println("[+] Webserver: Started");
  Serial.println("[*] Tusk is running");

  for (unsigned char i = 0; i < MAX_BITS; i++) {
    lastWrittenDatabits[i] = 0;
  }
}

void loop() {
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

      writeSD();

      lastWrittenBitCount = bitCount;
      for (i = 0; i < bitCount; i++) {
        lastWrittenDatabits[i] = databits[i];
      }
    }

    // cleanup and get ready for the next card
    cardType = "";
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
