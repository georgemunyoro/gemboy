#pragma once

#include <_types/_uint64_t.h>
#include <_types/_uint8_t.h>

#include <utility>

uint64_t getTimeNanoseconds();

std::pair<uint8_t, bool> addWithOverflow(uint8_t a, uint8_t b);