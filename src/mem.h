#pragma once

#include <_types/_uint16_t.h>
#include <_types/_uint8_t.h>

#include <cstdlib>

class Memory {
 public:
  // Read 8-bit byte from a given address
  uint8_t readByte(uint16_t address);
  // Read 16-bit word from a given address
  uint16_t readWord(uint16_t address);

  // Write 8-bit byte to a given address
  void writeByte(uint16_t address, uint8_t value);
  // Write 16-bit word to a given address
  void writeWord(uint16_t address, uint16_t value);

  uint8_t memory[0xFFFF];

 private:
};
