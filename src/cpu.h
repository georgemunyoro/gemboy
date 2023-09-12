#pragma once

#include <_types/_uint16_t.h>
#include <_types/_uint8_t.h>

#include <vector>

#include "../lib/tester.h"
#include "mem.h"
#include "registers.h"

enum ArithmeticTarget {
  A,
  B,
  C,
  D,
  E,
  H,
  L,
};

class Instruction {
 public:
  enum class Type {
    NOP = 0x00,
    LD_BC_d16 = 0x01,
    LD_BC_A = 0x02,
    INC_BC = 0x03,
    INC_B = 0x04,
    DEC_B = 0x05,
    LD_B_d8 = 0x06,
    RLCA = 0x07,
    LD_a16_SP = 0x08,
    ADD_HL_BC = 0x09,
    LD_A_BC = 0x0A,
    DEC_BC = 0x0B,
    INC_C = 0x0C,
    DEC_C = 0x0D,
    LD_C_d8 = 0x0E,
    RRCA = 0x0F,
    STOP = 0x10,
    LD_DE_d16 = 0x11,
    LD_DE_A = 0x12,
    INC_DE = 0x13,
    INC_D = 0x14,
    DEC_D = 0x15,
    LD_D_d8 = 0x16,
    RLA = 0x17,
    JR_s8 = 0x18,
    ADD_HL_DE = 0x19,
    LD_A_DE = 0x1A,
    DEC_DE = 0x1B,
    INC_E = 0x1C,
    DEC_E = 0x1D,
    LD_E_d8 = 0x1E,
    RRA = 0x1F,
    JR_NZ_s8 = 0x20,
    LD_HL_d16 = 0x21,
    LD_HL_inc__A = 0x22,
    INC_HL = 0x23,
    INC_H = 0x24,
    DEC_H = 0x25,
    LD_H_d8 = 0x26,
    DAA = 0x27,
    JR_Z_s8 = 0x28,
    ADD_HL_HL = 0x29,
    LD_A_HL_inc_ = 0x2A,
    DEC_HL = 0x2B,
    INC_L = 0x2C,
    DEC_L = 0x2D,
    LD_L_d8 = 0x2E,
    CPL = 0x2F,
    JR_NC_s8 = 0x30,
    LD_SP_d16 = 0x31,
    LD_HL_dec_A = 0x32,
    INC_SP = 0x33,
    INC_mem_HL = 0x34,
    DEC_mem_HL = 0x35,
    LD_HL_d8 = 0x36,
    SCF = 0x37,
    JR_C_s8 = 0x38,
    ADD_HL_SP = 0x39,
    LD_A_HL_dec_ = 0x3A,
    DEC_SP = 0x3B,
    INC_A = 0x3C,
    DEC_A = 0x3D,
    LD_A_d8 = 0x3E,
    CCF = 0x3F,
    LD_B_B = 0x40,
    LD_B_C = 0x41,
    LD_B_D = 0x42,
    LD_B_E = 0x43,
    LD_B_H = 0x44,
    LD_B_L = 0x45,
    LD_B_HL = 0x46,
    LD_B_A = 0x47,
    LD_C_B = 0x48,
    LD_C_C = 0x49,
    LD_C_D = 0x4A,
    LD_C_E = 0x4B,
    LD_C_H = 0x4C,
    LD_C_L = 0x4D,
    LD_C_HL = 0x4E,
    LD_C_A = 0x4F,
    LD_D_B = 0x50,
    LD_D_C = 0x51,
    LD_D_D = 0x52,
    LD_D_E = 0x53,
    LD_D_H = 0x54,
    LD_D_L = 0x55,
    LD_D_HL = 0x56,
    LD_D_A = 0x57,
    LD_E_B = 0x58,
    LD_E_C = 0x59,
    LD_E_D = 0x5A,
    LD_E_E = 0x5B,
    LD_E_H = 0x5C,
    LD_E_L = 0x5D,
    LD_E_HL = 0x5E,
    LD_E_A = 0x5F,
    LD_H_B = 0x60,
    LD_H_C = 0x61,
    LD_H_D = 0x62,
    LD_H_E = 0x63,
    LD_H_H = 0x64,
    LD_H_L = 0x65,
    LD_H_HL = 0x66,
    LD_H_A = 0x67,
    LD_L_B = 0x68,
    LD_L_C = 0x69,
    LD_L_D = 0x6A,
    LD_L_E = 0x6B,
    LD_L_H = 0x6C,
    LD_L_L = 0x6D,
    LD_L_HL = 0x6E,
    LD_L_A = 0x6F,
    LD_HL_B = 0x70,
    LD_HL_C = 0x71,
    LD_HL_D = 0x72,
    LD_HL_E = 0x73,
    LD_HL_H = 0x74,
    LD_HL_L = 0x75,
    HALT = 0x76,
    LD_HL_A = 0x77,
    LD_A_B = 0x78,
    LD_A_C = 0x79,
    LD_A_D = 0x7A,
    LD_A_E = 0x7B,
    LD_A_H = 0x7C,
    LD_A_L = 0x7D,
    LD_A_HL = 0x7E,
    LD_A_A = 0x7F,
    ADD_A_B = 0x80,
    ADD_A_C = 0x81,
    ADD_A_D = 0x82,
    ADD_A_E = 0x83,
    ADD_A_H = 0x84,
    ADD_A_L = 0x85,
    ADD_A_HL = 0x86,
    ADD_A_A = 0x87,
    ADC_A_B = 0x88,
    ADC_A_C = 0x89,
    ADC_A_D = 0x8A,
    ADC_A_E = 0x8B,
    ADC_A_H = 0x8C,
    ADC_A_L = 0x8D,
    ADC_A_HL = 0x8E,
    ADC_A_A = 0x8F,
    SUB_B = 0x90,
    SUB_C = 0x91,
    SUB_D = 0x92,
    SUB_E = 0x93,
    SUB_H = 0x94,
    SUB_L = 0x95,
    SUB_HL = 0x96,
    SUB_A = 0x97,
    SBC_A_B = 0x98,
    SBC_A_C = 0x99,
    SBC_A_D = 0x9A,
    SBC_A_E = 0x9B,
    SBC_A_H = 0x9C,
    SBC_A_L = 0x9D,
    SBC_A_HL = 0x9E,
    SBC_A_A = 0x9F,
    AND_B = 0xA0,
    AND_C = 0xA1,
    AND_D = 0xA2,
    AND_E = 0xA3,
    AND_H = 0xA4,
    AND_L = 0xA5,
    AND_HL = 0xA6,
    AND_A = 0xA7,
    XOR_B = 0xA8,
    XOR_C = 0xA9,
    XOR_D = 0xAA,
    XOR_E = 0xAB,
    XOR_H = 0xAC,
    XOR_L = 0xAD,
    XOR_HL = 0xAE,
    XOR_A = 0xAF,
    OR_B = 0xB0,
    OR_C = 0xB1,
    OR_D = 0xB2,
    OR_E = 0xB3,
    OR_H = 0xB4,
    OR_L = 0xB5,
    OR_HL = 0xB6,
    OR_A = 0xB7,
    CP_B = 0xB8,
    CP_C = 0xB9,
    CP_D = 0xBA,
    CP_E = 0xBB,
    CP_H = 0xBC,
    CP_L = 0xBD,
    CP_HL = 0xBE,
    CP_A = 0xBF,
    RET_NZ = 0xC0,
    POP_BC = 0xC1,
    JP_NZ_a16 = 0xC2,
    JP_a16 = 0xC3,
    CALL_NZ_a16 = 0xC4,
    PUSH_BC = 0xC5,
    ADD_A_d8 = 0xC6,
    RST_0 = 0xC7,
    RET_Z = 0xC8,
    RET = 0xC9,
    JP_Z_a16 = 0xCA,
    CALL_Z_a16 = 0xCC,
    CALL_a16 = 0xCD,
    ADC_A_d8 = 0xCE,
    RST_1 = 0xCF,
    RET_NC = 0xD0,
    POP_DE = 0xD1,
    JP_NC_a16 = 0xD2,
    CALL_NC_a16 = 0xD4,
    PUSH_DE = 0xD5,
    SUB_d8 = 0xD6,
    RST_2 = 0xD7,
    RET_C = 0xD8,
    RETI = 0xD9,
    JP_C_a16 = 0xDA,
    CALL_C_a16 = 0xDC,
    SBC_A_d8 = 0xDE,
    RST_3 = 0xDF,
    LD_a8_A = 0xE0,
    POP_HL = 0xE1,
    LD_mem_C_A = 0xE2,
    PUSH_HL = 0xE5,
    AND_d8 = 0xE6,
    RST_4 = 0xE7,
    ADD_SP_s8 = 0xE8,
    JP_HL = 0xE9,
    LD_a16_A = 0xEA,
    XOR_d8 = 0xEE,
    RST_5 = 0xEF,
    LD_A_a8 = 0xF0,
    POP_AF = 0xF1,
    LD_A_mem_C = 0xF2,
    DI = 0xF3,
    PUSH_AF = 0xF5,
    OR_d8 = 0xF6,
    RST_6 = 0xF7,
    LD_HL_SP_inc_s8 = 0xF8,
    LD_SP_HL = 0xF9,
    LD_A_a16 = 0xFA,
    EI = 0xFB,
    CP_d8 = 0xFE,
    RST_7 = 0xFF,

