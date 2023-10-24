#pragma once

#include <SDL.h>

#include <cstdio>

#include "gameboy.h"
#include "utils.h"

void runBlarggTests() {
  static const std::vector<std::string> BLARGG_ROMS = {
      "./roms/blargg/01-special.gb",
      "./roms/blargg/02-interrupts.gb",
      "./roms/blargg/03-op sp,hl.gb",
      "./roms/blargg/04-op r,imm.gb",
      "./roms/blargg/05-op rp.gb",
      "./roms/blargg/06-ld r,r.gb",
      "./roms/blargg/07-jr,jp,call,ret,rst.gb",
      "./roms/blargg/08-misc instrs.gb",
      "./roms/blargg/09-op r,r.gb",
      "./roms/blargg/10-bit ops.gb",
      "./roms/blargg/11-op a,(hl).gb",
  };

  for (std::string filename : BLARGG_ROMS) {
    GameBoy gb = GameBoy();

    std::string output = "";
    bool passed = false;
    bool done = false;

    gb.addEventListener(MEM_WRITE_BYTE, [&](GameboyEventData data) {
      if (data.memory.address == 0xFF02 && data.memory.value8 == 0x81) {
        char charToAdd = (char)data.memory.memory[0xFF01];
        output += charToAdd;
        if (charToAdd == '\n') output = "";
        if (output == "Passed") {
          done = true;
          passed = true;
        }

        if (output == "Failed") {
          done = true;
          passed = false;
        }
      }

      if (done) {
        gb.isRunning = false;
      }
    });

    gb.loadRom(filename.c_str(), 0x0000, true);
    gb.run();

    if (passed) {
      printf("✅ PASSED: %s\n", filename.c_str());
    } else {
      printf("❌ FAILED: %s\n", filename.c_str());
    }
  }
}
