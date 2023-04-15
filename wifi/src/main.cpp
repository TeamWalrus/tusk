#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

// Uncomment for debugging / verbose messages
// #define VERBOSE

/* #####----- WiFi SoftAP config -----##### */
#define ssid "Tusk"         // This is the SSID that ESP32 will broadcast
#define password "12345678" // password should be atleast 8 characters to make it work
#define dns_port 53
const IPAddress local_ip(192, 168, 100, 1);
const IPAddress gateway(192, 168, 100, 1);
const IPAddress subnet(255, 255, 255, 0);

// Define CS pin for the SD card module
#define sd_cs 5

/* #####----- Webserver and DNS config -----##### */
DNSServer dnsServer;
AsyncWebServer server(80);

/* #####----- Card Reader Config -----##### */
// Card Reader variables
#define MAX_BITS 100           // max number of bits
#define WEIGAND_WAIT_TIME 3000 // time to wait for another weigand pulse

unsigned char databits[MAX_BITS]; // stores all of the data bits
volatile unsigned int bitCount = 0;
unsigned char flagDone;       // goes low when data is currently being captured
unsigned int weigand_counter; // countdown until we assume there are no more bits

volatile unsigned long facilityCode = 0; // decoded facility code
volatile unsigned long cardCode = 0;     // decoded card code

// Breaking up card value into 2 chunks to create 10 char HEX value
volatile unsigned long bitHolder1 = 0;
volatile unsigned long bitHolder2 = 0;
volatile unsigned long cardChunk1 = 0;
volatile unsigned long cardChunk2 = 0;

// Define reader input pins
#define DATA0 32 // card reader DATA0
#define DATA1 33 // card reader DATA1

/* #####----- Process interupts -----##### */
// interrupt that happens when INT0 goes low (0 bit)
void ISR_INT0()
{
  // Serial.print("0");
  bitCount++;
  flagDone = 0;

  if (bitCount < 23)
  {
    bitHolder1 = bitHolder1 << 1;
  }
  else
  {
    bitHolder2 = bitHolder2 << 1;
  }
  weigand_counter = WEIGAND_WAIT_TIME;
}

// interrupt that happens when INT1 goes low (1 bit)
void ISR_INT1()
{
  // Serial.print("1");
  databits[bitCount] = 1;
  bitCount++;
  flagDone = 0;

  if (bitCount < 23)
  {
    bitHolder1 = bitHolder1 << 1;
    bitHolder1 |= 1;
  }
  else
  {
    bitHolder2 = bitHolder2 << 1;
    bitHolder2 |= 1;
  }

  weigand_counter = WEIGAND_WAIT_TIME;
}

/* #####----- Print bits function -----##### */
void printBits()
{
  if (bitCount >= 26)
  { // ignore data caused by noise
    Serial.print(bitCount);
    Serial.print(" bit card. ");
    Serial.print("FC = ");
    Serial.print(facilityCode);
    Serial.print(", CC = ");
    Serial.print(cardCode);
    Serial.print(", 44bit HEX = ");
    Serial.print(cardChunk1, HEX);
    Serial.println(cardChunk2, HEX);
  }
}

/* #####----- Process card format function -----##### */
void getCardNumAndSiteCode()
{
  unsigned char i;

  // bits to be decoded differently depending on card format length
  // see http://www.pagemac.com/projects/rfid/hid_data_formats for more info
  // also specifically: www.brivo.com/app/static_data/js/calculate.js
  switch (bitCount)
  {

  ///////////////////////////////////////
  // standard 26 bit format
  // facility code = bits 2 to 9
  case 26:
    for (i = 1; i < 9; i++)
    {
      facilityCode <<= 1;
      facilityCode |= databits[i];
    }

    // card code = bits 10 to 23
    for (i = 9; i < 25; i++)
    {
      cardCode <<= 1;
      cardCode |= databits[i];
    }
    break;

  ///////////////////////////////////////
  // 33 bit HID Generic
  case 33:
    for (i = 1; i < 8; i++)
    {
      facilityCode <<= 1;
      facilityCode |= databits[i];
    }

    // card code
    for (i = 8; i < 32; i++)
    {
      cardCode <<= 1;
      cardCode |= databits[i];
    }
    break;

  ///////////////////////////////////////
  // 34 bit HID Generic
  case 34:
    for (i = 1; i < 17; i++)
    {
      facilityCode <<= 1;
      facilityCode |= databits[i];
    }

    // card code
    for (i = 17; i < 33; i++)
    {
      cardCode <<= 1;
      cardCode |= databits[i];
    }
    break;

  ///////////////////////////////////////
  // 35 bit HID Corporate 1000 format
  // facility code = bits 2 to 14
  case 35:
    for (i = 2; i < 14; i++)
    {
      facilityCode <<= 1;
      facilityCode |= databits[i];
    }

    // card code = bits 15 to 34
    for (i = 14; i < 34; i++)
    {
      cardCode <<= 1;
      cardCode |= databits[i];
    }
    break;
  }
  return;
}

