#include "registers.h"

#include <_types/_uint8_t.h>

void FlagsRegister::setValue(uint8_t newValue) {
  std::bitset<8> newValueBits(newValue);

  //    ┌-> Carry
  //  ┌-+> Subtraction
  //  | |
  // 1111 0000
  // | |
  // └-+> Zero
  //   └-> Half Carry

  zero = newValueBits.test(7);
  subtraction = newValueBits.test(6);
  halfCarry = newValueBits.test(5);
  carry = newValueBits.test(4);

  value = newValue;
}

uint16_t Registers::get_AF() { return ((uint16_t)(A) << 8) | F.getValue(); };
uint16_t Registers::get_BC() { return ((uint16_t)(B) << 8) | C; };
uint16_t Registers::get_DE() { return ((uint16_t)(D) << 8) | E; };
uint16_t Registers::get_HL() { return ((uint16_t)(H) << 8) | L; };

void Registers::set_AF(u_int16_t value) {
  A = ((value & 0xFF00) >> 8);
  F.setValue(value & 0x00FF);
}

void Registers::set_BC(u_int16_t value) {
  B = ((value & 0xFF00) >> 8);
  C = value & 0x00FF;
}

void Registers::set_DE(u_int16_t value) {
  D = ((value & 0xFF00) >> 8);
  E = value & 0x00FF;
}

void Registers::set_HL(u_int16_t value) {
  H = ((value & 0xFF00) >> 8);
  L = value & 0x00FF;
}
