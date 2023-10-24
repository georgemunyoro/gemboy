#pragma once

#include <SDL.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <_types/_uint8_t.h>
#include <malloc/_malloc.h>

#include <cstddef>
#include <string>
#include <vector>

#include "utils.h"

class Display {
 public:
  Display()
      : sdlDisplay("gameboy", this->SCREEN_WIDTH, this->SCREEN_HEIGHT,
                   this->SCALE_FACTOR, false) {}

  void hBlank();
  void vBlank();
  void write(uint8_t pixel);

  uint64_t frames;

  SDL_Color palette[4] = {
      {0xe0, 0xf0, 0xe7, 0xff},  // White
      {0x8b, 0xa3, 0x94, 0xff},  // Light gray
      {0x55, 0x64, 0x5a, 0xff},  // Dark gray
      {0x34, 0x3d, 0x37, 0xff},  // Black
  };

 private:
  int offset = 0;
  SDL_Display sdlDisplay;

  int X = 0;
  int Y = 0;

  static const int SCREEN_WIDTH = 160;
  static const int SCREEN_HEIGHT = 144;
  static const int SCALE_FACTOR = 2;
};