    // Prefixed instructions
    RLC_B = 0xCB00,
    RLC_C = 0xCB01,
    RLC_D = 0xCB02,
    RLC_E = 0xCB03,
    RLC_H = 0xCB04,
    RLC_L = 0xCB05,
    RLC_HL = 0xCB06,
    RLC_A = 0xCB07,
    RRC_B = 0xCB08,
    RRC_C = 0xCB09,
    RRC_D = 0xCB0A,
    RRC_E = 0xCB0B,
    RRC_H = 0xCB0C,
    RRC_L = 0xCB0D,
    RRC_HL = 0xCB0E,
    RRC_A = 0xCB0F,
    RL_B = 0xCB10,
    RL_C = 0xCB11,
    RL_D = 0xCB12,
    RL_E = 0xCB13,
    RL_H = 0xCB14,
    RL_L = 0xCB15,
    RL_HL = 0xCB16,
    RL_A = 0xCB17,
    RR_B = 0xCB18,
    RR_C = 0xCB19,
    RR_D = 0xCB1A,
    RR_E = 0xCB1B,
    RR_H = 0xCB1C,
    RR_L = 0xCB1D,
    RR_HL = 0xCB1E,
    RR_A = 0xCB1F,
    SLA_B = 0xCB20,
    SLA_C = 0xCB21,
    SLA_D = 0xCB22,
    SLA_E = 0xCB23,
    SLA_H = 0xCB24,
    SLA_L = 0xCB25,
    SLA_HL = 0xCB26,
    SLA_A = 0xCB27,
    SRA_B = 0xCB28,
    SRA_C = 0xCB29,
    SRA_D = 0xCB2A,
    SRA_E = 0xCB2B,
    SRA_H = 0xCB2C,
    SRA_L = 0xCB2D,
    SRA_HL = 0xCB2E,
    SRA_A = 0xCB2F,
    SWAP_B = 0xCB30,
    SWAP_C = 0xCB31,
    SWAP_D = 0xCB32,
    SWAP_E = 0xCB33,
    SWAP_H = 0xCB34,
    SWAP_L = 0xCB35,
    SWAP_HL = 0xCB36,
    SWAP_A = 0xCB37,
    SRL_B = 0xCB38,
    SRL_C = 0xCB39,
    SRL_D = 0xCB3A,
    SRL_E = 0xCB3B,
    SRL_H = 0xCB3C,
    SRL_L = 0xCB3D,
    SRL_HL = 0xCB3E,
    SRL_A = 0xCB3F,
    BIT_0_B = 0xCB40,
    BIT_0_C = 0xCB41,
    BIT_0_D = 0xCB42,
    BIT_0_E = 0xCB43,
    BIT_0_H = 0xCB44,
    BIT_0_L = 0xCB45,
    BIT_0_HL = 0xCB46,
    BIT_0_A = 0xCB47,
    BIT_1_B = 0xCB48,
    BIT_1_C = 0xCB49,
    BIT_1_D = 0xCB4A,
    BIT_1_E = 0xCB4B,
    BIT_1_H = 0xCB4C,
    BIT_1_L = 0xCB4D,
    BIT_1_HL = 0xCB4E,
    BIT_1_A = 0xCB4F,
    BIT_2_B = 0xCB50,
    BIT_2_C = 0xCB51,
    BIT_2_D = 0xCB52,
    BIT_2_E = 0xCB53,
    BIT_2_H = 0xCB54,
    BIT_2_L = 0xCB55,
    BIT_2_HL = 0xCB56,
    BIT_2_A = 0xCB57,
    BIT_3_B = 0xCB58,
    BIT_3_C = 0xCB59,
    BIT_3_D = 0xCB5A,
    BIT_3_E = 0xCB5B,
    BIT_3_H = 0xCB5C,
    BIT_3_L = 0xCB5D,
    BIT_3_HL = 0xCB5E,
    BIT_3_A = 0xCB5F,
    BIT_4_B = 0xCB60,
    BIT_4_C = 0xCB61,
    BIT_4_D = 0xCB62,
    BIT_4_E = 0xCB63,
    BIT_4_H = 0xCB64,
    BIT_4_L = 0xCB65,
    BIT_4_HL = 0xCB66,
    BIT_4_A = 0xCB67,
    BIT_5_B = 0xCB68,
    BIT_5_C = 0xCB69,
    BIT_5_D = 0xCB6A,
    BIT_5_E = 0xCB6B,
    BIT_5_H = 0xCB6C,
    BIT_5_L = 0xCB6D,
    BIT_5_HL = 0xCB6E,
    BIT_5_A = 0xCB6F,
    BIT_6_B = 0xCB70,
    BIT_6_C = 0xCB71,
    BIT_6_D = 0xCB72,
    BIT_6_E = 0xCB73,
    BIT_6_H = 0xCB74,
    BIT_6_L = 0xCB75,
    BIT_6_HL = 0xCB76,
    BIT_6_A = 0xCB77,
    BIT_7_B = 0xCB78,
    BIT_7_C = 0xCB79,
    BIT_7_D = 0xCB7A,
    BIT_7_E = 0xCB7B,
    BIT_7_H = 0xCB7C,
    BIT_7_L = 0xCB7D,
    BIT_7_HL = 0xCB7E,
    BIT_7_A = 0xCB7F,
    RES_0_B = 0xCB80,
    RES_0_C = 0xCB81,
    RES_0_D = 0xCB82,
    RES_0_E = 0xCB83,
    RES_0_H = 0xCB84,
    RES_0_L = 0xCB85,
    RES_0_HL = 0xCB86,
    RES_0_A = 0xCB87,
    RES_1_B = 0xCB88,
    RES_1_C = 0xCB89,
    RES_1_D = 0xCB8A,
    RES_1_E = 0xCB8B,
    RES_1_H = 0xCB8C,
    RES_1_L = 0xCB8D,
    RES_1_HL = 0xCB8E,
    RES_1_A = 0xCB8F,
    RES_2_B = 0xCB90,
    RES_2_C = 0xCB91,
    RES_2_D = 0xCB92,
    RES_2_E = 0xCB93,
    RES_2_H = 0xCB94,
    RES_2_L = 0xCB95,
    RES_2_HL = 0xCB96,
    RES_2_A = 0xCB97,
    RES_3_B = 0xCB98,
    RES_3_C = 0xCB99,
    RES_3_D = 0xCB9A,
    RES_3_E = 0xCB9B,
    RES_3_H = 0xCB9C,
    RES_3_L = 0xCB9D,
    RES_3_HL = 0xCB9E,
    RES_3_A = 0xCB9F,
    RES_4_B = 0xCBA0,
    RES_4_C = 0xCBA1,
    RES_4_D = 0xCBA2,
    RES_4_E = 0xCBA3,
    RES_4_H = 0xCBA4,
    RES_4_L = 0xCBA5,
    RES_4_HL = 0xCBA6,
    RES_4_A = 0xCBA7,
    RES_5_B = 0xCBA8,
    RES_5_C = 0xCBA9,
    RES_5_D = 0xCBAA,
    RES_5_E = 0xCBAB,
    RES_5_H = 0xCBAC,
    RES_5_L = 0xCBAD,
    RES_5_HL = 0xCBAE,
    RES_5_A = 0xCBAF,
    RES_6_B = 0xCBB0,
    RES_6_C = 0xCBB1,
    RES_6_D = 0xCBB2,
    RES_6_E = 0xCBB3,
    RES_6_H = 0xCBB4,
    RES_6_L = 0xCBB5,
    RES_6_HL = 0xCBB6,
    RES_6_A = 0xCBB7,
    RES_7_B = 0xCBB8,
    RES_7_C = 0xCBB9,
    RES_7_D = 0xCBBA,
    RES_7_E = 0xCBBB,
    RES_7_H = 0xCBBC,
    RES_7_L = 0xCBBD,
    RES_7_HL = 0xCBBE,
    RES_7_A = 0xCBBF,
    SET_0_B = 0xCBC0,
    SET_0_C = 0xCBC1,
    SET_0_D = 0xCBC2,
    SET_0_E = 0xCBC3,
    SET_0_H = 0xCBC4,
    SET_0_L = 0xCBC5,
    SET_0_HL = 0xCBC6,
    SET_0_A = 0xCBC7,
    SET_1_B = 0xCBC8,
    SET_1_C = 0xCBC9,
    SET_1_D = 0xCBCA,
    SET_1_E = 0xCBCB,
    SET_1_H = 0xCBCC,
    SET_1_L = 0xCBCD,
    SET_1_HL = 0xCBCE,
    SET_1_A = 0xCBCF,
    SET_2_B = 0xCBD0,
    SET_2_C = 0xCBD1,
    SET_2_D = 0xCBD2,
    SET_2_E = 0xCBD3,
    SET_2_H = 0xCBD4,
    SET_2_L = 0xCBD5,
    SET_2_HL = 0xCBD6,
    SET_2_A = 0xCBD7,
    SET_3_B = 0xCBD8,
    SET_3_C = 0xCBD9,
    SET_3_D = 0xCBDA,
    SET_3_E = 0xCBDB,
    SET_3_H = 0xCBDC,
    SET_3_L = 0xCBDD,
    SET_3_HL = 0xCBDE,
    SET_3_A = 0xCBDF,
    SET_4_B = 0xCBE0,
    SET_4_C = 0xCBE1,
    SET_4_D = 0xCBE2,
    SET_4_E = 0xCBE3,
    SET_4_H = 0xCBE4,
    SET_4_L = 0xCBE5,
    SET_4_HL = 0xCBE6,
    SET_4_A = 0xCBE7,
    SET_5_B = 0xCBE8,
    SET_5_C = 0xCBE9,
    SET_5_D = 0xCBEA,
    SET_5_E = 0xCBEB,
    SET_5_H = 0xCBEC,
    SET_5_L = 0xCBED,
    SET_5_HL = 0xCBEE,
    SET_5_A = 0xCBEF,
    SET_6_B = 0xCBF0,
    SET_6_C = 0xCBF1,
    SET_6_D = 0xCBF2,
    SET_6_E = 0xCBF3,
    SET_6_H = 0xCBF4,
    SET_6_L = 0xCBF5,
    SET_6_HL = 0xCBF6,
    SET_6_A = 0xCBF7,
    SET_7_B = 0xCBF8,
    SET_7_C = 0xCBF9,
    SET_7_D = 0xCBFA,
    SET_7_E = 0xCBFB,
    SET_7_H = 0xCBFC,
    SET_7_L = 0xCBFD,
    SET_7_HL = 0xCBFE,
    SET_7_A = 0xCBFF

  };

