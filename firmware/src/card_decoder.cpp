#include "card_decoder.h"
#include <Arduino.h>

namespace {

// Lookup table used to deobfuscate Gallagher Cardax cardholder credentials.
constexpr uint8_t GALLAGHER_LUT[256] = {
    0x2f, 0x6e, 0xdd, 0xdf, 0x1d, 0x0f, 0xb0, 0x76, 0xad, 0xaf, 0x7f, 0xbb,
    0x77, 0x85, 0x11, 0x6d, 0xf4, 0xd2, 0x84, 0x42, 0xeb, 0xf7, 0x34, 0x55,
    0x4a, 0x3a, 0x10, 0x71, 0xe7, 0xa1, 0x62, 0x1a, 0x3e, 0x4c, 0x14, 0xd3,
    0x5e, 0xb2, 0x7d, 0x56, 0xbc, 0x27, 0x82, 0x60, 0xe3, 0xae, 0x1f, 0x9b,
    0xaa, 0x2b, 0x95, 0x49, 0x73, 0xe1, 0x92, 0x79, 0x91, 0x38, 0x6c, 0x19,
    0x0e, 0xa9, 0xe2, 0x8d, 0x66, 0xc7, 0x5a, 0xf5, 0x1c, 0x80, 0x99, 0xbe,
    0x4e, 0x41, 0xf0, 0xe8, 0xa6, 0x20, 0xab, 0x87, 0xc8, 0x1e, 0xa0, 0x59,
    0x7b, 0x0c, 0xc3, 0x3c, 0x61, 0xcc, 0x40, 0x9e, 0x06, 0x52, 0x1b, 0x32,
    0x8c, 0x12, 0x93, 0xbf, 0xef, 0x3b, 0x25, 0x0d, 0xc2, 0x88, 0xd1, 0xe0,
    0x07, 0x2d, 0x70, 0xc6, 0x29, 0x6a, 0x4d, 0x47, 0x26, 0xa3, 0xe4, 0x8b,
    0xf6, 0x97, 0x2c, 0x5d, 0x3d, 0xd7, 0x96, 0x28, 0x02, 0x08, 0x30, 0xa7,
    0x22, 0xc9, 0x65, 0xf8, 0xb7, 0xb4, 0x8a, 0xca, 0xb9, 0xf2, 0xd0, 0x17,
    0xff, 0x46, 0xfb, 0x9a, 0xba, 0x8f, 0xb6, 0x69, 0x68, 0x8e, 0x21, 0x6f,
    0xc4, 0xcb, 0xb3, 0xce, 0x51, 0xd4, 0x81, 0x00, 0x2e, 0x9c, 0x74, 0x63,
    0x45, 0xd9, 0x16, 0x35, 0x5f, 0xed, 0x78, 0x9f, 0x01, 0x48, 0x04, 0xc1,
    0x33, 0xd6, 0x4f, 0x94, 0xde, 0x31, 0x9d, 0x0a, 0xac, 0x18, 0x4b, 0xcd,
    0x98, 0xb8, 0x37, 0xa2, 0x83, 0xec, 0x03, 0xd8, 0xda, 0xe5, 0x7a, 0x6b,
    0x53, 0xd5, 0x15, 0xa4, 0x43, 0xe9, 0x90, 0x67, 0x58, 0xc0, 0xa5, 0xfa,
    0x2a, 0xb1, 0x75, 0x50, 0x39, 0x5c, 0xe6, 0xdc, 0x89, 0xfc, 0xcf, 0xfe,
    0xf9, 0x57, 0x54, 0x64, 0xa8, 0xee, 0x23, 0x0b, 0xf1, 0xea, 0xfd, 0xdb,
    0xbd, 0x09, 0xb5, 0x5b, 0x05, 0x86, 0x13, 0xf3, 0x24, 0xc5, 0x3f, 0x44,
    0x72, 0x7c, 0x7e, 0x36};

// HID format table — one row per supported bit length.
// fc_start/end and cn_start/end are bit ranges within the raw Wiegand stream
// used to extract the facility code and card number. The chunk offsets are
// used by buildHexChunks() to reconstruct the hex card value with the
// standard preamble pattern. Reference: pagemac.com/projects/rfid/hid_data_formats
struct HIDFormat {
  uint16_t bit_count;
  uint8_t fc_start, fc_end;
  uint8_t cn_start, cn_end;
  uint8_t chunk1_offset;
  uint8_t bit_holder_offset;
  uint8_t chunk2_offset;
};

constexpr HIDFormat HID_FORMATS[] = {
    {26, 1, 9, 9, 25, 2, 20, 4},   {27, 1, 13, 13, 27, 3, 19, 5},
    {29, 1, 13, 13, 29, 5, 17, 7}, {30, 1, 13, 13, 29, 6, 16, 8},
    {31, 1, 5, 5, 28, 7, 15, 9},   {32, 1, 13, 13, 31, 8, 14, 10},
    {33, 1, 8, 8, 32, 9, 13, 11},  {34, 1, 17, 17, 33, 10, 12, 12},
    {35, 2, 14, 14, 34, 11, 11, 13}, {36, 21, 33, 1, 17, 12, 10, 14},
};

const HIDFormat *findHIDFormat(uint16_t bit_count) {
  for (const auto &fmt : HID_FORMATS) {
    if (fmt.bit_count == bit_count) return &fmt;
  }
  return nullptr;
}

uint32_t extractBits(const uint8_t *bits, uint16_t start, uint16_t end) {
  uint32_t value = 0;
  for (uint16_t i = start; i < end; i++) {
    value = (value << 1) | bits[i];
  }
  return value;
}

String prefixPad(const String &in, char c, size_t len) {
  String out = in;
  while (out.length() < len) {
    out = c + out;
  }
  return out;
}

String bitsToString(const uint8_t *bits, size_t bit_count) {
  String out;
  out.reserve(bit_count);
  for (size_t i = 0; i < bit_count; i++) {
    out += (bits[i] ? '1' : '0');
  }
  return out;
}

// Reconstruct the bitHolder1/bitHolder2 packed values that the original
// Wiegand ISRs maintained: the first 22 bits land in bitHolder1, the rest
// in bitHolder2.
void packBitHolders(const uint8_t *bits, size_t bit_count, uint32_t &h1,
                    uint32_t &h2) {
  h1 = 0;
  h2 = 0;
  for (size_t i = 0; i < bit_count; i++) {
    if (i + 1 < 23) {
      h1 = (h1 << 1) | bits[i];
    } else {
      h2 = (h2 << 1) | bits[i];
    }
  }
}

// Builds the two-chunk hex card value using the same scheme as the original
// firmware. See setCardChunkBits in the legacy code for the source of truth.
String buildHexChunks(const HIDFormat &fmt, uint32_t bitHolder1,
                      uint32_t bitHolder2) {
  uint32_t cardChunk1 = 0;
  uint32_t cardChunk2 = 0;

  for (int i = 19; i >= 0; i--) {
    if (i == 13 || i == fmt.chunk1_offset) {
      bitWrite(cardChunk1, i, 1);
    } else if (i > fmt.chunk1_offset) {
      bitWrite(cardChunk1, i, 0);
    } else {
      bitWrite(cardChunk1, i, bitRead(bitHolder1, i + fmt.bit_holder_offset));
    }
    if (i < fmt.bit_holder_offset) {
      bitWrite(cardChunk2, i + fmt.chunk2_offset, bitRead(bitHolder1, i));
    }
    if (i < fmt.chunk2_offset) {
      bitWrite(cardChunk2, i, bitRead(bitHolder2, i));
    }
  }

  return String(cardChunk1, HEX) + prefixPad(String(cardChunk2, HEX), '0', 6);
}

std::optional<Card> decodeHID(const uint8_t *bits, size_t bit_count) {
  const HIDFormat *fmt = findHIDFormat(bit_count);
  if (!fmt) {
    Serial.println("[-] Unsupported bitCount for HID card");
    return std::nullopt;
  }

  uint32_t h1 = 0, h2 = 0;
  packBitHolders(bits, bit_count, h1, h2);

  Card card;
  card.type = CardType::HID;
  card.bit_count = bit_count;
  card.facility_code = extractBits(bits, fmt->fc_start, fmt->fc_end);
  card.card_number = extractBits(bits, fmt->cn_start, fmt->cn_end);
  card.hex = buildHexChunks(*fmt, h1, h2);
  card.raw = bitsToString(bits, bit_count);
  return card;
}

struct GallagherFields {
  uint32_t region_code;
  uint32_t facility_code;
  uint32_t card_number;
  uint32_t issue_level;
};

GallagherFields deobfuscateGallagher(const uint8_t bytes[8]) {
  uint8_t arr[8];
  for (int i = 0; i < 8; i++) arr[i] = GALLAGHER_LUT[bytes[i]];

  return {
      static_cast<uint32_t>((arr[3] & 0x1E) >> 1),
      static_cast<uint32_t>(((arr[5] & 0x0F) << 12) | (arr[1] << 4) |
                            ((arr[7] >> 4) & 0x0F)),
      static_cast<uint32_t>((arr[0] << 16) | ((arr[4] & 0x1F) << 11) |
                            (arr[2] << 3) | ((arr[3] & 0xE0) >> 5)),
      static_cast<uint32_t>(arr[7] & 0x0F),
  };
}

// Recovers the 8 obfuscated bytes from a 96-bit Gallagher Cardax 125 kHz
// stream. Returns std::nullopt when the magic prefix is missing or the
// embedded redundancy bits don't agree.
std::optional<std::array<uint8_t, 8>>
extractGallagherBytes(const String &raw) {
  static constexpr const char *MAGIC = "01111111111010";
  int magic_idx = raw.indexOf(MAGIC);
  if (magic_idx == -1) {
    Serial.println(
        "[!] Magic prefix not found - not a valid gallagher cardax card");
    return std::nullopt;
  }

  String data = raw.substring(magic_idx + 16);
  data = data.substring(0, 9 * 8 + 8);
  if (data.length() != 9 * 8 + 8) {
    Serial.println("[!] Invalid gallagher card data length");
    return std::nullopt;
  }

  String b;
  while (b.length() < 64 + 8) {
    String n = data.substring(9 * b.length() / 8);
    if (b.length() < 64 && n.charAt(7) == n.charAt(8)) {
      Serial.println("[!] Invalid gallagher data");
      return std::nullopt;
    }
    b += n.substring(0, 8);
  }

  uint64_t value = strtoull(b.substring(0, 64).c_str(), nullptr, 2);
  std::array<uint8_t, 8> out{};
  for (int i = 0; i < 8; i++) {
    out[i] = (value >> (56 - i * 8)) & 0xFF;
  }
  return out;
}

std::optional<Card> decodeGallagher(const uint8_t *bits, size_t bit_count) {
  Card card;
  card.type = CardType::Gallagher;
  card.bit_count = bit_count;
  card.raw = bitsToString(bits, bit_count);

  auto bytes = extractGallagherBytes(card.raw);
  if (!bytes) return std::nullopt;

  for (int i = 0; i < 8; i++) {
    card.hex += String((*bytes)[i], HEX);
  }
  GallagherFields fields = deobfuscateGallagher(bytes->data());
  card.region_code = fields.region_code;
  card.facility_code = fields.facility_code;
  card.card_number = fields.card_number;
  card.issue_level = fields.issue_level;
  return card;
}

}  // namespace

std::optional<Card> decodeCard(const uint8_t *bits, size_t bit_count) {
  if (bit_count >= 26 && bit_count <= 36) {
    return decodeHID(bits, bit_count);
  }
  if (bit_count == 96) {
    return decodeGallagher(bits, bit_count);
  }
  return std::nullopt;
}
