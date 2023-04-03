#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
// #include "wsEventHandler.h"

#define ssid "Tusk"         // This is the SSID that ESP32 will broadcast
#define password "12345678" // password should be atleast 8 characters to make it work
#define dns_port 53
const IPAddress local_ip(192, 168, 100, 1);
const IPAddress gateway(192, 168, 100, 1);
const IPAddress subnet(255, 255, 255, 0);

// Comment out for debugging / verbose messages
// #define VERBOSE

DNSServer dnsServer;
AsyncWebServer server(80);
// AsyncWebSocket websocket("/ws");

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
  Serial.println("\n[*] Creating ESP32 Access Point");
  Serial.print("[+] Access Point created with IP Gateway ");
  Serial.println(local_ip());
#endif

  if (!LittleFS.begin())
  {
#ifdef VERBOSE
    Serial.println("[-] An Error has occurred while mounting LITTLEFS");
#endif
    return;
  }

  // bind websocket to async web server
  // websocket.onEvent(wsEventHandler);
  // server.addHandler(&websocket);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
#ifdef VERBOSE
              Serial.println("Serving file:  /index.html.gz");
#endif
              AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
              response->addHeader("Content-Encoding", "gzip");
              request->send(response); });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request)
            {
#ifdef VERBOSE
              Serial.println("Serving file:  /index.html.gz");
#endif
              AsyncWebServerResponse *response = request->beginResponse(LittleFS, "/index.html.gz", "text/html");
              response->addHeader("Content-Encoding", "gzip");
              request->send(response); });

  server.on("/assets/*", HTTP_GET, [](AsyncWebServerRequest *request)
            {
            String url = request->url();
            String urlgz = url + ".gz";
#ifdef VERBOSE
            Serial.println("Requested url file: " + url);
            Serial.println("Serving gzipped file: " + url);
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
              // Get LittleFS info
              unsigned int totalBytes = LittleFS.totalBytes();
              unsigned int usedBytes = LittleFS.usedBytes();

              AsyncResponseStream *response = request->beginResponseStream("application/json");
              DynamicJsonDocument json(512);
              json["totalBytes"] = LittleFS.totalBytes();
              json["usedBytes"] = LittleFS.usedBytes();
              serializeJson(json, *response);
              request->send(response); });

  // write some dummy card data to file
  File dummycarddata = LittleFS.open("/data.json", "w");
  StaticJsonDocument<512> doc;
  JsonArray entries = doc.createNestedArray("entries");

  JsonObject entries_0 = entries.createNestedObject();
  entries_0["id"] = "1";
  entries_0["bit_length"] = "26";
  entries_0["facility_code"] = "123";
  entries_0["card_number"] = "57273";
  entries_0["hex"] = "AAAAAAA";
  entries_0["raw"] = "0000101101001010111111";

  JsonObject entries_1 = entries.createNestedObject();
  entries_1["id"] = "2";
  entries_1["bit_length"] = "34";
  entries_1["facility_code"] = "987";
  entries_1["card_number"] = "121212";
  entries_1["hex"] = "FFFFFFF";
  entries_1["raw"] = "0000101101001010111111";

  if (serializeJson(doc, dummycarddata) == 0)
  {
    Serial.println(F("Failed to write to file"));
  }
  dummycarddata.close();

  server.on("/api/carddata", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              File file = LittleFS.open("/data.json", "r");
              if (!file)
              {
                Serial.println("No data found");
                return;
              }
              String data;
              while (file.available())
              {
                data = file.readString();
              }
              file.close();
              // Should we rather be streaming the json data?
              // AsyncResponseStream *response = request->beginResponseStream("application/json");
              AsyncWebServerResponse *response = request->beginResponse(200, "application/json", data);
              request->send(response); });

  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404); });

  server.on("*", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->redirect("http://" + local_ip.toString()); });

  server.begin();
  Serial.println("Server Started");
}

void loop()
{
  dnsServer.processNextRequest();
  vTaskDelay(1);
}