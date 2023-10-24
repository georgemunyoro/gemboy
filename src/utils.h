#pragma once

#include <SDL.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <_types/_uint16_t.h>
#include <_types/_uint64_t.h>
#include <_types/_uint8_t.h>

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

uint64_t getTimeNanoseconds();

std::pair<uint8_t, bool> addWithOverflow(uint8_t a, uint8_t b);

class SDL_Display {
 public:
  SDL_Display(char *name, int displayWidth, int displayHeight, int scaleFactor,
              bool hidden);
  ~SDL_Display();

  int displayWidth;
  int displayHeight;
  int scaleFactor;

  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  uint8_t *pixelBuffer;

  void render();
};
