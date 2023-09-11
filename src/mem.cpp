#include "mem.h"

#include <_types/_uint16_t.h>
#include <_types/_uint8_t.h>

uint8_t Memory::readByte(uint16_t address) { return memory[address]; }

uint16_t Memory::readWord(uint16_t address) {
  return ((uint16_t)memory[address + 1] << 8) | memory[address];
}

void Memory::writeByte(uint16_t address, uint8_t value) {
  memory[address] = value;
}

void Memory::writeWord(uint16_t address, uint16_t value) {
  memory[address] = (uint8_t)((value & 0xFF00) >> 8);
  memory[address + 1] = (uint8_t)(value & 0x00FF);
}
