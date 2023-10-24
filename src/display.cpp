#include "display.h"

#include <SDL_error.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <_types/_uint8_t.h>
#include <malloc/_malloc.h>

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <stdexcept>

#include "utils.h"

void Display::write(uint8_t pixel) {
  for (int i = 0; i < SCALE_FACTOR; ++i) {
    memcpy(&sdlDisplay.pixelBuffer[offset], &palette[pixel],
           sizeof(palette[pixel]));
    offset += 4;
  }
}

void Display::hBlank() {
  int scanlinePixelCount = SCREEN_WIDTH * SCALE_FACTOR * 4;
  for (int i = 0; i < scanlinePixelCount; ++i) {
    sdlDisplay.pixelBuffer[offset] =
        sdlDisplay.pixelBuffer[offset - scanlinePixelCount];
    ++offset;
  }
}

void Display::vBlank() {
  ++this->frames;
  offset = 0;
  sdlDisplay.render();
}
