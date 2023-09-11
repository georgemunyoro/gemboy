#include "utils.h"

#include <chrono>

uint64_t getTimeNanoseconds() {
  uint64_t ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();
  return ns;
}

std::pair<uint8_t, bool> addWithOverflow(uint8_t a, uint8_t b) {
  uint8_t result = a + b;
  bool overflow = result < a || result < b;
  return std::make_pair(result, overflow);
}