  Instruction(uint16_t opcode, bool isPrefixed);
  Instruction(Type type) : type_(type){};
  Instruction(Type type, ArithmeticTarget target)
      : type_(type), arithmeticTarget_(target) {}

  Type type() const { return type_; }
  ArithmeticTarget target() const { return arithmeticTarget_; };

  std::string TypeRepr();
  std::string ArithmeticTargetRepr();

 private:
  Type type_;
  ArithmeticTarget arithmeticTarget_;
};

struct Clock {
  uint8_t m;
  uint8_t t;
};

class CPU {
 public:
  CPU(Memory* m) : memory(m){};

  void setRomData(std::vector<uint8_t>& rd) {
    for (int i = 0; i <= 256; ++i) {
      romData.push_back(rd[i]);
    }
  }

  uint16_t executeInstruction(Instruction* instruction);
  void step();
  void setPC(uint16_t newPC) { PC = newPC; };
  uint16_t incrementPC() { return ++PC; };
  uint16_t getPC() { return PC; };
  Registers registers;

  //  private:
  Memory* memory;
  std::vector<uint8_t> romData;
  // Clock clock;

  uint16_t PC;
  uint16_t SP = 0;
  bool IME;

  // INSTRUCTIONS
  uint8_t add(uint8_t value);
  uint8_t adc(uint8_t value);
  uint8_t sub(uint8_t value);

