#pragma once

#include <SDL_events.h>
#include <_types/_uint16_t.h>
#include <_types/_uint8_t.h>
#include <malloc/_malloc.h>

#include <cstdlib>
#include <unordered_map>
#include <vector>

#include "../lib/tester.h"
#include "events.h"
#include "utils.h"

class Memory {
 public:
  Memory() {
    if (shouldWriteToMemory && memory == NULL) memory = new uint8_t[0x10000];
  }
  ~Memory() {
    if (memory != NULL && shouldWriteToMemory) free(memory);
  }

  // Read 8-bit byte from a given address
  uint8_t readByte(uint16_t address);
  // Read 16-bit word from a given address
  uint16_t readWord(uint16_t address);

  // Write 8-bit byte to a given address
  void writeByte(uint16_t address, uint8_t value);
  void writeByte(uint16_t address, uint8_t value, bool triggerListener);

  // Write 16-bit word to a given address
  void writeWord(uint16_t address, uint16_t value);

  uint8_t* memory = NULL;

  bool shouldWriteToMemory = true;
  int num_mem_accesses;
  struct mem_access mem_accesses[16];

  size_t MEM_SIZE = 0x10000;

  void addEventListener(GameboyEventType eventType,
                        GameboyEventCallback callback) {
    listeners[eventType].push_back(callback);
  };

 private:
  GameboyEventListenerMap listeners;
};

class VRAM {
 public:
  VRAM(Memory* m) : memory(m) {}

  uint8_t getLCDC() { return memory->readByte(LCDC_ADDR); };
  uint8_t getSTAT() { return memory->readByte(STAT_ADDR); };
  uint8_t getSCY() { return memory->readByte(SCY_ADDR); };
  uint8_t getSCX() { return memory->readByte(SCX_ADDR); };
  uint8_t getLYC() { return memory->readByte(LYC_ADDR); };
  uint8_t getLY() { return memory->readByte(LY_ADDR); };
  uint8_t getWX() { return memory->readByte(WX_ADDR); };
  uint8_t getWY() { return memory->readByte(WY_ADDR); };

  int getSpriteSize() { return 0; };

 private:
  Memory* memory;

  static const uint16_t LCDC_ADDR = 0xFF40;
  static const uint16_t STAT_ADDR = 0xFF41;
  static const uint16_t SCY_ADDR = 0xFF42;
  static const uint16_t SCX_ADDR = 0xFF43;
  static const uint16_t LYC_ADDR = 0xFF45;
  static const uint16_t LY_ADDR = 0xFF44;
  static const uint16_t WX_ADDR = 0xFF4A;
  static const uint16_t WY_ADDR = 0xFF4B;
};