/* #####----- Card value processing -----##### */
// Function to append the card value (bitHolder1 and bitHolder2) to the necessary array
// then translate that to the two chunks for the card value that will be output
void getCardValues()
{
  switch (bitCount)
  {
    // Example of full card value
    // |>   preamble   <| |>   Actual card value   <|
    // 000000100000000001 11 111000100000100100111000
    // |> write to chunk1 <| |>  write to chunk2   <|

  case 26:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 2)
      {
        bitWrite(cardChunk1, i, 1); // Write preamble 1's to the 13th and 2nd bits
      }
      else if (i > 2)
      {
        bitWrite(cardChunk1, i, 0); // Write preamble 0's to all other bits above 1
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 20)); // Write remaining bits to cardChunk1 from bitHolder1
      }
      if (i < 20)
      {
        bitWrite(cardChunk2, i + 4, bitRead(bitHolder1, i)); // Write the remaining bits of bitHolder1 to cardChunk2
      }
      if (i < 4)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i)); // Write the remaining bit of cardChunk2 with bitHolder2 bits
      }
    }
    break;

  case 27:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 3)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 3)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 19));
      }
      if (i < 19)
      {
        bitWrite(cardChunk2, i + 5, bitRead(bitHolder1, i));
      }
      if (i < 5)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 28:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 4)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 4)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 18));
      }
      if (i < 18)
      {
        bitWrite(cardChunk2, i + 6, bitRead(bitHolder1, i));
      }
      if (i < 6)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 29:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 5)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 5)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 17));
      }
      if (i < 17)
      {
        bitWrite(cardChunk2, i + 7, bitRead(bitHolder1, i));
      }
      if (i < 7)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 30:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 6)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 6)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 16));
      }
      if (i < 16)
      {
        bitWrite(cardChunk2, i + 8, bitRead(bitHolder1, i));
      }
      if (i < 8)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 31:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 7)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 7)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 15));
      }
      if (i < 15)
      {
        bitWrite(cardChunk2, i + 9, bitRead(bitHolder1, i));
      }
      if (i < 9)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 32:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 8)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 8)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 14));
      }
      if (i < 14)
      {
        bitWrite(cardChunk2, i + 10, bitRead(bitHolder1, i));
      }
      if (i < 10)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 33:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 9)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 9)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 13));
      }
      if (i < 13)
      {
        bitWrite(cardChunk2, i + 11, bitRead(bitHolder1, i));
      }
      if (i < 11)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 34:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 10)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 10)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 12));
      }
      if (i < 12)
      {
        bitWrite(cardChunk2, i + 12, bitRead(bitHolder1, i));
      }
      if (i < 12)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 35:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 11)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 11)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 11));
      }
      if (i < 11)
      {
        bitWrite(cardChunk2, i + 13, bitRead(bitHolder1, i));
      }
      if (i < 13)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 36:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13 || i == 12)
      {
        bitWrite(cardChunk1, i, 1);
      }
      else if (i > 12)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 10));
      }
      if (i < 10)
      {
        bitWrite(cardChunk2, i + 14, bitRead(bitHolder1, i));
      }
      if (i < 14)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;

  case 37:
    for (int i = 19; i >= 0; i--)
    {
      if (i == 13)
      {
        bitWrite(cardChunk1, i, 0);
      }
      else
      {
        bitWrite(cardChunk1, i, bitRead(bitHolder1, i + 9));
      }
      if (i < 9)
      {
        bitWrite(cardChunk2, i + 15, bitRead(bitHolder1, i));
      }
      if (i < 15)
      {
        bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
      }
    }
    break;
  }
  return;
}

