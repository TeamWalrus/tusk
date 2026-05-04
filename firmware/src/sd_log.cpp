#include "sd_log.h"

#include <ArduinoJson.h>
#include <SD.h>
#include <SPI.h>

namespace {
constexpr const char *CARDS_PATH = "/cards.jsonl";
bool mounted = false;
}  // namespace

namespace sd_log {

bool begin(int cs_pin) {
  pinMode(cs_pin, OUTPUT);
  if (!SD.begin(cs_pin)) {
    Serial.println("[-] SD Card: An error occurred while initializing");
    Serial.println("[-] SD Card: Fix & Power Cycle");
    mounted = false;
    return false;
  }
  Serial.println("[+] SD Card: Initialized successfully");
  mounted = true;

  if (!SD.exists(CARDS_PATH)) {
    Serial.println("[-] SD Card: File cards.jsonl not found, creating it");
    File f = SD.open(CARDS_PATH, FILE_WRITE);
    if (f) {
      f.close();
      Serial.println("[+] SD Card: File cards.jsonl created");
    } else {
      Serial.println("[-] SD Card: Could not create cards.jsonl");
    }
  }

  return true;
}

void appendCard(const Card &card) {
  if (!mounted) return;

  File file = SD.open(CARDS_PATH, FILE_APPEND);
  if (!file) {
    Serial.println("[-] SD Card: Failed to open cards.jsonl for append");
    return;
  }

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

  if (serializeJson(doc, file) == 0) {
    Serial.println("[-] SD Card: Failed to write json card data to file");
  }
  file.print('\n');
  file.close();
  Serial.println("[+] SD Card: Card written");
}

String readAll() {
  if (!mounted) return String();
  File file = SD.open(CARDS_PATH, FILE_READ);
  if (!file) {
    Serial.println("[-] SD Card: error opening json data");
    return String();
  }
  String contents;
  while (file.available()) {
    contents += static_cast<char>(file.read());
  }
  file.close();
  return contents;
}

void clear() {
  if (!mounted) return;
  File file = SD.open(CARDS_PATH, FILE_WRITE);
  if (file) file.close();
}

uint64_t totalBytes() { return mounted ? SD.totalBytes() : 0; }
uint64_t usedBytes() { return mounted ? SD.usedBytes() : 0; }

}  // namespace sd_log
