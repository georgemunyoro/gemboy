#pragma once

#include <_types/_uint16_t.h>
#include <_types/_uint64_t.h>
#include <_types/_uint8_t.h>
#include <sys/_types/_u_int16_t.h>

#include <vector>

#include "cpu.h"
#include "mem.h"
#include "ppu.h"

const std::string BOOT_ROM_FILEPATH = "./roms/dmg_boot.bin";
const uint64_t CLOCK_CYCLE_DURATION_NANOSECONDS = 238;
const uint64_t ONE_SECOND_MICROSECONDS = 1000000000;

class GameBoy {
 public:
  GameBoy() : cpu(&memory), ppu(&memory){};

  void run();
  void reset();

  void loadRom(const char *filename, uint16_t addr, bool shouldUpdateRomData);

 private:
  CPU cpu;
  PPU ppu;
  Memory memory;
  bool framebuffer[160][144];

  void loadBootRom();

  std::vector<uint8_t> romData;
};
