#include "web_server.h"

#include "config_store.h"
#include "sd_log.h"

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>

extern bool g_capturing;
extern bool g_debug_mode;
extern const char *kFirmwareVersion;

namespace web_server {
namespace {

AsyncWebServer server(80);
AsyncEventSource events("/events");

void logRequest(const String &url) {
  Serial.println("[*] Webserver: Requested url: " + url);
  Serial.println("[*] Webserver: Serving gzipped file: " + url + ".gz");
}

String contentTypeFor(const String &url) {
  String ext = url.substring(url.lastIndexOf('.') + 1);
  if (ext == "html" || ext == "htm") return "text/html";
  if (ext == "css") return "text/css";
  if (ext == "js") return "text/javascript";
  if (ext == "json") return "application/json";
  if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
  if (ext == "png") return "image/png";
  if (ext == "svg") return "image/svg+xml";
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

void handleGeneralSettingsGet(AsyncWebServerRequest *request) {
  JsonDocument doc;
  doc["capturing"] = g_capturing;
  doc["debug_mode"] = g_debug_mode;
  doc["version"] = kFirmwareVersion;
  String body;
  serializeJson(doc, body);
  request->send(200, "application/json", body);
}

void handleGeneralSettingsPost(AsyncWebServerRequest *request) {
  for (size_t i = 0; i < request->params(); i++) {
    const AsyncWebParameter *p = request->getParam(i);
    if (!p->isPost()) continue;
    if (p->name() == "capturing") {
      g_capturing = (p->value() == "true");
      config_store::setCapturing(g_capturing);
    } else if (p->name() == "debug_mode") {
      g_debug_mode = (p->value() == "true");
      config_store::setDebugMode(g_debug_mode);
    }
    Serial.printf("[+] Webserver: FormData - [%s]: %s\n", p->name().c_str(),
                  p->value().c_str());
  }
  request->send(200, "text/plain", "General settings updated");
}

void handleFilesystemInfo(AsyncWebServerRequest *request, bool sdcard) {
  JsonDocument doc;
  if (sdcard) {
    doc["totalBytes"] = sd_log::totalBytes();
    doc["usedBytes"] = sd_log::usedBytes();
  } else {
    doc["totalBytes"] = LittleFS.totalBytes();
    doc["usedBytes"] = LittleFS.usedBytes();
  }
  String body;
  serializeJson(doc, body);
  request->send(200, "application/json", body);
}

void handleCardDataGet(AsyncWebServerRequest *request) {
  request->send(200, "application/x-ndjson", sd_log::readAll());
}

void handleCardDataPost(AsyncWebServerRequest *request) {
  sd_log::clear();
  broadcastClear();
  request->send(200, "text/plain", "All card data deleted!");
}

void handleWiFiConfigGet(AsyncWebServerRequest *request) {
  WiFiSettings s = config_store::getWiFi();
  JsonDocument doc;
  doc["ssid"] = s.ssid;
  doc["password"] = s.password;
  doc["channel"] = String(s.channel);
  doc["hidessid"] = s.hidden ? "1" : "0";
  String body;
  serializeJson(doc, body);
  request->send(200, "application/json", body);
}

void handleWiFiConfigPost(AsyncWebServerRequest *request) {
  WiFiSettings s = config_store::getWiFi();
  // Browsers do not submit unchecked checkboxes; treat absence as "off".
  s.hidden = false;
  for (size_t i = 0; i < request->params(); i++) {
    const AsyncWebParameter *p = request->getParam(i);
    if (!p->isPost()) continue;
    const String &name = p->name();
    const String &value = p->value();
    if (name == "ssid") s.ssid = value;
    else if (name == "password") s.password = value;
    else if (name == "channel") s.channel = value.toInt();
    else if (name == "hidessid") s.hidden = (value == "on" || value == "1");
    Serial.printf("[+] Webserver: FormData - [%s]: %s\n", name.c_str(),
                  value.c_str());
  }
  config_store::setWiFi(s);
  request->send(200, "text/plain", "WiFi config updated. Rebooting now");
}

void handleReboot(AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "Rebooting device");
  delay(500);
  Serial.println("[*] Rebooting...");
  ESP.restart();
}

}  // namespace

void setupRoutes() {
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
    handleGzippedFile(request, url, contentTypeFor(url));
  });

  server.on("/api/device/littlefsinfo", HTTP_GET,
            [](AsyncWebServerRequest *r) { handleFilesystemInfo(r, false); });
  server.on("/api/device/sdcardinfo", HTTP_GET,
            [](AsyncWebServerRequest *r) { handleFilesystemInfo(r, true); });

  server.on("/api/device/settings/general", HTTP_GET,
            handleGeneralSettingsGet);
  server.on("/api/device/settings/general", HTTP_POST,
            handleGeneralSettingsPost);

  server.on("/api/carddata", HTTP_GET, handleCardDataGet);
  server.on("/api/carddata", HTTP_POST, handleCardDataPost);

  server.on("/api/device/wificonfig", HTTP_GET, handleWiFiConfigGet);
  server.on("/api/device/wificonfig", HTTP_POST, handleWiFiConfigPost);

  server.on("/api/device/reboot", HTTP_POST, handleReboot);

  server.addHandler(&events);

  server.onNotFound(
      [](AsyncWebServerRequest *request) { request->send(404); });
}

void begin() {
  server.begin();
  Serial.println("[+] Webserver: Started");
}

void broadcastCard(const Card &card) {
  JsonDocument doc;
  doc["card_type"] = cardTypeToString(card.type);
  doc["bit_length"] = card.bit_count;
  doc["facility_code"] = card.facility_code;
  doc["card_number"] = card.card_number;
  if (card.type == CardType::Gallagher) {
    doc["issue_level"] = card.issue_level;
    doc["region_code"] = card.region_code;
  }
  doc["raw"] = card.raw;
  doc["hex"] = card.hex;
  String payload;
  serializeJson(doc, payload);
  events.send(payload.c_str(), "card", millis());
}

void broadcastClear() { events.send("", "clear", millis()); }

}  // namespace web_server
