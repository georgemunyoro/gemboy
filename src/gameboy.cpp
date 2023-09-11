#include "gameboy.h"

#include <SDL.h>
#include <SDL_render.h>
#include <SDL_surface.h>
#include <SDL_video.h>
#include <_types/_uint16_t.h>
#include <_types/_uint64_t.h>
#include <_types/_uint8_t.h>

#include <bitset>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "cpu.h"
#include "utils.h"

const int PIXEL_WIDTH = 2;
const int SCREEN_WIDTH = PIXEL_WIDTH * 8 * 16;
const int SCREEN_HEIGHT = PIXEL_WIDTH * 8 * 24;

enum Pixel { WHITE, LIGHT_GRAY, DARK_GRAY, BLACK };

void GameBoy::run() {
  uint8_t nintendoLogo[48] = {
      0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73, 0x00, 0x83,
      0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e,
      0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63,
      0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e};

  for (int i = 0; i < 48; ++i) {
    assert(memory.readByte(0x104 + i) == nintendoLogo[i]);
    // memory.writeByte(0x104 + i, nintendoLogo[i]);
  }

  loadBootRom();

  uint64_t clockInterval = getTimeNanoseconds();
  uint64_t renderInterval = clockInterval;

  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;

  std::vector<SDL_Rect> blackRectangles;
  std::vector<SDL_Rect> whiteRectangles;
  std::vector<SDL_Rect> lgrayRectangles;
  std::vector<SDL_Rect> dgrayRectangles;
  std::vector<SDL_Rect> sepRectangles;

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
  else {
    window = SDL_CreateWindow("gameboy", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
      printf("Failed to create SDL window! SDL Error: %s\n", SDL_GetError());
    else {
      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

      SDL_Event e;
      bool quit = false;

      bool shouldPause = false;

      while (quit == false) {
        while (SDL_PollEvent(&e)) {
          if (e.type == SDL_QUIT) quit = true;
        }

        if (cpu.getPC() == 0x0100) shouldPause = true;
        if (shouldPause) getchar();

        // getchar();

        uint64_t currentTime = getTimeNanoseconds();
        uint64_t timeElapsed = currentTime - clockInterval;

        // Fetch opcode
        uint8_t opcode = memory.readByte(cpu.getPC());
        bool isPrefixed = opcode == 0xCB;
        if (isPrefixed) opcode = memory.readByte(cpu.getPC() + 1);

        // printf("0x%04X: 0x%02X", cpu.getPC(), opcode);
        // if (isPrefixed) printf(" (PREFIXED)");
        // printf("\n");

        // Decode opcode
        Instruction instruction = Instruction(opcode, isPrefixed);

        // 4194304 Hz
        if (timeElapsed >= (CLOCK_CYCLE_DURATION_NANOSECONDS * 100)) {
          // Execute opcode
          cpu.executeInstruction(&instruction);
          clockInterval = currentTime;

          ppu.tick();
        }

        if ((currentTime - renderInterval) >= 16000000) {
          ppu.display.render();

          blackRectangles.clear();
          whiteRectangles.clear();
          lgrayRectangles.clear();
          dgrayRectangles.clear();

          int xOffset = 0;
          int yOffset = 0;

          SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
          SDL_RenderClear(renderer);

          // for (int addr = 0x8000; addr <= 0x9FFF; addr += 0x10) {
          for (int k = 0; k < 384; ++k) {
            uint16_t addr = 0x8000 + (k * 16);
            Pixel tile[8][8];

            for (int i = 0; i < 8; ++i) {
              uint8_t byte1 = memory.readByte(addr + (i * 2));
              uint8_t byte2 = memory.readByte(addr + (i * 2) + 1);

              for (int p_index = 0; p_index < 8; ++p_index) {
                uint8_t mask = 1 << (7 - p_index);
                uint8_t lsb = byte1 & mask;
                uint8_t msb = byte2 & mask;

                if (lsb != 0 && msb != 0) tile[i][p_index] = Pixel::BLACK;
                if (lsb == 0 && msb != 0) tile[i][p_index] = Pixel::DARK_GRAY;
                if (lsb != 0 && msb == 0) tile[i][p_index] = Pixel::LIGHT_GRAY;
                if (lsb == 0 && msb == 0) tile[i][p_index] = Pixel::WHITE;
              }
            }

            for (int row = 0; row < 8; ++row) {
              for (int col = 0; col < 8; ++col) {
                Pixel p = tile[row][col];

                SDL_Rect r;
                r.h = PIXEL_WIDTH;
                r.w = PIXEL_WIDTH;
                r.x = xOffset + col * PIXEL_WIDTH;
                r.y = yOffset + row * PIXEL_WIDTH;

                switch (p) {
                  case Pixel::WHITE:
                    whiteRectangles.push_back(r);
                    break;

                  case Pixel::LIGHT_GRAY:
                    lgrayRectangles.push_back(r);
                    break;

                  case Pixel::DARK_GRAY:
                    dgrayRectangles.push_back(r);
                    break;

                  case Pixel::BLACK:
                    blackRectangles.push_back(r);
                    break;
                }
              }

              if ((sepRectangles.size() / sizeof(SDL_Rect)) <=
                  ((SCREEN_WIDTH / (PIXEL_WIDTH * 8)) *
                   (SCREEN_HEIGHT / (PIXEL_WIDTH * 8)))) {
                SDL_Rect rowSep;
                rowSep.h = 1;
                rowSep.w = SCREEN_WIDTH;
                rowSep.x = 0;
                rowSep.y = yOffset;

                sepRectangles.push_back(rowSep);

                SDL_Rect colSep;
                colSep.h = SCREEN_HEIGHT;
                colSep.w = 1;
                colSep.x = xOffset;
                colSep.y = 0;

                sepRectangles.push_back(colSep);
              }
            }

            xOffset += PIXEL_WIDTH * 8;
            if (xOffset == SCREEN_WIDTH) {
              yOffset += PIXEL_WIDTH * 8;
              xOffset = 0;
            }
          }

          SDL_SetRenderDrawColor(renderer, 154, 160, 166, 255);
          SDL_RenderFillRects(renderer, lgrayRectangles.data(),
                              lgrayRectangles.size());

          SDL_SetRenderDrawColor(renderer, 60, 64, 67, 255);
          SDL_RenderFillRects(renderer, dgrayRectangles.data(),
                              dgrayRectangles.size());

          SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
          SDL_RenderFillRects(renderer, whiteRectangles.data(),
                              whiteRectangles.size());

          SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
          SDL_RenderFillRects(renderer, blackRectangles.data(),
                              blackRectangles.size());

          SDL_RenderFillRects(renderer, sepRectangles.data(),
                              sepRectangles.size());

          SDL_RenderPresent(renderer);

          renderInterval = currentTime;
        }
      }
    }
  }
}

// Load rom file into memory starting at the address referenced by the
// current PC
void GameBoy::loadRom(const char *filename, uint16_t addr,
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
  printf("Loaded rom file: %s\n", filename);
}

void GameBoy::loadBootRom() {
  loadRom(BOOT_ROM_FILEPATH.c_str(), 0x0000, false);
  cpu.setPC(0x0000);

  printf("Loaded boot rom file.\n");
}
