#include "mem.h"

#include <_types/_uint16_t.h>
#include <_types/_uint8_t.h>

#include <cassert>
#include <cstdio>

uint8_t Memory::readByte(uint16_t address) {
  if (address >= MEM_SIZE) return 0x0aa;
  return memory[address];
}

uint16_t Memory::readWord(uint16_t address) {
  return (uint16_t)readByte(address + 1) << 8 | readByte(address);
}

void Memory::writeByte(uint16_t address, uint8_t value) {
  if (!shouldWriteToMemory) {
    mem_accesses[num_mem_accesses] =
        mem_access{MEM_ACCESS_WRITE, address, value};
    ++num_mem_accesses;
  } else {
    memory[address] = value;
  }
}

void Memory::writeWord(uint16_t address, uint16_t value) {
  writeByte(address + 1, (uint8_t)((value & 0xFF00) >> 8));
  writeByte(address, (uint8_t)(value & 0x00FF));
}
