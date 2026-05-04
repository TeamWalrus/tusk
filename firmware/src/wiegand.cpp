#include "wiegand.h"

#include <Arduino.h>
#include <cstring>

namespace wiegand {
namespace {

// Time, in main-loop iterations, to wait between bits before declaring a
// card transmission complete.
constexpr unsigned int WIEGAND_WAIT_TIME = 3000;

volatile uint8_t databits[MAX_BITS];
volatile size_t bit_count = 0;
volatile uint8_t flag_done = 1;
volatile unsigned int weigand_counter = WIEGAND_WAIT_TIME;

uint8_t last_accepted_bits[MAX_BITS];
size_t last_accepted_count = 0;

void IRAM_ATTR isr_data0() {
  if (bit_count < MAX_BITS) {
    bit_count++;
  }
  flag_done = 0;
  weigand_counter = WIEGAND_WAIT_TIME;
}

void IRAM_ATTR isr_data1() {
  if (bit_count < MAX_BITS) {
    databits[bit_count] = 1;
    bit_count++;
  }
  flag_done = 0;
  weigand_counter = WIEGAND_WAIT_TIME;
}

}  // namespace

void begin(int data0_pin, int data1_pin) {
  pinMode(data0_pin, INPUT);
  pinMode(data1_pin, INPUT);
  attachInterrupt(data0_pin, isr_data0, FALLING);
  attachInterrupt(data1_pin, isr_data1, FALLING);
  weigand_counter = WIEGAND_WAIT_TIME;
  std::memset(last_accepted_bits, 0, sizeof(last_accepted_bits));
}

bool ready() {
  if (!flag_done) {
    if (--weigand_counter == 0) flag_done = 1;
  }
  return bit_count > 0 && flag_done;
}

const uint8_t *bits() { return const_cast<const uint8_t *>(databits); }
size_t bitCount() { return bit_count; }

bool changedSinceLastAccepted() {
  if (bit_count != last_accepted_count) return true;
  for (size_t i = 0; i < bit_count; i++) {
    if (databits[i] != last_accepted_bits[i]) return true;
  }
  return false;
}

void markAccepted() {
  last_accepted_count = bit_count;
  for (size_t i = 0; i < bit_count; i++) {
    last_accepted_bits[i] = databits[i];
  }
}

void reset() {
  for (size_t i = 0; i < MAX_BITS; i++) databits[i] = 0;
  bit_count = 0;
  flag_done = 1;
  weigand_counter = WIEGAND_WAIT_TIME;
}

}  // namespace wiegand
