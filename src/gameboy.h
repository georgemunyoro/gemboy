#pragma once

#include <_types/_uint16_t.h>
#include <_types/_uint64_t.h>
#include <_types/_uint8_t.h>
#include <sys/_types/_u_int16_t.h>

#include <cstddef>
#include <vector>

#include "cpu.h"
#include "events.h"
#include "mem.h"
#include "ppu.h"
#include "timer.h"
#include "utils.h"

const std::string BOOT_ROM_FILEPATH = "./roms/dmg_boot.bin";
const uint64_t CLOCK_CYCLE_DURATION_NANOSECONDS = 238;
const uint64_t ONE_SECOND_MICROSECONDS = 1000000000;
const uint64_t _60FPS_INTERVAL = 16666666;

#define TILESET_HEIGHT 24
#define TILESET_WIDTH 16
#define TILE_WIDTH 8

class GameBoy {
 public:
  GameBoy()
      : cpu(&memory),
        ppu(&memory),
        timer(&memory),
        tilesetDisplay("Tileset", SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_WIDTH,
                       false),
        tilemapDisplay("Tilemap", 256, 256, 1, true){};

  ~GameBoy() { SDL_Quit(); }

  void tick();
  void run();
  void reset();

  void handleInterrupts();

  void vblankInterruptHandler();
  void lcdStatInterruptHandler();
  void timerInterruptHandler();
  void serialInterruptHandler();
  void joypadInterruptHandler();

  void updateTimer();
  void loadRom(const char *filename, uint16_t addr, bool shouldUpdateRomData);
  void renderTilesetDisplay();
  void renderTilemapDisplay();
  void setEndpoint(uint16_t addr);

  const int PIXEL_WIDTH = 1;
  const int SCREEN_WIDTH = TILE_WIDTH * TILESET_WIDTH;
  const int SCREEN_HEIGHT = TILE_WIDTH * TILESET_HEIGHT;

  void addEventListener(GameboyEventType eventType,
                        GameboyEventCallback callback) {
    memory.addEventListener(eventType, callback);
  };

  bool isRunning;

 private:
  CPU cpu;
  PPU ppu;
  Memory memory;
  Timer timer;

  void loadBootRom();

  std::vector<uint8_t> romData;

  uint64_t ticks;

  SDL_Display tilesetDisplay;
  SDL_Display tilemapDisplay;

  uint16_t endpoint = 0;
};
