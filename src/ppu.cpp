
#include "ppu.h"

#include <_types/_uint16_t.h>
#include <_types/_uint8_t.h>
#include <sys/_types/_int16_t.h>
#include <sys/_types/_int8_t.h>

#include <bitset>

#include "mem.h"

void PixelFetcher::readTileLine(uint8_t bitPlane, uint16_t tileDataAddr,
                                uint8_t tileId, bool signedId, uint8_t tileLine,
                                uint flags, uint8_t *data[8]) {
  uint16_t offset =
      signedId ? uint16_t(int16_t(tileDataAddr) + int16_t(int8_t(tileId)) * 16)
               : tileDataAddr + (uint16_t(tileId) * 16);

  if ((flags & spriteFlipY) != 0) {
    auto height =
        uint8(8 << ((vram.getLCDC() & vram.getSpriteSize()) >> 2) - 1);
    auto tileLine = height - tileLine;
  }
}

void PixelFetcher::tick() {
  ++ticks;

  if (ticks < 2) return;

  ticks = 0;

  /*
    TILE_SEL	VRAM Range
    0	$8800 - $97FF
    1	$8000 - $8FFF (OBJ area)
  */
  std::bitset<8> LCDC = std::bitset<8>(memory->readByte(0xFF40));
  int TILE_SEL = LCDC.test(4);
  uint16_t vramRange = TILE_SEL == 0 ? 0x8800 : 0x8000;

  uint8_t LY = memory->memory[0xFF44];
  uint8_t LX = tileIndex;
  uint8_t SCY = memory->memory[0xFF42];
  uint8_t SCX = memory->memory[0xFF43];

  switch (state) {
    case State::READ_TILE_ID: {
      tileId = memory->readByte(mapAddr + uint16_t(tileIndex));
      state = State::READ_TILE_DATA_0;
      break;
    }

    case State::READ_TILE_DATA_0: {
      int offset = vramRange + (uint16_t(tileId) * 16);
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

  if (memory->readByte(0xFF44) == memory->readByte(0xFF45)) {
    memory->writeByte(0xFF0F, memory->readByte(0xFF0F) | 0x2);
  }

  switch (state) {
    case State::OAM_SCAN: {
      // In this state, the PPU would scan the OAM (Objects Attribute Memory)
      // from 0xfe00 to 0xfe9f to mix sprite pixels in the current line later.
      // This always takes 40 ticks.

      if (ticks == 40) {
        x = 0;
        int y = memory->readByte(0xFF42) + memory->readByte(0xFF44);

        uint8_t tileLine = y % 8;
        uint16_t tileMapRowAddr = 0x9800 + (uint16_t((y % 256) / 8) * 32);

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

        auto paletteColor =
            (memory->readByte(0xFF47) >> ((uint8_t)pixelColor * 2)) & 3;

        display.write(paletteColor);
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

          memory->writeByte(0xFF0F, memory->readByte(0xFF0F) | 0x1);

          state = State::V_BLANK;
        } else {
          state = State::OAM_SCAN;
        }
      }

      break;
    }

    case State::V_BLANK: {
      // printf("%3d V_BLANK %d\n", ticks, memory->readByte(0xFF44));
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
