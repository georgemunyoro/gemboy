#include <iostream>

#include "gameboy.h"

int main(int argc, char *argv[]) {
  GameBoy gb = GameBoy();

  if (argc != 2) {
    printf(
        "Running without ROM! Correct usage is:\n\tgameboy <ROM "
        "filepath>\n");

  } else {
    gb.loadRom(argv[1], 0x0000, true);
  }

  gb.run();
}
