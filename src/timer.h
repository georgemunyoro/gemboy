#pragma once

#include <_types/_uint16_t.h>
#include <_types/_uint8_t.h>

#include "mem.h"

class Timer {
  /*
      FF04 - DIV - Divider Register (R/W)
      This register is incremented at rate of 16384Hz (~16779Hz on SGB). In CGB
     Double Speed Mode it is incremented twice as fast, ie. at 32768Hz. Writing
     any value to this register resets it to 00h.

      FF05 - TIMA - Timer counter (R/W)
      This timer is incremented by a clock frequency specified by the TAC
     register ($FF07). When the value overflows (gets bigger than FFh) then it
     will be reset to the value specified in TMA (FF06), and an interrupt will
     be requested, as described below.

      FF06 - TMA - Timer Modulo (R/W)
      When the TIMA overflows, this data will be loaded.

      FF07 - TAC - Timer Control (R/W)
        Bit 2    - Timer Stop  (0=Stop, 1=Start)
        Bits 1-0 - Input Clock Select
                   00:   4096 Hz    (~4194 Hz SGB)
                   01: 262144 Hz  (~268400 Hz SGB)
                   10:  65536 Hz   (~67110 Hz SGB)
                   11:  16384 Hz   (~16780 Hz SGB)

      INT 50 - Timer Interrupt
      Each time when the timer overflows (ie. when TIMA gets bigger than FFh),
     then an interrupt is requested by setting Bit 2 in the IF Register (FF0F).
     When that interrupt is enabled, then the CPU will execute it by calling the
     timer interrupt vector at 0050h.

      Note
      The above described Timer is the built-in timer in the gameboy. It has
     nothing to do with the MBC3s battery buffered Real Time Clock - that's a
     completely different thing, described in the chapter about Memory Banking
     Controllers.
  */

 public:
  Timer(Memory* m) : memory(m){};

  void run(int cycles);

  uint16_t getDividerRegister();
  uint8_t getTimerCounter();
  uint8_t getTimerModulo();

  bool isRunning();
  uint64_t getFrequency();

  void setDividerRegister(uint16_t newValue);
  void setTimerCounter(uint8_t newValue);
  void setTimerModulo(uint8_t newValue);

  void triggerTimerInterrupt();

 private:
  Memory* memory = NULL;

  static constexpr uint64_t INPUT_CLOCK_SELECT_Hz_MAP[4] = {
      4096,
      262144,
      65536,
      16384,
  };

  static constexpr uint16_t DIV_ADDR = 0xFF04;
  static constexpr uint16_t TIM_ADDR = 0xFF05;
  static constexpr uint16_t TMA_ADDR = 0xFF06;
  static constexpr uint16_t TAC_ADDR = 0xFF07;
};
