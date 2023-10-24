
#include "timer.h"

#include <_types/_uint16_t.h>
#include <_types/_uint64_t.h>
#include <_types/_uint8_t.h>

#include <cassert>
#include <cstdio>

bool Timer::isRunning() { return (memory->readByte(TAC_ADDR) & 0x4) != 0; }

void Timer::run(int cycles) {
  setDividerRegister(getDividerRegister() + cycles);
  if (getDividerRegister() >= 256) {
    setDividerRegister(getDividerRegister() - 256);
    setDividerRegister(getDividerRegister() + 1);
  }

  if (isRunning()) {
    setTimerCounter(getTimerCounter() + 1);

    uint64_t frequency = getFrequency();

    while (getTimerCounter() >= (4194304 / frequency)) {
      setTimerCounter(getTimerCounter() + 1);

      if (getTimerCounter() == 0x00) {
        triggerTimerInterrupt();
        setTimerCounter(getTimerModulo());
      }

      setTimerCounter(getTimerCounter() - (4194304 / frequency));
    }
  }
}

uint64_t Timer::getFrequency() {
  int clockIndex = memory->readByte(TAC_ADDR) & 0x3;
  assert(clockIndex >= 0 && clockIndex < 4);
  return INPUT_CLOCK_SELECT_Hz_MAP[clockIndex];
}

uint8_t Timer::getTimerModulo() { return memory->readByte(TMA_ADDR); }

uint8_t Timer::getTimerCounter() { return memory->readByte(TIM_ADDR); }

uint16_t Timer::getDividerRegister() { return memory->readWord(DIV_ADDR); }

void Timer::setTimerCounter(uint8_t newValue) {
  memory->writeByte(TIM_ADDR, newValue);
}

void Timer::setDividerRegister(uint16_t newValue) {
  memory->memory[DIV_ADDR + 1] = (uint8_t)((newValue & 0xFF00) >> 8);
  memory->memory[DIV_ADDR] = (uint8_t)(newValue & 0x00FF);
}

void Timer::triggerTimerInterrupt() {
  memory->writeByte(0xFF0F, memory->readByte(0xFF0F) | 0x4);
}
