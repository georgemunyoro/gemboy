#pragma once

#include <_types/_uint8_t.h>

#include <cstddef>
#include <deque>
#include <vector>

#include "display.h"
#include "mem.h"

class PixelFIFO {
 public:
  void push(uint8_t value) { values.push_back(value); }
  uint8_t pop() {
    uint8_t returnValue = values[0];
    values.pop_front();
    return returnValue;
  }
  size_t length() { return values.size() / sizeof(uint8_t); }
  bool isEmpty() { return length() == 0; }
  void clear() { values.clear(); };

 private:
  std::deque<uint8_t> values;
};

class PixelFetcher {
 public:
  PixelFetcher(Memory* m) : memory(m){};

  enum class State {
    READ_TILE_ID,
    READ_TILE_DATA_0,
    READ_TILE_DATA_1,
    PUSH_TO_FIFO
  };

  void start(uint16_t tileMapRowAddr, uint8_t tileLine);
  void tick();

  PixelFIFO fifo;

 private:
  State state;
  int ticks;
  Memory* memory;
  uint16_t mapAddr;
  uint8_t tileLine;
  int tileIndex;
  int tileId;
  uint8_t pixelData[8];
};

class PPU {
 public:
  PPU(Memory* m) : memory(m), pixelFetcher(m) {
    // LCDC = m->memory + 0xFF40;
    // STAT = m->memory + 0xFF41;

    // SCY = m->memory + 0xFF42;
    // SCX = m->memory + 0xFF43;

    LY = m->memory + 0xFF44;
    // LYC = m->memory + 0xFF45;
    // WX = m->memory + 0xFF4A;
    // WY = m->memory + 0xFF4B;
  };

  enum class State { OAM_SCAN, PIXEL_TRANSFER, H_BLANK, V_BLANK };

  // uint8_t* WX;
  // uint8_t* WY;
  uint8_t* LY;

  // uint8_t* LYC;
  // uint8_t* SCY;
  // uint8_t* SCX;

  // uint8_t* STAT;
  // uint8_t* LCDC;

  uint8_t x;
  int ticks;

  State state;

  void tick();
  Display display;

 private:
  Memory* memory;
  PixelFetcher pixelFetcher;
};
