#pragma once

#include <cstddef>
#include <cstdint>

namespace wiegand {

constexpr size_t MAX_BITS = 100;

void begin(int data0_pin, int data1_pin);

// Returns true once the Wiegand inter-bit timer has elapsed and at least
// one bit has been captured — i.e. the card transmission has finished.
bool ready();

const uint8_t *bits();
size_t bitCount();

// Compares the current buffer with the last buffer accepted via
// markAccepted(). Used to suppress duplicate writes for the same swipe.
bool changedSinceLastAccepted();

void markAccepted();
void reset();

}  // namespace wiegand
