#include "card.h"

const char *cardTypeToString(CardType type) {
  switch (type) {
  case CardType::HID:
    return "hid";
  case CardType::Gallagher:
    return "gallagher";
  case CardType::Unknown:
    return "unknown";
  }
  return "unknown";
}
