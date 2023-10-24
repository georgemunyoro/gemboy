#pragma once

#include <_types/_uint16_t.h>
#include <_types/_uint8_t.h>

#include <functional>
#include <unordered_map>

enum GameboyEventType {
  MEM_READ_BYTE,
  MEM_READ_WORD,
  MEM_WRITE_BYTE,
  MEM_WRITE_WORD
};

enum GameboyMemoryEventType { READ, WRITE };

struct GameboyMemoryEventData {
  uint16_t address;
  uint8_t value8;
  uint16_t value16;
  uint8_t *memory;
};

struct GameboyEventData {
  GameboyMemoryEventData memory;
};

typedef std::function<void(GameboyEventData data)> GameboyEventCallback;

typedef std::unordered_map<GameboyEventType, std::vector<GameboyEventCallback>>
    GameboyEventListenerMap;
