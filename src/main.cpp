#include <iostream>

#include "gameboy.h"
#include "test.h"

// #define TEST

int main(int argc, char *argv[]) {
#ifdef TEST
  runBlarggTests();
#endif

#ifndef TEST
  GameBoy gb = GameBoy();

  if (argc != 2)
    printf(
        "Running without ROM! Correct usage is:\n\tgameboy <ROM "
        "filepath>\n");

  else
    gb.loadRom(argv[1], 0x0000, true);

  gb.run();
#endif
}
