#pragma once

#include "card.h"
#include <cstddef>
#include <cstdint>
#include <optional>

// Decode a buffer of Wiegand bits (0/1 bytes) into a Card.
// Returns std::nullopt if the bit count or payload is not recognised.
std::optional<Card> decodeCard(const uint8_t *bits, size_t bit_count);