/* #####----- Write to SD card -----##### */
void writeSD()
{
  // open file - note only one file can be open at a time
  File SDFile = SD.open("/cards.jsonl", FILE_APPEND);
  //  if file opens correctly, write to it
  if (SDFile)
  {
    if (bitCount >= 26)
    { // ignore data caused by noise
      DynamicJsonDocument doc(512);
      doc["bit_length"] = bitCount;
      doc["facility_code"] = (facilityCode, DEC);
      doc["card_number"] = (cardCode, DEC);
      doc["hex"] = (cardChunk1, HEX) + (cardChunk2, HEX);
      String raw;
      for (int i = 19; i >= 0; i--)
      {
        raw = (bitRead(cardChunk1, i));
      }
      for (int i = 23; i >= 0; i--)
      {
        raw += (bitRead(cardChunk2, i));
      }
      doc["raw"] = raw;
      if (serializeJson(doc, SDFile) == 0)
      {
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

void setup()
{
  Serial.begin(115200);

  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);
  dnsServer.start(dns_port, "*", local_ip);

#ifdef VERBOSE
  Serial.println("\n[*] WiFi: Creating ESP32 Access Point");
  Serial.print("[+] WiFi: Access Point created with IP Gateway ");
  Serial.println(local_ip);
#endif

  // set tx/rx pins for card reader
  pinMode(DATA0, INPUT); // DATA0 (INT0)
  pinMode(DATA1, INPUT); // DATA1 (INT1)

  // binds the ISR functions to the falling edge of INT0 and INT1
  attachInterrupt(DATA0, ISR_INT0, FALLING);
  attachInterrupt(DATA1, ISR_INT1, FALLING);
  weigand_counter = WEIGAND_WAIT_TIME;

  // Initialize SD card
  pinMode(sd_cs, OUTPUT);

  // SD.begin(sd_cs);

  delay(5000);
  if (!SD.begin(sd_cs))
  {
#ifdef VERBOSE
    Serial.println("[-] SD Card: An error occurred while initializing");
    Serial.println("[-] SD Card: Fix & Power Cycle");
#endif
  }
  else
  {
#ifdef VERBOSE
    Serial.println("[+] SD Card: Initialized successfully");
#endif
  }

  // Check for cards.jsonl on SD card
  delay(3000);
  if (!SD.exists("/cards.jsonl"))
  {
#ifdef VERBOSE
    Serial.println("[-] SD Card: File cards.jsonl not found");
    Serial.println("[*] SD Card: Created cards.jsonl and performing software reset");
#endif
    // If file doesn't exist, create it
    File file = SD.open("/cards.jsonl", FILE_WRITE);
    file.print("");
    file.close();
#ifdef VERBOSE
    Serial.println("[+] SD Card: File cards.jsonl created");
    Serial.println("[*] SD Card: Rebooting...");
#endif
    delay(5000);
    ESP.restart();
  }
  else
  {
#ifdef VERBOSE
    Serial.println("[+] SD Card: Found cards.jsonl");
#endif
  }

  delay(5000);
  if (!LittleFS.begin())
  {
#ifdef VERBOSE
    Serial.println("[-] LittleFS: An error occurred while mounting");
#endif
  }
  else
  {
#ifdef VERBOSE
    Serial.println("[+] LittleFS: Mounted successfully");
#endif
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
#ifdef VERBOSE
              Serial.println("[*] Webserver: Serving file:  /index.html.gz");
#endif
              AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
              response->addHeader("Content-Encoding", "gzip");
              request->send(response); });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request)
            {
#ifdef VERBOSE
              Serial.println("[*] Webserver: Serving file:  /index.html.gz");
#endif
              AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
              response->addHeader("Content-Encoding", "gzip");
              request->send(response); });

  server.on("/assets/*", HTTP_GET, [](AsyncWebServerRequest *request)
            {
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

            AsyncWebServerResponse *response = request->beginResponse(LittleFS, urlgz, contentType);
            response->addHeader("Content-Encoding", "gzip");
            request->send(response); });

  server.on("/api/littlefsinfo", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              AsyncResponseStream *response = request->beginResponseStream("application/json");
              DynamicJsonDocument json(512);
              json["totalBytes"] = LittleFS.totalBytes();
              json["usedBytes"] = LittleFS.usedBytes();
              serializeJson(json, *response);
              request->send(response); });

  server.on("/api/sdcardinfo", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              AsyncResponseStream *response = request->beginResponseStream("application/json");
              DynamicJsonDocument json(512);
              json["totalBytes"] = SD.totalBytes();
              json["usedBytes"] = SD.usedBytes();
              serializeJson(json, *response);
              request->send(response); });

  server.on("/api/carddata", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              File SDFile = SD.open("/cards.jsonl", FILE_READ);
              String cardData;
              if (SDFile)
              {
                while (SDFile.available())
                {
                  cardData = SDFile.readString();
                }
#ifdef VERBOSE
                Serial.println(cardData);
#endif
                SDFile.close();
              }
              else
              {
#ifdef VERBOSE
                Serial.println("[-] SD Card: error opening json data");
#endif
              }
              // Should we rather be streaming the json data?
              // AsyncResponseStream *response = request->beginResponseStream("application/json");
              AsyncWebServerResponse *response = request->beginResponse(200, "application/x-ndjson", cardData);
              request->send(response); });

  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404); });

  server.on("*", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->redirect("http://" + local_ip.toString()); });

  server.begin();
#ifdef VERBOSE
  Serial.println("[+] Webserver: Started");
#endif
  Serial.println("[*] Tusk is running");
}

void loop()
{
  dnsServer.processNextRequest();
  vTaskDelay(1);

  if (!flagDone)
  {
    if (--weigand_counter == 0)
      flagDone = 1;
  }

  // if there are bits and the weigand counter went out
  if (bitCount > 0 && flagDone)
  {
    unsigned char i;

    getCardValues();
    getCardNumAndSiteCode();

#ifdef VERBOSE
    printBits();
#endif

    writeSD();

    // cleanup and get ready for the next card
    bitCount = 0;
    facilityCode = 0;
    cardCode = 0;
    bitHolder1 = 0;
    bitHolder2 = 0;
    cardChunk1 = 0;
    cardChunk2 = 0;

    for (i = 0; i < MAX_BITS; i++)
    {
      databits[i] = 0;
    }
  }
}