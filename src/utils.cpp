#include "utils.h"

#include <SDL_video.h>

#include <chrono>

uint64_t getTimeNanoseconds() {
  uint64_t ns =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();
  return ns;
}

std::pair<uint8_t, bool> addWithOverflow(uint8_t a, uint8_t b) {
  uint8_t result = a + b;
  bool overflow = result < a || result < b;
  return std::make_pair(result, overflow);
}

SDL_Display::SDL_Display(char *name, int displayWidth, int displayHeight,
                         int scaleFactor, bool hidden) {
  this->displayHeight = displayHeight;
  this->displayWidth = displayWidth;
  this->scaleFactor = scaleFactor;

  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    throw std::runtime_error("Failed to initialize SDL! SDL Error: " +
                             std::string(SDL_GetError()));

  window = SDL_CreateWindow(name, 100, 100, displayWidth * scaleFactor,
                            displayHeight * scaleFactor,
                            hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN);

  if (window == NULL)
    throw std::runtime_error("Failed to create SDL window! SDL Error: " +
                             std::string(SDL_GetError()));

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  if (renderer == NULL)
    throw std::runtime_error("Failed to create SDL renderer! SDL Error: " +
                             std::string(SDL_GetError()));

  texture = SDL_CreateTexture(
      renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
      displayWidth * scaleFactor, displayHeight * scaleFactor);

  pixelBuffer =
      (uint8_t *)malloc(sizeof(SDL_Color) * (displayWidth * scaleFactor) *
                        (displayHeight * scaleFactor));
};

SDL_Display::~SDL_Display() {
  if (pixelBuffer != NULL) free(pixelBuffer);
}

void SDL_Display::render() {
  SDL_UpdateTexture(texture, NULL, pixelBuffer, displayWidth * scaleFactor * 4);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}
