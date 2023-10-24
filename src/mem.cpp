#include "mem.h"

#include <_types/_uint16_t.h>
#include <_types/_uint8_t.h>

#include <cassert>
#include <cstdio>

#include "events.h"
#include "utils.h"

uint8_t Memory::readByte(uint16_t address) {
  if (address >= MEM_SIZE) return 0x0aa;
  uint8_t value = memory[address];

  if (listeners[GameboyEventType::MEM_READ_BYTE].size() > 0) {
    for (auto callback : listeners[GameboyEventType::MEM_READ_BYTE])
      callback({.memory = {address, value, 0, memory}});
  }

  return value;
}

uint16_t Memory::readWord(uint16_t address) {
  uint16_t value = (uint16_t)readByte(address + 1) << 8 | readByte(address);

  if (listeners[GameboyEventType::MEM_READ_WORD].size() > 0) {
    for (auto callback : listeners[GameboyEventType::MEM_READ_WORD])
      callback({.memory = {address, 0, value, memory}});
  }

  return value;
}

void Memory::writeByte(uint16_t address, uint8_t value) {
  writeByte(address, value, true);
}

void Memory::writeByte(uint16_t address, uint8_t value, bool triggerListener) {
  if (address == 0xFF04) value = 0;

  if (triggerListener &&
      listeners[GameboyEventType::MEM_WRITE_BYTE].size() > 0) {
    for (auto callback : listeners[GameboyEventType::MEM_WRITE_BYTE])
      callback({.memory = {address, value, 0, memory}});
  }

  if (!shouldWriteToMemory) {
    mem_accesses[num_mem_accesses] =
        mem_access{MEM_ACCESS_WRITE, address, value};
    ++num_mem_accesses;
  } else {
    memory[address] = value;
  }
}

void Memory::writeWord(uint16_t address, uint16_t value) {
  if (address == 0xFF04) value = 0;

  if (listeners[GameboyEventType::MEM_WRITE_WORD].size() > 0) {
    for (auto callback : listeners[GameboyEventType::MEM_WRITE_WORD])
      callback({.memory = {address, 0, value, memory}});
  }

  writeByte(address + 1, (uint8_t)((value & 0xFF00) >> 8), false);
  writeByte(address, (uint8_t)(value & 0x00FF), false);
}
