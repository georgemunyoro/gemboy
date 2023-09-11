#include "display.h"

#include <_types/_uint8_t.h>

#include <cstdio>

Display::Display() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return;
  }

  window = SDL_CreateWindow("gameboy", 100, 100, this->SCREEN_WIDTH,
                            this->SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

  if (window == NULL) {
    printf("Failed to create SDL window! SDL Error: %s\n", SDL_GetError());
    return;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

void Display::render() {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderFillRects(renderer, pixelBuffers[0].data(), pixelBuffers[0].size());

  SDL_SetRenderDrawColor(renderer, 154, 160, 166, 255);
  SDL_RenderFillRects(renderer, pixelBuffers[1].data(), pixelBuffers[1].size());

  SDL_SetRenderDrawColor(renderer, 60, 64, 67, 255);
  SDL_RenderFillRects(renderer, pixelBuffers[2].data(), pixelBuffers[2].size());

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderFillRects(renderer, pixelBuffers[3].data(), pixelBuffers[3].size());

  SDL_RenderPresent(renderer);
}

void Display::write(uint8_t pixel) {
  SDL_Rect r;
  r.h = PIXEL_SIZE;
  r.w = PIXEL_SIZE;
  r.x = X;
  r.y = Y;

  pixelBuffers[pixel].push_back(r);

  X += PIXEL_SIZE;
}

void Display::hBlank() {
  X = 0;
  Y += PIXEL_SIZE;
}

void Display::vBlank() {
  Y += PIXEL_SIZE;
  for (int i = 0; i < 4; ++i) {
    pixelBuffers[i].clear();
  }
}
