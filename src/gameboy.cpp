#include "gameboy.h"

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <_types/_uint16_t.h>
#include <_types/_uint64_t.h>
#include <_types/_uint8_t.h>
#include <string.h>

#include <bitset>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "cpu.h"
#include "display.h"
#include "utils.h"

enum Pixel { WHITE, LIGHT_GRAY, DARK_GRAY, BLACK };

void GameBoy::tick() {
  ++ticks;
  if ((ticks % 4) == 0) cpu.tick();
  if ((cpu.memory->readByte(0xFF40) & 0x80) != 0) ppu.tick();
  handleInterrupts();
}

// Check if any interrupt flags are set and handle them accordingly.
void GameBoy::handleInterrupts() {
  uint8_t interruptsFired = memory.readByte(0xFFFF) & memory.readByte(0xFF0F);

  if (cpu.IME && interruptsFired > 0) {
    if (interruptsFired & 0x01) {
      // printf("V_BLANK INTERRUPT\n");
      memory.memory[0xFF0F] &= (0xFF - 0x01);
      return vblankInterruptHandler();
    }

    if (interruptsFired & 0x02) {
      // printf("LCD_STAT INTERRUPT\n");
      memory.memory[0xFF0F] &= (0xFF - 0x02);
      return lcdStatInterruptHandler();
    }

    if (interruptsFired & 0x04) {
      // printf("TIMER INTERRUPT\n");
      memory.memory[0xFF0F] &= (0xFF - 0x04);
      return timerInterruptHandler();
    }

    if (interruptsFired & 0x08) {
      // printf("SERIAL INTERRUPT\n");
      memory.memory[0xFF0F] &= (0xFF - 0x08);
      return serialInterruptHandler();
    }

    if (interruptsFired & 0x10) {
      // printf("JOYPAD INTERRUPT\n");
      memory.memory[0xFF0F] &= (0xFF - 0x10);
      return joypadInterruptHandler();
    }
  }
}

void GameBoy::vblankInterruptHandler() {
  cpu.IME = false;
  cpu.push(cpu.PC);
  cpu.setPC(0x0040);
}

void GameBoy::lcdStatInterruptHandler() {
  cpu.IME = false;
  cpu.push(cpu.PC);
  cpu.setPC(0x0048);
}

void GameBoy::timerInterruptHandler() {
  cpu.IME = false;
  cpu.push(cpu.PC);
  cpu.setPC(0x0050);
}

void GameBoy::serialInterruptHandler() {
  cpu.IME = false;
  cpu.push(cpu.PC);
  cpu.setPC(0x0058);
}

void GameBoy::joypadInterruptHandler() {
  cpu.IME = false;
  cpu.push(cpu.PC);
  cpu.setPC(0x0060);
}

void GameBoy::run() {
  loadBootRom();
  isRunning = true;
  ticks = 0;

  SDL_Event e;
  while (SDL_PollEvent(&e))
    if (e.type == SDL_QUIT) isRunning = false;

  uint64_t tickInterval = getTimeNanoseconds();
  uint64_t ioInterval = tickInterval;
  uint64_t renderTileDisplayInterval = tickInterval;

  while (isRunning) {
    uint64_t currentTime = getTimeNanoseconds();

    if ((currentTime - tickInterval) >=
        (CLOCK_CYCLE_DURATION_NANOSECONDS - 50)) {
      tick();
      tickInterval = currentTime;
    }

    if ((currentTime - ioInterval) >= ONE_SECOND_MICROSECONDS) {
      // printf("FPS: %llu\n", ppu.display.frames);
      ppu.display.frames = 0;
      ioInterval = currentTime;
    }

    if ((currentTime - renderTileDisplayInterval) >= _60FPS_INTERVAL &&
        cpu.PC > 0x0100) {
      renderTilesetDisplay();
      renderTilemapDisplay();
      renderTileDisplayInterval = currentTime;
    }

    // timer.run(cpu.cycles);

    // printf("%04X : %02X %02X %02X %02X\n", cpu.getPC(),
    // memory.readByte(0xFF04),
    //        memory.readByte(0xFF05), memory.readByte(0xFF06),
    //        memory.readByte(0xFF07));
    // getchar();

    cpu.cycles = 0;

    if (endpoint != 0 && cpu.PC == endpoint) break;
  }
}