  void cp(uint8_t value);

  void push(uint16_t value);
  uint16_t pop();

  uint8_t rl(uint8_t& reg);
  uint8_t rr(uint8_t& reg);
  uint8_t rrc(uint8_t& reg);
  uint8_t rlc(uint8_t& reg);

  uint8_t dec(uint8_t& reg);
  uint8_t inc(uint8_t& reg);

  void bit(uint8_t value, uint8_t b);

  uint16_t addCompoundRegisters(uint16_t a, uint16_t b);
  uint8_t xor_(uint8_t reg);
  uint8_t or_(uint8_t reg);
  uint8_t and_(uint8_t reg);
  void rst(uint8_t addr);

  bool inBootRom = true;
  bool halted = false;
  bool stopped = false;

  int cycles;
};

extern Memory g_Memory;
extern CPU g_CPU;

/*
 * Called once during startup. The area of memory pointed to by
 * tester_instruction_mem will contain instructions the tester will inject, and
 * should be mapped read-only at addresses [0,tester_instruction_mem_size).
 */
static void mycpu_init(size_t tester_instruction_mem_size,
                       uint8_t* tester_instruction_mem) {
  g_CPU.memory->memory = tester_instruction_mem;
  g_Memory.MEM_SIZE = tester_instruction_mem_size;
}

