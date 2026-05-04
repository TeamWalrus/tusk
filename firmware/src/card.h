#pragma once

#include <Arduino.h>
#include <cstdint>

enum class CardType {
  Unknown,
  HID,
  Gallagher,
};

const char *cardTypeToString(CardType type);

struct Card {
  CardType type = CardType::Unknown;
  uint16_t bit_count = 0;
  uint32_t facility_code = 0;
  uint32_t card_number = 0;
  uint32_t region_code = 0;
  uint32_t issue_level = 0;
  String hex;
  String raw;
};
