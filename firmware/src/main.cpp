// vim: ts=2 sw=2 et

#include "card.h"
#include "card_decoder.h"
#include "config_store.h"
#include "sd_log.h"
#include "web_server.h"
#include "wiegand.h"

#include <Arduino.h>
#include <LittleFS.h>
#include <WiFi.h>

namespace {
constexpr int SD_CS_PIN = 5;
constexpr int DATA0_PIN = 32;
constexpr int DATA1_PIN = 33;
const IPAddress AP_IP(192, 168, 100, 1);
const IPAddress AP_GATEWAY(192, 168, 100, 1);
const IPAddress AP_SUBNET(255, 255, 255, 0);
}  // namespace

// State shared with web_server. Defined here so the linker can resolve the
// extern declarations there without dragging more headers around.
bool g_capturing = true;
bool g_debug_mode = false;
const char *kFirmwareVersion = "0.3";

namespace {

void printCardSummary(const Card &card) {
  Serial.print("[*] Bit length: ");
  Serial.println(card.bit_count);
  Serial.print("[*] Facility code: ");
  Serial.println(card.facility_code);
  Serial.print("[*] Card number: ");
  Serial.println(card.card_number);
  if (card.type == CardType::Gallagher) {
    Serial.print("[*] Region Code: ");
    Serial.println(card.region_code);
    Serial.print("[*] Issue Level: ");
    Serial.println(card.issue_level);
  }
  Serial.print("[*] Hex: ");
  Serial.println(card.hex);
  Serial.print("[*] Raw: ");
  Serial.println(card.raw);
}

void startAccessPoint() {
  WiFiSettings s = config_store::getWiFi();
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
  WiFi.softAP(s.ssid.c_str(), s.password.c_str(), s.channel, s.hidden);
  Serial.print("[+] WiFi: Creating access point: ");
  Serial.println(s.ssid);
  Serial.print("[+] WiFi: Gateway IP address: ");
  Serial.println(AP_IP);
}

}  // namespace

void setup() {
  Serial.begin(115200);

  config_store::begin();
  g_capturing = config_store::getCapturing();
  g_debug_mode = config_store::getDebugMode();

  sd_log::begin(SD_CS_PIN);

  if (!LittleFS.begin()) {
    Serial.println("[-] LittleFS: An error occurred while mounting");
  } else {
    Serial.println("[+] LittleFS: Mounted successfully");
  }

  startAccessPoint();

  wiegand::begin(DATA0_PIN, DATA1_PIN);

  web_server::setupRoutes();
  web_server::begin();
  Serial.println("[+] Tusk: is running");
}

void loop() {
  // Track disarm→arm transitions so we can flush any partial/stale Wiegand
  // state captured by the ISRs while we weren't processing. Without this,
  // re-arming could replay garbage or drop the next legitimate read.
  static bool was_disarmed = false;

  if (!g_capturing) {
    if (!was_disarmed) {
      Serial.println("[-] Tusk: capture disarmed");
      was_disarmed = true;
    }
    // Discard any in-flight Wiegand bits while disarmed and yield briefly
    // so the network/web stack can run. Short delay so re-arm is picked up
    // within ~50ms instead of up to a minute.
    wiegand::reset();
    delay(50);
    return;
  }

  if (was_disarmed) {
    Serial.println("[+] Tusk: capture re-armed");
    wiegand::reset();
    was_disarmed = false;
  }

  if (!wiegand::ready()) return;

  // Debug mode: bypass the dedup gate so a single test card can be
  // presented repeatedly to verify the reader/wiegand/web pipeline.
  if (g_debug_mode || wiegand::changedSinceLastAccepted()) {
    auto decoded = decodeCard(wiegand::bits(), wiegand::bitCount());
    if (decoded && (decoded->facility_code != 0 || decoded->card_number != 0)) {
      printCardSummary(*decoded);
      sd_log::appendCard(*decoded);
      web_server::broadcastCard(*decoded);
      wiegand::markAccepted();
    } else if (decoded) {
      Serial.println(
          "[-] Tusk: Invalid card data detected - blank facility code or "
          "card number");
    } else {
      Serial.println(
          "[-] Tusk: Invalid card data detected - bitCount not within valid "
          "range");
    }
  }

  wiegand::reset();
}
