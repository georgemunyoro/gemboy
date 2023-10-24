#pragma once

#include <_types/_uint64_t.h>
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
  PixelFetcher(Memory* m) : memory(m), vram(m){};

  enum class State {
    READ_TILE_ID,
    READ_TILE_DATA_0,
    READ_TILE_DATA_1,
    PUSH_TO_FIFO
  };

  PixelFIFO fifo;

  void start(uint16_t tileMapRowAddr, uint8_t tileLine);
  void tick();
  void readTileLine(uint8_t bitPlane, uint16_t tileDataAddr, uint8_t tileId,
                    bool signedId, uint8_t tileLine, uint flags,
                    uint8_t* data[8]);

 private:
  State state;
  int ticks;
  Memory* memory;
  uint16_t mapAddr;
  uint8_t tileLine;
  int tileIndex;
  int tileId;
  uint8_t pixelData[8];
  uint8_t LX;
  VRAM vram;
};

class PPU {
 public:
  PPU(Memory* m)
      : memory(m),
        pixelFetcher(m),
        vram(m){

        };

  enum class State { OAM_SCAN, PIXEL_TRANSFER, H_BLANK, V_BLANK };

  uint8_t x;
  int ticks;

  State state;

  void tick();
  Display display;

  VRAM vram;

 private:
  Memory* memory;
  PixelFetcher pixelFetcher;
};
