#pragma once

#include <_types/_uint16_t.h>
#include <_types/_uint8_t.h>

#include <bitset>

class FlagsRegister {
 public:
  bool zero;
  bool subtraction;
  bool halfCarry;
  bool carry;

  uint8_t getValue() {
    uint8_t v = 0;
    if (zero) v |= (1 << 7);
    if (subtraction) v |= (1 << 6);
    if (halfCarry) v |= (1 << 5);
    if (carry) v |= (1 << 4);
    return v;
  };
  void setValue(uint8_t newValue);

 private:
  uint8_t value;
};

class Registers {
 public:
  uint8_t A;
  uint8_t B;
  uint8_t C;
  uint8_t D;
  uint8_t E;

  FlagsRegister F;

  uint8_t H;
  uint8_t L;

  uint16_t get_AF();
  uint16_t get_BC();
  uint16_t get_DE();
  uint16_t get_HL();

  void set_AF(uint16_t value);
  void set_BC(uint16_t value);
  void set_DE(uint16_t value);
  void set_HL(uint16_t value);
};