void GameBoy::renderTilemapDisplay() {
  for (int tileRow = 0; tileRow < 32; ++tileRow) {
    for (int tileCol = 0; tileCol < 32; ++tileCol) {
      int tileNumber = tileRow * 32 + tileCol;
      uint16_t addr = 0x9C00 + (tileNumber * 16);
      int tileOffset = tileRow * (256 * 8 * 4) + tileCol * 32;

      for (int tilePixelRow = 0; tilePixelRow < 8; ++tilePixelRow) {
        uint8_t byte1 = memory.memory[addr + tilePixelRow * 2];
        uint8_t byte2 = memory.memory[addr + tilePixelRow * 2 + 1];

        for (int tilePixelCol = 0; tilePixelCol < 8; ++tilePixelCol) {
          uint8_t mask = 1 << (7 - tilePixelCol);
          uint8_t lsb = byte1 & mask;
          uint8_t msb = byte2 & mask;

          int pixel = 0 | (lsb != 0 ? 1 : 0) << 1 | (msb != 0 ? 1 : 0);

          int pixelOffset =
              tileOffset + (tilePixelRow * 256 * 4) + (tilePixelCol * 4);

          memcpy(&tilemapDisplay.pixelBuffer[pixelOffset],
                 &ppu.display.palette[pixel], sizeof(SDL_Color));
        }
      }
    }
  }
  tilemapDisplay.render();
}

void GameBoy::renderTilesetDisplay() {
  for (int k = 0; k < 384; ++k) {
    uint16_t addr = 0x8000 + (k * 16);

    for (int row = 0; row < 8; ++row) {
      uint8_t byte1 = memory.readByte(addr + (row * 2));
      uint8_t byte2 = memory.readByte(addr + (row * 2) + 1);

      for (int col = 0; col < 8; ++col) {
        uint8_t mask = 1 << (7 - col);
        uint8_t lsb = byte1 & mask;
        uint8_t msb = byte2 & mask;

        SDL_Color pixel =
            ppu.display
                .palette[0 | (lsb != 0 ? 1 : 0) << 1 | (msb != 0 ? 1 : 0)];

        int offset = ((int)(k / 16) * 512 * 8) + (512 * row) +
                     ((k % 16) * 8 * 4) + (4 * col);

        memcpy(&tilesetDisplay.pixelBuffer[offset], &pixel, sizeof(SDL_Color));
      }
    }
  }

  tilesetDisplay.render();
}

// Load rom file into memory starting at the address referenced by the
// current PC
void GameBoy::loadRom(const char* filename, uint16_t addr,
                      bool shouldUpdateRomData) {
  std::fstream romFile(filename);

  if (!romFile.is_open()) {
    std::cerr << "Failed to read rom file from : " << BOOT_ROM_FILEPATH
              << std::endl;
    exit(1);
  }

  cpu.setPC(addr);
  while (romFile) {
    memory.writeByte(cpu.getPC(), romFile.get());
    if (shouldUpdateRomData) romData.push_back(memory.readByte(cpu.getPC()));
    // printf("0x%04X: 0x%02X\n", cpu.getPC(),
    // memory.readByte(cpu.getPC()));
    cpu.incrementPC();
  }

  if (shouldUpdateRomData) {
    cpu.setRomData(romData);
  }

  romFile.close();
  // printf("Loaded rom file: %s\n", filename);
}

void GameBoy::loadBootRom() {
  loadRom(BOOT_ROM_FILEPATH.c_str(), 0x0000, false);
  cpu.setPC(0x0000);

  // printf("Loaded boot rom file.\n");
}

void GameBoy::setEndpoint(uint16_t addr) { endpoint = addr; }
