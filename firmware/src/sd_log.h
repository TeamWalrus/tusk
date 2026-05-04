#pragma once

#include "card.h"
#include <Arduino.h>

namespace sd_log {

// Initializes the SD card and ensures the JSONL log file exists. Returns
// false if the SD card could not be mounted (caller should still continue
// since the rest of the device can run without it).
bool begin(int cs_pin);

// Append a single decoded card to the JSONL log file.
void appendCard(const Card &card);

// Read the entire JSONL log file as a single string. Returns an empty
// string when the SD card or file is unavailable.
String readAll();

// Truncate the JSONL log file.
void clear();

uint64_t totalBytes();
uint64_t usedBytes();

}  // namespace sd_log
