
#include "ppu.h"

#include <_types/_uint16_t.h>
#include <_types/_uint8_t.h>

void PixelFetcher::tick() {
  ++ticks;
  if (ticks < 2) return;
  ticks = 0;

  switch (state) {
    case State::READ_TILE_ID: {
      tileId = memory->readByte(mapAddr + uint16_t(tileIndex));
      state = State::READ_TILE_DATA_0;
      break;
    }

    case State::READ_TILE_DATA_0: {
      int offset = 0x8000 + (uint16_t(tileId) * 16);
      int addr = offset + (uint16_t(tileLine) * 2);
      int data = memory->readByte(addr);
      for (unsigned int i = 0; i <= 7; ++i) {
        pixelData[i] = (data >> i) & 1;
      }

      state = State::READ_TILE_DATA_1;
      break;
    }

    case State::READ_TILE_DATA_1: {
      int offset = 0x8000 + (uint16_t(tileId) * 16);
      uint16_t addr = offset + (uint16_t(tileLine) * 2);
      uint8_t data = memory->readByte(addr + 1);
      for (unsigned int i = 0; i <= 7; ++i) {
        pixelData[i] |= ((data >> i) & 1) << 1;
      }

      state = State::PUSH_TO_FIFO;
      break;
    }

    case State::PUSH_TO_FIFO: {
      if (fifo.length() <= 8) {
        for (int i = 7; i >= 0; --i) {
          fifo.push(pixelData[i]);
        }

        ++tileIndex;
        state = State::READ_TILE_ID;
      }
      break;
    }
  }
}

void PixelFetcher::start(uint16_t mapAddr, uint8_t tileLine) {
  tileIndex = 0;
  this->mapAddr = mapAddr;
  this->tileLine = tileLine;
  state = State::READ_TILE_ID;
  fifo.clear();
}

void PPU::tick() {
  ++ticks;

  // getchar();

  switch (state) {
    case State::OAM_SCAN: {
      // In this state, the PPU would scan the OAM (Objects Attribute Memory)
      // from 0xfe00 to 0xfe9f to mix sprite pixels in the current line later.
      // This always takes 40 ticks.

      if (ticks == 40) {
        x = 0;
        uint8_t tileLine = memory->readByte(0xFF44) % 8;
        uint16_t tileMapRowAddr =
            0x9800 + (uint16_t(memory->readByte(0xFF44) / 8) * 32);

        pixelFetcher.start(tileMapRowAddr, tileLine);

        state = State::PIXEL_TRANSFER;
      }
      break;
    }

    case State::PIXEL_TRANSFER: {
      // Fetch pixel data into our pixel FIFO.
      pixelFetcher.tick();

      if (!pixelFetcher.fifo.isEmpty()) {
        uint8_t pixelColor = pixelFetcher.fifo.pop();
        display.write(pixelColor);
        ++x;
      }

      if (x == 160) {
        display.hBlank();
        state = State::H_BLANK;
      }
      break;
    }

    case State::H_BLANK: {
      // A full scanline takes 456 ticks to complete. At the end of a
      // scanline, the PPU goes back to the initial OAM Search state.
      // When we reach line 144, we switch to VBlank state instead.
      // printf("%3d H_BLANK\n", ticks);

      if (ticks == 456) {
        ticks = 0;
        ++memory->memory[0xFF44];
        if (memory->memory[0xFF44] == 144) {
          display.vBlank();
          state = State::V_BLANK;
        } else {
          state = State::OAM_SCAN;
        }
      }

      break;
    }

    case State::V_BLANK: {
      printf("%3d V_BLANK %d\n", ticks, memory->readByte(0xFF44));
      // Wait ten more scanlines before starting over.
      // getchar();
      if (ticks == 456) {
        ticks = 0;
        ++memory->memory[0xFF44];
        if (memory->memory[0xFF44] == 153) {
          memory->writeByte(0xFF44, 0);
          state = State::OAM_SCAN;
        }
      }

      break;
    }
  }
}