/*
 * Resets the CPU state (e.g., registers) to a given state state.
 */
static void mycpu_set_state(struct state* state) {
  (void)state;

  g_Memory.num_mem_accesses = 0;

  g_CPU.registers.A = state->reg8.A;
  g_CPU.registers.F.setValue(state->reg8.F);
  g_CPU.registers.B = state->reg8.B;
  g_CPU.registers.C = state->reg8.C;
  g_CPU.registers.D = state->reg8.D;
  g_CPU.registers.E = state->reg8.E;
  g_CPU.registers.H = state->reg8.H;
  g_CPU.registers.L = state->reg8.L;

  g_CPU.SP = state->SP;
  g_CPU.PC = state->PC;

  g_CPU.halted = state->halted;
  g_CPU.IME = state->interrupts_master_enabled;

  for (int i = 0; i < 16; ++i)
    g_Memory.mem_accesses[i] = state->mem_accesses[i];
}

static void mycpu_get_state(struct state* state) {
  state->num_mem_accesses = g_Memory.num_mem_accesses;

  state->reg8.A = g_CPU.registers.A;
  state->reg8.F = g_CPU.registers.F.getValue();
  state->reg8.B = g_CPU.registers.B;
  state->reg8.C = g_CPU.registers.C;
  state->reg8.D = g_CPU.registers.D;
  state->reg8.E = g_CPU.registers.E;
  state->reg8.H = g_CPU.registers.H;
  state->reg8.L = g_CPU.registers.L;

  state->SP = g_CPU.SP;
  state->PC = g_CPU.PC;

  state->halted = g_CPU.halted;
  state->interrupts_master_enabled = g_CPU.IME;

  for (int i = 0; i < 16; ++i)
    state->mem_accesses[i] = g_Memory.mem_accesses[i];
}

static int mycpu_step(void) {
  g_CPU.step();
  return g_CPU.cycles;
}
