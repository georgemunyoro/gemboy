#pragma once

#include <SDL.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <_types/_uint8_t.h>

#include <string>
#include <vector>

class Display {
 public:
  Display();

  void hBlank();
  void vBlank();
  void write(uint8_t pixel);
  void render();

 private:
  std::vector<SDL_Rect> pixelBuffers[4];

  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;

  int SCREEN_HEIGHT = 512;
  int SCREEN_WIDTH = 512;
  int PIXEL_SIZE = 2;
  int X = 0;
  int Y = 0;
};
