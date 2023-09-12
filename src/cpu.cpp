
#include "cpu.h"

#include <_types/_uint16_t.h>
#include <_types/_uint32_t.h>
#include <_types/_uint8_t.h>
#include <sys/_types/_int8_t.h>

#include <__errc>
#include <bitset>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../lib/tester.h"
#include "mem.h"
#include "registers.h"
#include "utils.h"

uint16_t signedAdd(uint16_t a, uint8_t b) {
  int8_t bSigned = (int8_t)b;
  return bSigned >= 0 ? a + (uint16_t)bSigned : a - (uint16_t)(-bSigned);
}

// #define DEBUG_LOG ;  // Comment out to disable instruction logging.

// Executes an instruction and returns the next PC.
uint16_t CPU::executeInstruction(Instruction *instruction) {
#ifdef DEBUG_LOG
  printf(
      "A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X "
      "PC:%04X "
      "PCMEM:%02X,%02X,%02X,%02X OP:%02X %s\n",
      registers.A, registers.F.getValue(), registers.B, registers.C,
      registers.D, registers.E, registers.H, registers.L, SP, PC,
      memory->readByte(PC + 1), memory->readByte(PC + 2),
      memory->readByte(PC + 3), memory->readByte(PC + 4), memory->readByte(PC),
      instruction->TypeRepr().c_str());
#endif

  switch (instruction->type()) {
    // Only advances the program counter by 1. Performs no other operations
    // that would have an effect.
    case Instruction::Type::NOP: {
      incrementPC();
      break;
    }

    case Instruction::Type::HALT: {
      halted = true;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the 2 bytes of immediate data into register pair SP.
    case Instruction::Type::LD_SP_d16: {
      SP = memory->readWord(PC + 1);
      setPC(PC + 3);
      break;
    }

    // Take the logical exclusive-OR for each bit of the contents of register
    // A and the contents of register A, and store the results in register A.
    case Instruction::Type::XOR_A: {
      xor_(registers.A);
      incrementPC();
      break;
    }

    // Load the 2 bytes of immediate data into register pair HL.
    case Instruction::Type::LD_HL_d16: {
      registers.set_HL(memory->readWord(PC + 1));
      setPC(PC + 3);
      break;
    }

    // Store the contents of register A into the memory location specified by
    // register pair HL, and simultaneously decrement the contents of HL.
    case Instruction::Type::LD_HL_dec_A: {
      memory->writeByte(registers.get_HL(), registers.A);
      registers.set_HL(registers.get_HL() - 1);
      incrementPC();
      break;
    }

    // If the Z flag is 0, jump s8 steps from the current address stored in
    // the program counter (PC). If not, the instruction following the current
    // JP instruction is executed (as usual).
    case Instruction::Type::JR_NZ_s8: {
      if (!registers.F.zero) PC = signedAdd(PC, memory->readByte(PC + 1));
      setPC(PC + 2);
      break;
    }

    // Load the 8-bit immediate operand d8 into register A.
    case Instruction::Type::LD_A_d8: {
      registers.A = memory->readByte(PC + 1);
      setPC(PC + 2);
      break;
    }

    case Instruction::Type::LD_A_A: {
      registers.A = registers.A;
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::LD_A_B: {
      registers.A = registers.B;
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::LD_A_C: {
      registers.A = registers.C;
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::LD_A_D: {
      registers.A = registers.D;
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::LD_A_HL: {
      registers.A = memory->readByte(registers.get_HL());
      incrementPC();
      ++cycles;
      break;
    }

    // Load the 8-bit immediate operand d8 into register C.
    case Instruction::Type::LD_C_d8: {
      registers.C = memory->readByte(PC + 1);
      setPC(PC + 2);
      break;
    }

    // Store the contents of register A in the internal RAM, port register, or
    // mode register at the address in the range 0xFF00-0xFFFF specified by
    // register C.
    case Instruction::Type::LD_mem_C_A: {
      memory->writeByte(0xFF00 + registers.C, registers.A);
      incrementPC();
      break;
    }

    // Increment the contents of register C by 1.
    case Instruction::Type::INC_C: {
      inc(registers.C);
      incrementPC();
      break;
    }

    // Increment the contens of register pair BC by 1
    case Instruction::Type::INC_BC: {
      registers.set_BC(registers.get_BC() + 1);
      incrementPC();
      break;
    }

    // Store the contents of register A in the memory location specified by
    // register pair HL.
    case Instruction::Type::LD_HL_A: {
      memory->writeByte(registers.get_HL(), registers.A);
      incrementPC();
      break;
    }

    // Store the contents of register A in the internal RAM, port register, or
    // mode register at the address in the range 0xFF00-0xFFFF specified by
    // the 8-bit immediate operand a8.
    case Instruction::Type::LD_a8_A: {
      if (memory->readByte(PC + 1) == 0x50 && inBootRom) {
        for (int i = 0; i <= 256; ++i) {
          memory->writeByte(i, romData[i]);
        }
        inBootRom = false;
      }

      memory->writeByte(0xFF00 + memory->readByte(PC + 1), registers.A);
      setPC(PC + 2);
      break;
    }

    // Load the 2 bytes of immediate data into register pair DE.
    case Instruction::Type::LD_DE_d16: {
      registers.set_DE(memory->readWord(PC + 1));
      setPC(PC + 3);
      break;
    }

    // Take the logical AND for each bit of the contents of register C and the
    // contents of register A, and store the results in register A.
    case Instruction::Type::AND_C: {
      registers.A &= registers.C;

      registers.F.zero = registers.A == 0;
      registers.F.carry = false;
      registers.F.halfCarry = true;
      registers.F.subtraction = false;

      incrementPC();
      break;
    }

    // Load the 8-bit contents of memory specified by register pair DE into
    // register A.
    case Instruction::Type::LD_A_DE: {
      registers.A = memory->readByte(registers.get_DE());
      incrementPC();
      break;
    }

    // In memory, push the program counter PC value corresponding to the
    // address following the CALL instruction to the 2 bytes following the
    // byte specified by the current stack pointer SP. Then load the 16-bit
    // immediate operand a16 into PC.
    case Instruction::Type::CALL_a16: {
      uint16_t a16 = memory->readWord(PC + 1);
      push(PC + 3);
      setPC(a16);
      break;
    }

    // Flip the carry flag CY.
    case Instruction::Type::CCF: {
      registers.F.carry = !registers.F.carry;
      registers.F.subtraction = false;
      registers.F.halfCarry = false;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register A into register B.
    case Instruction::Type::LD_B_A: {
      registers.B = registers.A;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register B into register B.
    case Instruction::Type::LD_B_B: {
      registers.B = registers.B;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register C into register B.
    case Instruction::Type::LD_B_C: {
      registers.B = registers.C;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register D into register B.
    case Instruction::Type::LD_B_D: {
      registers.B = registers.D;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register E into register B.
    case Instruction::Type::LD_B_E: {
      registers.B = registers.E;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register H into register B.
    case Instruction::Type::LD_B_H: {
      registers.B = registers.H;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register L into register B.
    case Instruction::Type::LD_B_L: {
      registers.B = registers.L;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register A into register C.
    case Instruction::Type::LD_C_A: {
      registers.C = registers.A;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register B into register B.
    case Instruction::Type::LD_C_B: {
      registers.C = registers.B;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register C into register B.
    case Instruction::Type::LD_C_C: {
      registers.C = registers.C;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register D into register B.
    case Instruction::Type::LD_C_D: {
      registers.C = registers.D;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register E into register B.
    case Instruction::Type::LD_C_E: {
      registers.C = registers.E;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register H into register B.
    case Instruction::Type::LD_C_H: {
      registers.C = registers.H;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register L into register B.
    case Instruction::Type::LD_C_L: {
      registers.C = registers.L;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register A into register D.
    case Instruction::Type::LD_D_A: {
      registers.D = registers.A;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register B into register D.
    case Instruction::Type::LD_D_B: {
      registers.D = registers.B;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register C into register D.
    case Instruction::Type::LD_D_C: {
      registers.D = registers.C;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register D into register D.
    case Instruction::Type::LD_D_D: {
      registers.D = registers.D;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register E into register D.
    case Instruction::Type::LD_D_E: {
      registers.D = registers.E;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register H into register D.
    case Instruction::Type::LD_D_H: {
      registers.D = registers.H;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register L into register D.
    case Instruction::Type::LD_D_L: {
      registers.D = registers.L;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register A into register E.
    case Instruction::Type::LD_E_A: {
      registers.E = registers.A;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register B into register E.
    case Instruction::Type::LD_E_B: {
      registers.E = registers.B;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register C into register E.
    case Instruction::Type::LD_E_C: {
      registers.E = registers.C;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register D into register E.
    case Instruction::Type::LD_E_D: {
      registers.E = registers.D;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register E into register E.
    case Instruction::Type::LD_E_E: {
      registers.E = registers.E;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register H into register E.
    case Instruction::Type::LD_E_H: {
      registers.E = registers.H;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register L into register E.
    case Instruction::Type::LD_E_L: {
      registers.E = registers.L;
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::LD_H_C: {
      registers.H = registers.C;
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::LD_H_D: {
      registers.H = registers.D;
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::LD_H_E: {
      registers.H = registers.E;
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::LD_H_L: {
      registers.H = registers.L;
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::LD_H_H: {
      registers.H = registers.H;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register A into register L.
    case Instruction::Type::LD_L_A: {
      registers.L = registers.A;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register B into register L.
    case Instruction::Type::LD_L_B: {
      registers.L = registers.B;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register C into register L.
    case Instruction::Type::LD_L_C: {
      registers.L = registers.C;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register D into register L.
    case Instruction::Type::LD_L_D: {
      registers.L = registers.D;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register E into register L.
    case Instruction::Type::LD_L_E: {
      registers.L = registers.E;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register H into register L.
    case Instruction::Type::LD_L_H: {
      registers.L = registers.H;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of register L into register L.
    case Instruction::Type::LD_L_L: {
      registers.L = registers.L;
      incrementPC();
      ++cycles;
      break;
    }

    // Load the 8-bit contents of memory specified by register pair HL into
    // register B.
    case Instruction::Type::LD_B_HL: {
      registers.B = memory->readByte(registers.get_HL());
      incrementPC();
      cycles += 2;
      break;
    }

    // Load the 8-bit contents of memory specified by register pair HL into
    // register C.
    case Instruction::Type::LD_C_HL: {
      registers.C = memory->readByte(registers.get_HL());
      incrementPC();
      cycles += 2;
      break;
    }

    // Load the 8-bit contents of memory specified by register pair HL into
    // register D.
    case Instruction::Type::LD_D_HL: {
      registers.D = memory->readByte(registers.get_HL());
      incrementPC();
      cycles += 2;
      break;
    }

    // Load the 8-bit contents of memory specified by register pair HL into
    // register E.
    case Instruction::Type::LD_E_HL: {
      registers.E = memory->readByte(registers.get_HL());
      incrementPC();
      cycles += 2;
      break;
    }

    // Load the 8-bit contents of memory specified by register pair HL into
    // register H.
    case Instruction::Type::LD_H_HL: {
      registers.H = memory->readByte(registers.get_HL());
      incrementPC();
      cycles += 2;
      break;
    }

    // Load the 8-bit contents of memory specified by register pair HL into
    // register L.
    case Instruction::Type::LD_L_HL: {
      registers.L = memory->readByte(registers.get_HL());
      incrementPC();
      cycles += 2;
      break;
    }

    // If the CY flag is 0, control is returned to the source program by
    // popping from the memory stack the program counter PC value that was
    // pushed to the stack when the subroutine was called.
    case Instruction::Type::RET_NC: {
      if (!registers.F.carry)
        PC = pop();
      else
        incrementPC();
      break;
    }

    // Load the 8-bit immediate operand d8 into register B.
    case Instruction::Type::LD_B_d8: {
      registers.B = memory->readByte(PC + 1);
      setPC(PC + 2);
      break;
    }

    // Push the contents of register pair BC onto the memory stack
    case Instruction::Type::PUSH_BC: {
      push(registers.get_BC());
      incrementPC();
      break;
    }

    // Rotate the contents of register A to the left, through the carry (CY)
    // flag.
    case Instruction::Type::RLA: {
      rl(registers.A);
      registers.F.zero = false;
      incrementPC();
      break;
    }

    // Pop the contents from the memory stack into register pair BC.
    case Instruction::Type::POP_BC: {
      registers.set_BC(pop());
      incrementPC();
      break;
    }

    // Decrement the contents of register B by 1.
    case Instruction::Type::DEC_B: {
      dec(registers.B);
      incrementPC();
      break;
    }

    // Store the contents of register A into the memory location specified by
    // register pair HL, and simultaneously increment the contents of HL.
    case Instruction::Type::LD_HL_inc__A: {
      memory->writeByte(registers.get_HL(), registers.A);
      registers.set_HL(registers.get_HL() + 1);
      incrementPC();
      break;
    }

    // Load the contents of memory specified by register pair HL into register
    // A, and simultaneously increment the contents of HL.
    case Instruction::Type::LD_A_HL_inc_: {
      registers.A = memory->readByte(registers.get_HL());
      registers.set_HL(registers.get_HL() + 1);
      incrementPC();
      break;
    }

    // Increment the contents of register pair HL by 1.
    case Instruction::Type::INC_HL: {
      registers.set_HL(registers.get_HL() + 1);
      incrementPC();
      break;
    }

    case Instruction::Type::LD_HL_B: {
      memory->writeByte(registers.get_HL(), registers.B);
      incrementPC();
      cycles += 2;
      break;
    }

    case Instruction::Type::LD_HL_C: {
      memory->writeByte(registers.get_HL(), registers.C);
      incrementPC();
      cycles += 2;
      break;
    }

    case Instruction::Type::LD_HL_D: {
      memory->writeByte(registers.get_HL(), registers.D);
      incrementPC();
      cycles += 2;
      break;
    }

    case Instruction::Type::LD_HL_E: {
      memory->writeByte(registers.get_HL(), registers.E);
      incrementPC();
      cycles += 2;
      break;
    }

    case Instruction::Type::LD_HL_H: {
      memory->writeByte(registers.get_HL(), registers.H);
      incrementPC();
      cycles += 2;
      break;
    }

    case Instruction::Type::LD_HL_L: {
      memory->writeByte(registers.get_HL(), registers.L);
      incrementPC();
      cycles += 2;
      break;
    }

    // Pop from the memory stack the program counter PC value pushed when the
    // subroutine was called, returning control to the source program.
    case Instruction::Type::RET: {
      PC = pop();
      break;
    }

    // Increment the contents of register pair DE by 1.
    case Instruction::Type::INC_DE: {
      registers.set_DE(registers.get_DE() + 1);
      incrementPC();
      break;
    }

    // Load the contents of register E into register A.
    case Instruction::Type::LD_A_E: {
      registers.A = registers.E;
      incrementPC();
      break;
    }

    // Compare the contents of register A and the contents of the 8-bit
    // immediate operand d8 by calculating A - d8, and set the Z flag if they
    // are equal.
    case Instruction::Type::CP_d8: {
      // printf("CP d8=%04X A=%04X\n", memory->readByte(PC + 1), registers.A);
      cp(memory->readByte(PC + 1));
      setPC(PC + 2);
      break;
    }

    // Compare the contents of memory specified by register pair HL and the
    // contents of register A by calculating A - (HL), and set the Z flag if
    // they are equal.
    case Instruction::Type::CP_HL: {
      cp(memory->readByte(registers.get_HL()));
      incrementPC();
      break;
    }

    // Store the contents of register A in the internal RAM or register
    // specified by the 16-bit immediate operand a16.
    case Instruction::Type::LD_a16_A: {
      memory->writeByte(memory->readWord(PC + 1), registers.A);
      setPC(PC + 3);
      break;
    }

    // Decrement the contents of register A by 1.
    case Instruction::Type::DEC_A: {
      dec(registers.A);
      incrementPC();
      break;
    }

    // If the Z flag is 1, jump s8 steps from the current address stored in the
    // program counter (PC). If not, the instruction following the current JP
    // instruction is executed (as usual).
    case Instruction::Type::JR_Z_s8: {
      if (registers.F.zero)
        PC += (int8_t)memory->readByte(PC + 1) + 2;
      else
        setPC(PC + 2);
      break;
    }

    // Decrement the contents of register C by 1.
    case Instruction::Type::DEC_C: {
      dec(registers.C);
      incrementPC();
      break;
    }

    // Load the 8-bit immediate operand d8 into register L.
    case Instruction::Type::LD_L_d8: {
      registers.L = memory->readByte(PC + 1);
      setPC(PC + 2);
      break;
    }

    // Jump s8 steps from the current address in the program counter (PC). (Jump
    // relative.)
    case Instruction::Type::JR_s8: {
      PC = signedAdd(PC + 2, memory->readByte(PC + 1));
      break;
    }

    // Rotate the contents of register A to the right, through the carry (CY)
    // flag.
    case Instruction::Type::RRA: {
      rr(registers.A);
      registers.F.zero = false;
      incrementPC();
      ++cycles;
      break;
    }

    // Flips all the bits in the 8-bit A register, and sets the N and H flags.
    case Instruction::Type::CPL: {
      registers.A = ~registers.A;
      registers.F.subtraction = true;
      registers.F.halfCarry = true;
      incrementPC();
      ++cycles;
      break;
    }

    // Sets the carry flag, and clears the N and H flags.
    case Instruction::Type::SCF: {
      registers.F.carry = true;
      registers.F.halfCarry = false;
      registers.F.subtraction = false;
      incrementPC();
      ++cycles;
      break;
    }

    // Adjust the accumulator (register A) to a binary-coded decimal (BCD)
    // number after BCD addition and subtraction operations.
    case Instruction::Type::DAA: {
      if (!registers.F.subtraction) {
        if (registers.F.carry || registers.A > 0x99) {
          registers.A += 0x60;
          registers.F.carry = true;
        }

        if (registers.F.halfCarry || (registers.A & 0x0f) > 0x09) {
          registers.A += 0x6;
        }
      } else {
        if (registers.F.carry) {
          registers.A -= 0x60;
        }
        if (registers.F.halfCarry) {
          registers.A -= 0x6;
        }
      }

      registers.F.zero = registers.A == 0;
      registers.F.halfCarry = false;

      incrementPC();
      ++cycles;
      break;
    }

    // Load the contents of memory specified by register pair HL into register
    // A, and simultaneously decrement the contents of HL.
    case Instruction::Type::LD_A_HL_dec_: {
      registers.A = memory->readByte(registers.get_HL());
      registers.set_HL(registers.get_HL() - 1);
      incrementPC();
      cycles += 2;
      break;
    }

    case Instruction::Type::JR_C_s8: {
      if (registers.F.carry) {
        PC = signedAdd(PC + 2, memory->readByte(PC + 1));
      } else {
        setPC(PC + 2);
      }

      ++cycles;
      break;
    }

    case Instruction::Type::JR_NC_s8: {
      if (!registers.F.carry) {
        PC = signedAdd(PC + 2, memory->readByte(PC + 1));
      } else {
        setPC(PC + 2);
      }

      ++cycles;
      break;
    }

    // Load the contents of register A into register H.
    case Instruction::Type::LD_H_A: {
      registers.H = registers.A;
      incrementPC();
      break;
    }

    // Load the contents of register B into register H.
    case Instruction::Type::LD_H_B: {
      registers.H = registers.B;
      incrementPC();
      break;
    }

    // Increment the contents of register B by 1.
    case Instruction::Type::INC_B: {
      inc(registers.B);
      incrementPC();
      break;
    }

    // Load the 8-bit immediate operand d8 into register E.
    case Instruction::Type::LD_E_d8: {
      registers.E = memory->readByte(PC + 1);
      setPC(PC + 2);
      break;
    }

    // Load into register A the contents of the internal RAM, port register,
    // or mode register at the address in the range 0xFF00-0xFFFF specified by
    // the 8-bit immediate operand a8.
    case Instruction::Type::LD_A_a8: {
      if (0xFF00 + (uint16_t)(memory->readByte(PC + 1)) == 0xFF44) {
        registers.A = 0x90;
      } else {
        registers.A =
            memory->readByte(0xFF00 + (uint16_t)memory->readByte(PC + 1));
      }

      setPC(PC + 2);
      break;
    }

    // Decrement the contents of register E by 1.
    case Instruction::Type::DEC_E: {
      dec(registers.E);
      incrementPC();
      break;
    }

    // Decrement the contents of register H by 1.
    case Instruction::Type::DEC_H: {
      dec(registers.H);
      incrementPC();
      break;
    }

    // Decrement the contents of register L by 1.
    case Instruction::Type::DEC_L: {
      dec(registers.L);
      incrementPC();
      break;
    }

    // Increment the contents of register H by 1.
    case Instruction::Type::INC_H: {
      inc(registers.H);
      incrementPC();
      break;
    }

    // Load the contents of register H into register A.
    case Instruction::Type::LD_A_H: {
      registers.A = registers.H;
      incrementPC();
      break;
    }

    // Subtract the contents of register B from the contents of register A,
    // and store the results in register A.
    case Instruction::Type::SUB_B: {
      sub(registers.B);
      incrementPC();
      break;
    }

    // Decrement the contents of register D by 1.
    case Instruction::Type::DEC_D: {
      dec(registers.D);
      incrementPC();
      break;
    }

    // Load the 8-bit immediate operand d8 into register D.
    case Instruction::Type::LD_D_d8: {
      registers.D = memory->readByte(PC + 1);
      setPC(PC + 2);
      break;
    }

    // Load the 2 bytes of immediate data into register pair BC.
    case Instruction::Type::LD_BC_d16: {
      registers.set_BC(memory->readWord(PC + 1));
      setPC(PC + 3);
      break;
    }

    // Load the contents of register L into register A.
    case Instruction::Type::LD_A_L: {
      registers.A = registers.L;
      incrementPC();
      break;
    }

    // Push the contents of register pair AF onto the memory stack
    case Instruction::Type::PUSH_AF: {
      push(registers.get_AF());
      incrementPC();
      break;
    }

    // Add the contents of memory specified by register pair HL to the
    // contents of register A, and store the results in register A.
    case Instruction::Type::ADD_A_HL: {
      add(memory->readByte(registers.get_HL()));
      incrementPC();
      break;
    }

    // Add the contents of register pair DE to the contents of register pair
    // HL, and store the results in register pair HL.
    case Instruction::Type::ADD_HL_DE: {
      registers.set_HL(
          addCompoundRegisters(registers.get_HL(), registers.get_DE()));
      cycles += 2;
      incrementPC();
      break;
    }

    // Add the contents of register pair BC to the contents of register pair
    // HL, and store the results in register pair HL.
    case Instruction::Type::ADD_HL_BC: {
      registers.set_HL(
          addCompoundRegisters(registers.get_HL(), registers.get_BC()));
      cycles += 2;
      incrementPC();
      break;
    }

    // Add the contents of register pair HL to the contents of register pair
    // HL, and store the results in register pair HL.
    case Instruction::Type::ADD_HL_HL: {
      registers.set_HL(
          addCompoundRegisters(registers.get_HL(), registers.get_HL()));
      cycles += 2;
      incrementPC();
      break;
    }

    // Add the contents of register SP to the contents of register pair HL,
    // and store the results in register pair HL.
    case Instruction::Type::ADD_HL_SP: {
      registers.set_HL(addCompoundRegisters(registers.get_HL(), SP));
      cycles += 2;
      incrementPC();
      break;
    }

    case Instruction::Type::RST_7: {
      rst(0x38);
      break;
    }

    // Store the lower byte of stack pointer SP at the address specified by
    // the 16-bit immediate operand a16, and store the upper byte of SP at
    // address a16 + 1.
    case Instruction::Type::LD_a16_SP: {
      uint16_t a16 = memory->readWord(PC + 1);
      memory->writeWord(a16, SP);
      setPC(PC + 3);
      break;
    }

    // Load the 16-bit immediate operand a16 into the program counter (PC).
    // a16 specifies the address of the subsequently executed instruction.
    case Instruction::Type::JP_a16: {
      PC = memory->readWord(PC + 1);
      break;
    }

    // Reset the interrupt master enable (IME) flag and prohibit maskable
    // interrupts.
    case Instruction::Type::DI: {
      IME = false;
      incrementPC();
      break;
    }

    // Push the contents of register pair HL onto the memory stack.
    case Instruction::Type::PUSH_HL: {
      push(registers.get_HL());
      incrementPC();
      break;
    }

    // Pop the contents from the memory stack into register pair into register
    // pair HL
    case Instruction::Type::POP_HL: {
      registers.set_HL(pop());
      incrementPC();
      break;
    }

    // Pop the contents from the memory stack into register pair into register
    // pair AF
    case Instruction::Type::POP_AF: {
      registers.set_AF(pop());
      incrementPC();
      break;
    }

    // Pop the contents from the memory stack into register pair into register
    // pair AF
    case Instruction::Type::POP_DE: {
      registers.set_DE(pop());
      incrementPC();
      break;
    }

      // Take the logical OR for each bit of the contents of register x and
      // the contents of register A, and store the results in register A.

    case Instruction::Type::OR_A: {
      or_(registers.A);
      incrementPC();
      break;
    }

    case Instruction::Type::OR_B: {
      or_(registers.B);
      incrementPC();
      break;
    }

    case Instruction::Type::OR_C: {
      or_(registers.C);
      incrementPC();
      break;
    }

    case Instruction::Type::OR_D: {
      or_(registers.D);
      incrementPC();
      break;
    }

    case Instruction::Type::OR_E: {
      or_(registers.E);
      incrementPC();
      break;
    }

    case Instruction::Type::OR_L: {
      or_(registers.L);
      incrementPC();
      break;
    }

    // ----

    // Load into register A the contents of the internal RAM or register
    // specified by the 16-bit immediate operand a16.
    case Instruction::Type::LD_A_a16: {
      registers.A = memory->readByte(memory->readWord(PC + 1));
      setPC(PC + 3);
      break;
    }

    // Load the 8-bit contents of memory specified by register pair BC into
    // register A.
    case Instruction::Type::LD_A_BC: {
      registers.A = memory->readByte(registers.get_BC());
      cycles += 2;
      incrementPC();
      break;
    }

    // Take the logical AND for each bit of the contents of 8-bit immediate
    // operand d8 and the contents of register A, and store the results in
    // register A.
    case Instruction::Type::AND_d8: {
      and_(memory->readByte(PC + 1));
      setPC(PC + 2);
      break;
    }

    // Store the contents of register A in the memory location specified by
    // register pair BC.
    case Instruction::Type::LD_BC_A: {
      memory->writeByte(registers.get_BC(), registers.A);
      cycles += 2;
      incrementPC();
      break;
    }

    // Increment the contents of register pair SP by 1.
    case Instruction::Type::INC_SP: {
      ++SP;
      cycles += 2;
      incrementPC();
      break;
    }

    // Increment the contents of register D by 1.
    case Instruction::Type::INC_D: {
      inc(registers.D);
      ++cycles;
      incrementPC();
      break;
    }

    // Increment the contents of register E by 1.
    case Instruction::Type::INC_E: {
      inc(registers.E);
      ++cycles;
      incrementPC();
      break;
    }

    // Increment the contents of register L by 1.
    case Instruction::Type::INC_L: {
      inc(registers.L);
      ++cycles;
      incrementPC();
      break;
    }

    // Increment the contents of memory specified by register pair HL by 1.
    case Instruction::Type::INC_mem_HL: {
      auto data = memory->readByte(registers.get_HL());
      registers.F.halfCarry = (data & 0xF) == 0xF;
      ++data;
      registers.F.zero = data == 0;
      registers.F.subtraction = false;
      memory->writeByte(registers.get_HL(), data);

      cycles += 3;
      incrementPC();
      break;
    }

    // Decrement the conents of memory specified by register pair HL by 1.
    case Instruction::Type::DEC_mem_HL: {
      uint8_t data = memory->readByte(registers.get_HL());
      --data;
      registers.F.zero = data == 0;
      registers.F.subtraction = true;
      registers.F.halfCarry = (data & 0xF) == 0xF;
      memory->writeByte(registers.get_HL(), data);

      cycles += 3;
      incrementPC();
      break;
    }

    // Decrement the contents of register SP by 1
    case Instruction::Type::DEC_SP: {
      --SP;
      cycles += 2;
      incrementPC();
      break;
    }

    // Decrement the contents of register DE by 1
    case Instruction::Type::DEC_DE: {
      registers.set_DE(registers.get_DE() - 1);
      cycles += 2;
      incrementPC();
      break;
    }

    // Decrement the contents of register BC by 1
    case Instruction::Type::DEC_BC: {
      registers.set_BC(registers.get_BC() - 1);
      cycles += 2;
      incrementPC();
      break;
    }

    // Decrement the contents of register HL by 1
    case Instruction::Type::DEC_HL: {
      registers.set_HL(registers.get_HL() - 1);
      cycles += 2;
      incrementPC();
      break;
    }

    // Increment the contents of register A by 1.
    case Instruction::Type::INC_A: {
      inc(registers.A);
      ++cycles;
      incrementPC();
      break;
    }

    // Load the 8-bit immediate operand d8 into register H.
    case Instruction::Type::LD_H_d8: {
      registers.H = memory->readByte(PC + 1);
      cycles += 2;
      setPC(PC + 2);
      break;
    }

    // Store the contents of 8-bit immediate operand d8 in the memory location
    // specified by register pair HL.
    case Instruction::Type::LD_HL_d8: {
      memory->writeByte(registers.get_HL(), memory->readByte(PC + 1));
      cycles += 3;
      setPC(PC + 2);
      break;
    }

    // Rotate the contents of register A to the right
    case Instruction::Type::RRCA: {
      rrc(registers.A);
      registers.F.zero = false;
      ++cycles;
      incrementPC();
      break;
    }

    // Rotate the contents of register A to the left
    case Instruction::Type::RLCA: {
      rlc(registers.A);
      registers.F.zero = false;
      ++cycles;
      incrementPC();
      break;
    }

    case Instruction::Type::STOP: {
      halted = true;
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADC_A_A: {
      adc(registers.A);
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADC_A_B: {
      adc(registers.B);
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADC_A_C: {
      adc(registers.C);
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADC_A_D: {
      adc(registers.D);
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADC_A_E: {
      adc(registers.E);
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADC_A_H: {
      adc(registers.H);
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADC_A_L: {
      adc(registers.L);
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADC_A_HL: {
      adc(memory->readByte(registers.get_HL()));
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADD_A_A: {
      add(registers.A);
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADD_A_B: {
      add(registers.B);
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADD_A_C: {
      add(registers.C);
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADD_A_D: {
      add(registers.D);
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADD_A_E: {
      add(registers.E);
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADD_A_H: {
      add(registers.H);
      incrementPC();
      ++cycles;
      break;
    }

    case Instruction::Type::ADD_A_L: {
      add(registers.L);
      incrementPC();
      ++cycles;
      break;
    }

    // Store the contents of register A in the memory location specified by
    // register pair DE.
    case Instruction::Type::LD_DE_A: {
      memory->writeByte(registers.get_DE(), registers.A);
      incrementPC();
      cycles += 2;
      break;
    }

      // =============================
      // === Prefixed instructions ===
      // =============================

    // Copy the complement of the contents of bit 7 in register H to the Z
    // flag of the program status word (PSW).
    case Instruction::Type::BIT_7_H: {
      bit(registers.H, 7);
      setPC(PC + 2);
      break;
    }

    // Rotate the contents of register C to the left.
    case Instruction::Type::RL_C: {
      rl(registers.C);
      setPC(PC + 2);
      break;
    }

    default: {
      throw std::runtime_error(
          std::string(
              "Encountered invalid instruction type. Instruction Type: ") +
          instruction->TypeRepr());
    }
  }

  return PC;
}

void CPU::step() {
  uint16_t opcode = memory->readByte(PC);

  bool isPrefixed = opcode == 0xCB;
  if (isPrefixed) opcode = memory->readByte(PC + 1);

  Instruction instruction = Instruction(opcode, isPrefixed);
  PC = executeInstruction(&instruction);
}

// ======================
// ==== INSTRUCTIONS ====
// ======================

// ADD r: Add (register)
// Adds to the 8-bit A register, the 8-bit register r, and stores the result
// back into the A register.
uint8_t CPU::add(uint8_t value) {
  uint16_t n = uint16_t(registers.A) + uint16_t(value);
  registers.F.zero = (n & 0xFF) == 0;
  registers.F.halfCarry =
      (((registers.A & 0x0F) + (value & 0x0F)) & 0x10) == 0x10;
  registers.F.carry = n > 0xFF;
  registers.F.subtraction = false;
  registers.A = uint8_t(n & 0xFF);
  return registers.A;
}

uint8_t CPU::adc(uint8_t value) {
  uint16_t n = registers.A + value + (registers.F.carry ? 1 : 0);
  registers.F.zero = (uint8_t)n == 0;
  registers.F.halfCarry = (registers.A ^ value ^ n) & 0x10 ? 1 : 0;
  registers.F.carry = n & 0x100 ? 1 : 0;
  registers.F.subtraction = false;
  registers.A = (uint8_t)n;

  return registers.A;
}

void CPU::cp(uint8_t value) {
  registers.F.zero = registers.A == value;
  registers.F.halfCarry = (registers.A & 0xF) < (value & 0xF);
  registers.F.subtraction = true;
  registers.F.carry = registers.A < value;
}

// Subtract the contents of a register from the contents of register A, and
// store the results in register A.
uint8_t CPU::sub(uint8_t value) {
  cp(value);
  registers.A -= registers.B;
  return registers.A;
}

// Push value to the stack
void CPU::push(uint16_t value) {
  memory->writeByte(--SP, (uint8_t)((value & 0xFF00) >> 8));
  memory->writeByte(--SP, (uint8_t)(value & 0xFF));
}

// Pop value from the stack
uint16_t CPU::pop() {
  uint16_t lsb = (uint16_t)memory->readByte(SP++);
  uint16_t msb = (uint16_t)memory->readByte(SP++);
  return (msb << 8) | lsb;
}

// Rotate bits left
uint8_t CPU::rl(uint8_t &reg) {
  bool oldCarry = registers.F.carry;
  registers.F.carry = (reg & (1 << 7)) == (1 << 7);
  registers.F.halfCarry = false;
  registers.F.subtraction = false;
  reg <<= 1;
  if (oldCarry) reg |= 0x1;
  registers.F.zero = reg == 0;
  return reg;
}

// Rotate bits right
uint8_t CPU::rr(uint8_t &reg) {
  bool oldCarry = registers.F.carry;
  registers.F.carry = (reg & 0x1) == 0x1;
  registers.F.halfCarry = false;
  registers.F.subtraction = false;
  reg >>= 1;
  if (oldCarry) reg |= (1 << 7);
  registers.F.zero = reg == 0;
  return reg;
}

uint8_t CPU::rrc(uint8_t &reg) {
  registers.F.carry = (reg & 0x1) == 0x1;
  registers.F.halfCarry = false;
  registers.F.subtraction = false;
  reg >>= 1;
  if (registers.F.carry) {
    reg |= (1 << 7);
  }
  registers.F.zero = reg == 0;
  return reg;
}

// Rotate bits left through carry flag
uint8_t CPU::rlc(uint8_t &reg) {
  registers.F.carry = (reg & (1 << 7)) == (1 << 7);
  registers.F.halfCarry = false;
  registers.F.subtraction = false;
  reg <<= 1;
  if (registers.F.carry) reg |= 0x1;
  registers.F.zero = reg == 0;
  return reg;
}

// Increment a register
uint8_t CPU::inc(uint8_t &reg) {
  registers.F.halfCarry = (reg & 0xF) == 0xF;
  ++reg;
  registers.F.zero = reg == 0;
  registers.F.subtraction = false;
  return reg;
}

// Decrement a register
uint8_t CPU::dec(uint8_t &reg) {
  --reg;
  registers.F.zero = reg == 0;
  registers.F.subtraction = true;
  registers.F.halfCarry = (reg & 0xF) == 0xF;
  return reg;
}

void CPU::bit(uint8_t value, uint8_t b) {
  registers.F.zero = (value & (1 << b)) == 0;
  registers.F.subtraction = false;
  registers.F.halfCarry = true;
}

uint16_t CPU::addCompoundRegisters(uint16_t a, uint16_t b) {
  uint32_t temp = uint32_t(a) + uint32_t(b);
  registers.F.subtraction = false;
  registers.F.carry = temp > 0xFFFF;
  registers.F.halfCarry = ((a & 0x0FFF) + (b & 0x0FFF)) > 0x0FFF;
  registers.set_HL(temp & 0xFFFF);
  return temp & 0xFFFF;
};

uint8_t CPU::xor_(uint8_t reg) {
  registers.A ^= reg;
  registers.F.carry = false;
  registers.F.halfCarry = false;
  registers.F.subtraction = false;
  registers.F.zero = registers.A == 0;
  return reg;
};

uint8_t CPU::or_(uint8_t reg) {
  registers.A |= reg;
  registers.F.zero = registers.A == 0;
  registers.F.halfCarry = false;
  registers.F.subtraction = false;
  registers.F.carry = false;
  return registers.A;
};

uint8_t CPU::and_(uint8_t reg) {
  registers.A &= reg;
  registers.F.zero = registers.A == 0;
  registers.F.halfCarry = true;
  registers.F.subtraction = false;
  registers.F.carry = false;
  return registers.A;
};

void CPU::rst(uint8_t addr) {
  push(addr);
  PC = uint16_t(addr);
};

// -------------------------------
// -- Instruction class methods --
// -------------------------------

// Generate an instruction object given an opcode.
Instruction::Instruction(uint16_t opcode, bool isPrefixed) {
  if (!isPrefixed) {
    switch (opcode) {
      case (int)Type::NOP:
      case (int)Type::LD_BC_d16:
      case (int)Type::LD_BC_A:
      case (int)Type::INC_BC:
      case (int)Type::INC_B:
      case (int)Type::DEC_B:
      case (int)Type::LD_B_d8:
      case (int)Type::RLCA:
      case (int)Type::LD_a16_SP:
      case (int)Type::ADD_HL_BC:
      case (int)Type::LD_A_BC:
      case (int)Type::DEC_BC:
      case (int)Type::INC_C:
      case (int)Type::DEC_C:
      case (int)Type::LD_C_d8:
      case (int)Type::RRCA:
      case (int)Type::STOP:
      case (int)Type::LD_DE_d16:
      case (int)Type::LD_DE_A:
      case (int)Type::INC_DE:
      case (int)Type::INC_D:
      case (int)Type::DEC_D:
      case (int)Type::LD_D_d8:
      case (int)Type::RLA:
      case (int)Type::JR_s8:
      case (int)Type::ADD_HL_DE:
      case (int)Type::LD_A_DE:
      case (int)Type::DEC_DE:
      case (int)Type::INC_E:
      case (int)Type::DEC_E:
      case (int)Type::LD_E_d8:
      case (int)Type::RRA:
      case (int)Type::JR_NZ_s8:
      case (int)Type::LD_HL_d16:
      case (int)Type::LD_HL_inc__A:
      case (int)Type::INC_HL:
      case (int)Type::INC_H:
      case (int)Type::DEC_H:
      case (int)Type::LD_H_d8:
      case (int)Type::DAA:
      case (int)Type::JR_Z_s8:
      case (int)Type::ADD_HL_HL:
      case (int)Type::LD_A_HL_inc_:
      case (int)Type::DEC_HL:
      case (int)Type::INC_L:
      case (int)Type::DEC_L:
      case (int)Type::LD_L_d8:
      case (int)Type::CPL:
      case (int)Type::JR_NC_s8:
      case (int)Type::LD_SP_d16:
      case (int)Type::LD_HL_dec_A:
      case (int)Type::INC_SP:
      case (int)Type::INC_mem_HL:
      case (int)Type::DEC_mem_HL:
      case (int)Type::LD_HL_d8:
      case (int)Type::SCF:
      case (int)Type::JR_C_s8:
      case (int)Type::ADD_HL_SP:
      case (int)Type::LD_A_HL_dec_:
      case (int)Type::DEC_SP:
      case (int)Type::INC_A:
      case (int)Type::DEC_A:
      case (int)Type::LD_A_d8:
      case (int)Type::CCF:
      case (int)Type::LD_B_B:
      case (int)Type::LD_B_C:
      case (int)Type::LD_B_D:
      case (int)Type::LD_B_E:
      case (int)Type::LD_B_H:
      case (int)Type::LD_B_L:
      case (int)Type::LD_B_HL:
      case (int)Type::LD_B_A:
      case (int)Type::LD_C_B:
      case (int)Type::LD_C_C:
      case (int)Type::LD_C_D:
      case (int)Type::LD_C_E:
      case (int)Type::LD_C_H:
      case (int)Type::LD_C_L:
      case (int)Type::LD_C_HL:
      case (int)Type::LD_C_A:
      case (int)Type::LD_D_B:
      case (int)Type::LD_D_C:
      case (int)Type::LD_D_D:
      case (int)Type::LD_D_E:
      case (int)Type::LD_D_H:
      case (int)Type::LD_D_L:
      case (int)Type::LD_D_HL:
      case (int)Type::LD_D_A:
      case (int)Type::LD_E_B:
      case (int)Type::LD_E_C:
      case (int)Type::LD_E_D:
      case (int)Type::LD_E_E:
      case (int)Type::LD_E_H:
      case (int)Type::LD_E_L:
      case (int)Type::LD_E_HL:
      case (int)Type::LD_E_A:
      case (int)Type::LD_H_B:
      case (int)Type::LD_H_C:
      case (int)Type::LD_H_D:
      case (int)Type::LD_H_E:
      case (int)Type::LD_H_H:
      case (int)Type::LD_H_L:
      case (int)Type::LD_H_HL:
      case (int)Type::LD_H_A:
      case (int)Type::LD_L_B:
      case (int)Type::LD_L_C:
      case (int)Type::LD_L_D:
      case (int)Type::LD_L_E:
      case (int)Type::LD_L_H:
      case (int)Type::LD_L_L:
      case (int)Type::LD_L_HL:
      case (int)Type::LD_L_A:
      case (int)Type::LD_HL_B:
      case (int)Type::LD_HL_C:
      case (int)Type::LD_HL_D:
      case (int)Type::LD_HL_E:
      case (int)Type::LD_HL_H:
      case (int)Type::LD_HL_L:
      case (int)Type::HALT:
      case (int)Type::LD_HL_A:
      case (int)Type::LD_A_B:
      case (int)Type::LD_A_C:
      case (int)Type::LD_A_D:
      case (int)Type::LD_A_E:
      case (int)Type::LD_A_H:
      case (int)Type::LD_A_L:
      case (int)Type::LD_A_HL:
      case (int)Type::LD_A_A:
      case (int)Type::ADD_A_B:
      case (int)Type::ADD_A_C:
      case (int)Type::ADD_A_D:
      case (int)Type::ADD_A_E:
      case (int)Type::ADD_A_H:
      case (int)Type::ADD_A_L:
      case (int)Type::ADD_A_HL:
      case (int)Type::ADD_A_A:
      case (int)Type::ADC_A_B:
      case (int)Type::ADC_A_C:
      case (int)Type::ADC_A_D:
      case (int)Type::ADC_A_E:
      case (int)Type::ADC_A_H:
      case (int)Type::ADC_A_L:
      case (int)Type::ADC_A_HL:
      case (int)Type::ADC_A_A:
      case (int)Type::SUB_B:
      case (int)Type::SUB_C:
      case (int)Type::SUB_D:
      case (int)Type::SUB_E:
      case (int)Type::SUB_H:
      case (int)Type::SUB_L:
      case (int)Type::SUB_HL:
      case (int)Type::SUB_A:
      case (int)Type::SBC_A_B:
      case (int)Type::SBC_A_C:
      case (int)Type::SBC_A_D:
      case (int)Type::SBC_A_E:
      case (int)Type::SBC_A_H:
      case (int)Type::SBC_A_L:
      case (int)Type::SBC_A_HL:
      case (int)Type::SBC_A_A:
      case (int)Type::AND_B:
      case (int)Type::AND_C:
      case (int)Type::AND_D:
      case (int)Type::AND_E:
      case (int)Type::AND_H:
      case (int)Type::AND_L:
      case (int)Type::AND_HL:
      case (int)Type::AND_A:
      case (int)Type::XOR_B:
      case (int)Type::XOR_C:
      case (int)Type::XOR_D:
      case (int)Type::XOR_E:
      case (int)Type::XOR_H:
      case (int)Type::XOR_L:
      case (int)Type::XOR_HL:
      case (int)Type::XOR_A:
      case (int)Type::OR_B:
      case (int)Type::OR_C:
      case (int)Type::OR_D:
      case (int)Type::OR_E:
      case (int)Type::OR_H:
      case (int)Type::OR_L:
      case (int)Type::OR_HL:
      case (int)Type::OR_A:
      case (int)Type::CP_B:
      case (int)Type::CP_C:
      case (int)Type::CP_D:
      case (int)Type::CP_E:
      case (int)Type::CP_H:
      case (int)Type::CP_L:
      case (int)Type::CP_HL:
      case (int)Type::CP_A:
      case (int)Type::RET_NZ:
      case (int)Type::POP_BC:
      case (int)Type::JP_NZ_a16:
      case (int)Type::JP_a16:
      case (int)Type::CALL_NZ_a16:
      case (int)Type::PUSH_BC:
      case (int)Type::ADD_A_d8:
      case (int)Type::RST_0:
      case (int)Type::RET_Z:
      case (int)Type::RET:
      case (int)Type::JP_Z_a16:
      case (int)Type::CALL_Z_a16:
      case (int)Type::CALL_a16:
      case (int)Type::ADC_A_d8:
      case (int)Type::RST_1:
      case (int)Type::RET_NC:
      case (int)Type::POP_DE:
      case (int)Type::JP_NC_a16:
      case (int)Type::CALL_NC_a16:
      case (int)Type::PUSH_DE:
      case (int)Type::SUB_d8:
      case (int)Type::RST_2:
      case (int)Type::RET_C:
      case (int)Type::RETI:
      case (int)Type::JP_C_a16:
      case (int)Type::CALL_C_a16:
      case (int)Type::SBC_A_d8:
      case (int)Type::RST_3:
      case (int)Type::LD_a8_A:
      case (int)Type::POP_HL:
      case (int)Type::LD_mem_C_A:
      case (int)Type::PUSH_HL:
      case (int)Type::AND_d8:
      case (int)Type::RST_4:
      case (int)Type::ADD_SP_s8:
      case (int)Type::JP_HL:
      case (int)Type::LD_a16_A:
      case (int)Type::XOR_d8:
      case (int)Type::RST_5:
      case (int)Type::LD_A_a8:
      case (int)Type::POP_AF:
      case (int)Type::LD_A_mem_C:
      case (int)Type::DI:
      case (int)Type::PUSH_AF:
      case (int)Type::OR_d8:
      case (int)Type::RST_6:
      case (int)Type::LD_HL_SP_inc_s8:
      case (int)Type::LD_SP_HL:
      case (int)Type::LD_A_a16:
      case (int)Type::EI:
      case (int)Type::CP_d8:
      case (int)Type::RST_7:
        type_ = (Type)opcode;
        break;

      default: {
        throw std::runtime_error(
            std::string("Invalid non-prefixed instruction opcode: ") +
            TypeRepr());
      }
    }
  } else {
    switch (0xCB00 | opcode) {
      // Prefixed instructions
      case (int)Type::RLC_B:
      case (int)Type::RLC_C:
      case (int)Type::RLC_D:
      case (int)Type::RLC_E:
      case (int)Type::RLC_H:
      case (int)Type::RLC_L:
      case (int)Type::RLC_HL:
      case (int)Type::RLC_A:
      case (int)Type::RRC_B:
      case (int)Type::RRC_C:
      case (int)Type::RRC_D:
      case (int)Type::RRC_E:
      case (int)Type::RRC_H:
      case (int)Type::RRC_L:
      case (int)Type::RRC_HL:
      case (int)Type::RRC_A:
      case (int)Type::RL_B:
      case (int)Type::RL_C:
      case (int)Type::RL_D:
      case (int)Type::RL_E:
      case (int)Type::RL_H:
      case (int)Type::RL_L:
      case (int)Type::RL_HL:
      case (int)Type::RL_A:
      case (int)Type::RR_B:
      case (int)Type::RR_C:
      case (int)Type::RR_D:
      case (int)Type::RR_E:
      case (int)Type::RR_H:
      case (int)Type::RR_L:
      case (int)Type::RR_HL:
      case (int)Type::RR_A:
      case (int)Type::SLA_B:
      case (int)Type::SLA_C:
      case (int)Type::SLA_D:
      case (int)Type::SLA_E:
      case (int)Type::SLA_H:
      case (int)Type::SLA_L:
      case (int)Type::SLA_HL:
      case (int)Type::SLA_A:
      case (int)Type::SRA_B:
      case (int)Type::SRA_C:
      case (int)Type::SRA_D:
      case (int)Type::SRA_E:
      case (int)Type::SRA_H:
      case (int)Type::SRA_L:
      case (int)Type::SRA_HL:
      case (int)Type::SRA_A:
      case (int)Type::SWAP_B:
      case (int)Type::SWAP_C:
      case (int)Type::SWAP_D:
      case (int)Type::SWAP_E:
      case (int)Type::SWAP_H:
      case (int)Type::SWAP_L:
      case (int)Type::SWAP_HL:
      case (int)Type::SWAP_A:
      case (int)Type::SRL_B:
      case (int)Type::SRL_C:
      case (int)Type::SRL_D:
      case (int)Type::SRL_E:
      case (int)Type::SRL_H:
      case (int)Type::SRL_L:
      case (int)Type::SRL_HL:
      case (int)Type::SRL_A:
      case (int)Type::BIT_0_B:
      case (int)Type::BIT_0_C:
      case (int)Type::BIT_0_D:
      case (int)Type::BIT_0_E:
      case (int)Type::BIT_0_H:
      case (int)Type::BIT_0_L:
      case (int)Type::BIT_0_HL:
      case (int)Type::BIT_0_A:
      case (int)Type::BIT_1_B:
      case (int)Type::BIT_1_C:
      case (int)Type::BIT_1_D:
      case (int)Type::BIT_1_E:
      case (int)Type::BIT_1_H:
      case (int)Type::BIT_1_L:
      case (int)Type::BIT_1_HL:
      case (int)Type::BIT_1_A:
      case (int)Type::BIT_2_B:
      case (int)Type::BIT_2_C:
      case (int)Type::BIT_2_D:
      case (int)Type::BIT_2_E:
      case (int)Type::BIT_2_H:
      case (int)Type::BIT_2_L:
      case (int)Type::BIT_2_HL:
      case (int)Type::BIT_2_A:
      case (int)Type::BIT_3_B:
      case (int)Type::BIT_3_C:
      case (int)Type::BIT_3_D:
      case (int)Type::BIT_3_E:
      case (int)Type::BIT_3_H:
      case (int)Type::BIT_3_L:
      case (int)Type::BIT_3_HL:
      case (int)Type::BIT_3_A:
      case (int)Type::BIT_4_B:
      case (int)Type::BIT_4_C:
      case (int)Type::BIT_4_D:
      case (int)Type::BIT_4_E:
      case (int)Type::BIT_4_H:
      case (int)Type::BIT_4_L:
      case (int)Type::BIT_4_HL:
      case (int)Type::BIT_4_A:
      case (int)Type::BIT_5_B:
      case (int)Type::BIT_5_C:
      case (int)Type::BIT_5_D:
      case (int)Type::BIT_5_E:
      case (int)Type::BIT_5_H:
      case (int)Type::BIT_5_L:
      case (int)Type::BIT_5_HL:
      case (int)Type::BIT_5_A:
      case (int)Type::BIT_6_B:
      case (int)Type::BIT_6_C:
      case (int)Type::BIT_6_D:
      case (int)Type::BIT_6_E:
      case (int)Type::BIT_6_H:
      case (int)Type::BIT_6_L:
      case (int)Type::BIT_6_HL:
      case (int)Type::BIT_6_A:
      case (int)Type::BIT_7_B:
      case (int)Type::BIT_7_C:
      case (int)Type::BIT_7_D:
      case (int)Type::BIT_7_E:
      case (int)Type::BIT_7_H:
      case (int)Type::BIT_7_L:
      case (int)Type::BIT_7_HL:
      case (int)Type::BIT_7_A:
      case (int)Type::RES_0_B:
      case (int)Type::RES_0_C:
      case (int)Type::RES_0_D:
      case (int)Type::RES_0_E:
      case (int)Type::RES_0_H:
      case (int)Type::RES_0_L:
      case (int)Type::RES_0_HL:
      case (int)Type::RES_0_A:
      case (int)Type::RES_1_B:
      case (int)Type::RES_1_C:
      case (int)Type::RES_1_D:
      case (int)Type::RES_1_E:
      case (int)Type::RES_1_H:
      case (int)Type::RES_1_L:
      case (int)Type::RES_1_HL:
      case (int)Type::RES_1_A:
      case (int)Type::RES_2_B:
      case (int)Type::RES_2_C:
      case (int)Type::RES_2_D:
      case (int)Type::RES_2_E:
      case (int)Type::RES_2_H:
      case (int)Type::RES_2_L:
      case (int)Type::RES_2_HL:
      case (int)Type::RES_2_A:
      case (int)Type::RES_3_B:
      case (int)Type::RES_3_C:
      case (int)Type::RES_3_D:
      case (int)Type::RES_3_E:
      case (int)Type::RES_3_H:
      case (int)Type::RES_3_L:
      case (int)Type::RES_3_HL:
      case (int)Type::RES_3_A:
      case (int)Type::RES_4_B:
      case (int)Type::RES_4_C:
      case (int)Type::RES_4_D:
      case (int)Type::RES_4_E:
      case (int)Type::RES_4_H:
      case (int)Type::RES_4_L:
      case (int)Type::RES_4_HL:
      case (int)Type::RES_4_A:
      case (int)Type::RES_5_B:
      case (int)Type::RES_5_C:
      case (int)Type::RES_5_D:
      case (int)Type::RES_5_E:
      case (int)Type::RES_5_H:
      case (int)Type::RES_5_L:
      case (int)Type::RES_5_HL:
      case (int)Type::RES_5_A:
      case (int)Type::RES_6_B:
      case (int)Type::RES_6_C:
      case (int)Type::RES_6_D:
      case (int)Type::RES_6_E:
      case (int)Type::RES_6_H:
      case (int)Type::RES_6_L:
      case (int)Type::RES_6_HL:
      case (int)Type::RES_6_A:
      case (int)Type::RES_7_B:
      case (int)Type::RES_7_C:
      case (int)Type::RES_7_D:
      case (int)Type::RES_7_E:
      case (int)Type::RES_7_H:
      case (int)Type::RES_7_L:
      case (int)Type::RES_7_HL:
      case (int)Type::RES_7_A:
      case (int)Type::SET_0_B:
      case (int)Type::SET_0_C:
      case (int)Type::SET_0_D:
      case (int)Type::SET_0_E:
      case (int)Type::SET_0_H:
      case (int)Type::SET_0_L:
      case (int)Type::SET_0_HL:
      case (int)Type::SET_0_A:
      case (int)Type::SET_1_B:
      case (int)Type::SET_1_C:
      case (int)Type::SET_1_D:
      case (int)Type::SET_1_E:
      case (int)Type::SET_1_H:
      case (int)Type::SET_1_L:
      case (int)Type::SET_1_HL:
      case (int)Type::SET_1_A:
      case (int)Type::SET_2_B:
      case (int)Type::SET_2_C:
      case (int)Type::SET_2_D:
      case (int)Type::SET_2_E:
      case (int)Type::SET_2_H:
      case (int)Type::SET_2_L:
      case (int)Type::SET_2_HL:
      case (int)Type::SET_2_A:
      case (int)Type::SET_3_B:
      case (int)Type::SET_3_C:
      case (int)Type::SET_3_D:
      case (int)Type::SET_3_E:
      case (int)Type::SET_3_H:
      case (int)Type::SET_3_L:
      case (int)Type::SET_3_HL:
      case (int)Type::SET_3_A:
      case (int)Type::SET_4_B:
      case (int)Type::SET_4_C:
      case (int)Type::SET_4_D:
      case (int)Type::SET_4_E:
      case (int)Type::SET_4_H:
      case (int)Type::SET_4_L:
      case (int)Type::SET_4_HL:
      case (int)Type::SET_4_A:
      case (int)Type::SET_5_B:
      case (int)Type::SET_5_C:
      case (int)Type::SET_5_D:
      case (int)Type::SET_5_E:
      case (int)Type::SET_5_H:
      case (int)Type::SET_5_L:
      case (int)Type::SET_5_HL:
      case (int)Type::SET_5_A:
      case (int)Type::SET_6_B:
      case (int)Type::SET_6_C:
      case (int)Type::SET_6_D:
      case (int)Type::SET_6_E:
      case (int)Type::SET_6_H:
      case (int)Type::SET_6_L:
      case (int)Type::SET_6_HL:
      case (int)Type::SET_6_A:
      case (int)Type::SET_7_B:
      case (int)Type::SET_7_C:
      case (int)Type::SET_7_D:
      case (int)Type::SET_7_E:
      case (int)Type::SET_7_H:
      case (int)Type::SET_7_L:
      case (int)Type::SET_7_HL:
      case (int)Type::SET_7_A:
        type_ = (Type)opcode;
        break;

      default: {
        throw std::runtime_error(
            std::string("Invalid prefixed instruction opcode: ") +
            std::to_string(opcode));
      }
    }
  }
}

std::string Instruction::ArithmeticTargetRepr() {
  switch (arithmeticTarget_) {
    case A:
      return "A";
    case B:
      return "B";
    case C:
      return "C";
    case D:
      return "D";
    case E:
      return "E";
    case H:
      return "H";
    case L:
      return "L";
  }
}

std::string Instruction::TypeRepr() {
  switch (type_) {
    case Type::NOP:
      return "0x00__NOP";
    case Type::LD_BC_d16:
      return "0x01__LD_BC_d16";
    case Type::LD_BC_A:
      return "0x02__LD_BC_A";
    case Type::INC_BC:
      return "0x03__INC_BC";
    case Type::INC_B:
      return "0x04__INC_B";
    case Type::DEC_B:
      return "0x05__DEC_B";
    case Type::LD_B_d8:
      return "0x06__LD_B_d8";
    case Type::RLCA:
      return "0x07__RLCA";
    case Type::LD_a16_SP:
      return "0x08__LD_a16_SP";
    case Type::ADD_HL_BC:
      return "0x09__ADD_HL_BC";
    case Type::LD_A_BC:
      return "0x0A__LD_A_BC";
    case Type::DEC_BC:
      return "0x0B__DEC_BC";
    case Type::INC_C:
      return "0x0C__INC_C";
    case Type::DEC_C:
      return "0x0D__DEC_C";
    case Type::LD_C_d8:
      return "0x0E__LD_C_d8";
    case Type::RRCA:
      return "0x0F__RRCA";
    case Type::STOP:
      return "0x1000__STOP";
    case Type::LD_DE_d16:
      return "0x11__LD_DE_d16";
    case Type::LD_DE_A:
      return "0x12__LD_DE_A";
    case Type::INC_DE:
      return "0x13__INC_DE";
    case Type::INC_D:
      return "0x14__INC_D";
    case Type::DEC_D:
      return "0x15__DEC_D";
    case Type::LD_D_d8:
      return "0x16__LD_D_d8";
    case Type::RLA:
      return "0x17__RLA";
    case Type::JR_s8:
      return "0x18__JR_s8";
    case Type::ADD_HL_DE:
      return "0x19__ADD_HL_DE";
    case Type::LD_A_DE:
      return "0x1A__LD_A_DE";
    case Type::DEC_DE:
      return "0x1B__DEC_DE";
    case Type::INC_E:
      return "0x1C__INC_E";
    case Type::DEC_E:
      return "0x1D__DEC_E";
    case Type::LD_E_d8:
      return "0x1E__LD_E_d8";
    case Type::RRA:
      return "0x1F__RRA";
    case Type::JR_NZ_s8:
      return "0x20__JR_NZ_s8";
    case Type::LD_HL_d16:
      return "0x21__LD_HL_d16";
    case Type::LD_HL_inc__A:
      return "0x22__LD_HL_inc__A";
    case Type::INC_HL:
      return "0x23__INC_HL";
    case Type::INC_H:
      return "0x24__INC_H";
    case Type::DEC_H:
      return "0x25__DEC_H";
    case Type::LD_H_d8:
      return "0x26__LD_H_d8";
    case Type::DAA:
      return "0x27__DAA";
    case Type::JR_Z_s8:
      return "0x28__JR_Z_s8";
    case Type::ADD_HL_HL:
      return "0x29__ADD_HL_HL";
    case Type::LD_A_HL_inc_:
      return "0x2A__LD_A_HL_inc_";
    case Type::DEC_HL:
      return "0x2B__DEC_HL";
    case Type::INC_L:
      return "0x2C__INC_L";
    case Type::DEC_L:
      return "0x2D__DEC_L";
    case Type::LD_L_d8:
      return "0x2E__LD_L_d8";
    case Type::CPL:
      return "0x2F__CPL";
    case Type::JR_NC_s8:
      return "0x30__JR_NC_s8";
    case Type::LD_SP_d16:
      return "0x31__LD_SP_d16";
    case Type::LD_HL_dec_A:
      return "0x32__LD_HL_dec__A";
    case Type::INC_SP:
      return "0x33__INC_SP";
    case Type::INC_mem_HL:
      return "0x34__INC_mem_HL";
    case Type::DEC_mem_HL:
      return "0x35__DEC_mem_HL";
    case Type::LD_HL_d8:
      return "0x36__LD_HL_d8";
    case Type::SCF:
      return "0x37__SCF";
    case Type::JR_C_s8:
      return "0x38__JR_C_s8";
    case Type::ADD_HL_SP:
      return "0x39__ADD_HL_SP";
    case Type::LD_A_HL_dec_:
      return "0x3A__LD_A_HL_dec_";
    case Type::DEC_SP:
      return "0x3B__DEC_SP";
    case Type::INC_A:
      return "0x3C__INC_A";
    case Type::DEC_A:
      return "0x3D__DEC_A";
    case Type::LD_A_d8:
      return "0x3E__LD_A_d8";
    case Type::CCF:
      return "0x3F__CCF";
    case Type::LD_B_B:
      return "0x40__LD_B_B";
    case Type::LD_B_C:
      return "0x41__LD_B_C";
    case Type::LD_B_D:
      return "0x42__LD_B_D";
    case Type::LD_B_E:
      return "0x43__LD_B_E";
    case Type::LD_B_H:
      return "0x44__LD_B_H";
    case Type::LD_B_L:
      return "0x45__LD_B_L";
    case Type::LD_B_HL:
      return "0x46__LD_B_HL";
    case Type::LD_B_A:
      return "0x47__LD_B_A";
    case Type::LD_C_B:
      return "0x48__LD_C_B";
    case Type::LD_C_C:
      return "0x49__LD_C_C";
    case Type::LD_C_D:
      return "0x4A__LD_C_D";
    case Type::LD_C_E:
      return "0x4B__LD_C_E";
    case Type::LD_C_H:
      return "0x4C__LD_C_H";
    case Type::LD_C_L:
      return "0x4D__LD_C_L";
    case Type::LD_C_HL:
      return "0x4E__LD_C_HL";
    case Type::LD_C_A:
      return "0x4F__LD_C_A";
    case Type::LD_D_B:
      return "0x50__LD_D_B";
    case Type::LD_D_C:
      return "0x51__LD_D_C";
    case Type::LD_D_D:
      return "0x52__LD_D_D";
    case Type::LD_D_E:
      return "0x53__LD_D_E";
    case Type::LD_D_H:
      return "0x54__LD_D_H";
    case Type::LD_D_L:
      return "0x55__LD_D_L";
    case Type::LD_D_HL:
      return "0x56__LD_D_HL";
    case Type::LD_D_A:
      return "0x57__LD_D_A";
    case Type::LD_E_B:
      return "0x58__LD_E_B";
    case Type::LD_E_C:
      return "0x59__LD_E_C";
    case Type::LD_E_D:
      return "0x5A__LD_E_D";
    case Type::LD_E_E:
      return "0x5B__LD_E_E";
    case Type::LD_E_H:
      return "0x5C__LD_E_H";
    case Type::LD_E_L:
      return "0x5D__LD_E_L";
    case Type::LD_E_HL:
      return "0x5E__LD_E_HL";
    case Type::LD_E_A:
      return "0x5F__LD_E_A";
    case Type::LD_H_B:
      return "0x60__LD_H_B";
    case Type::LD_H_C:
      return "0x61__LD_H_C";
    case Type::LD_H_D:
      return "0x62__LD_H_D";
    case Type::LD_H_E:
      return "0x63__LD_H_E";
    case Type::LD_H_H:
      return "0x64__LD_H_H";
    case Type::LD_H_L:
      return "0x65__LD_H_L";
    case Type::LD_H_HL:
      return "0x66__LD_H_HL";
    case Type::LD_H_A:
      return "0x67__LD_H_A";
    case Type::LD_L_B:
      return "0x68__LD_L_B";
    case Type::LD_L_C:
      return "0x69__LD_L_C";
    case Type::LD_L_D:
      return "0x6A__LD_L_D";
    case Type::LD_L_E:
      return "0x6B__LD_L_E";
    case Type::LD_L_H:
      return "0x6C__LD_L_H";
    case Type::LD_L_L:
      return "0x6D__LD_L_L";
    case Type::LD_L_HL:
      return "0x6E__LD_L_HL";
    case Type::LD_L_A:
      return "0x6F__LD_L_A";
    case Type::LD_HL_B:
      return "0x70__LD_HL_B";
    case Type::LD_HL_C:
      return "0x71__LD_HL_C";
    case Type::LD_HL_D:
      return "0x72__LD_HL_D";
    case Type::LD_HL_E:
      return "0x73__LD_HL_E";
    case Type::LD_HL_H:
      return "0x74__LD_HL_H";
    case Type::LD_HL_L:
      return "0x75__LD_HL_L";
    case Type::HALT:
      return "0x76__HALT";
    case Type::LD_HL_A:
      return "0x77__LD_HL_A";
    case Type::LD_A_B:
      return "0x78__LD_A_B";
    case Type::LD_A_C:
      return "0x79__LD_A_C";
    case Type::LD_A_D:
      return "0x7A__LD_A_D";
    case Type::LD_A_E:
      return "0x7B__LD_A_E";
    case Type::LD_A_H:
      return "0x7C__LD_A_H";
    case Type::LD_A_L:
      return "0x7D__LD_A_L";
    case Type::LD_A_HL:
      return "0x7E__LD_A_HL";
    case Type::LD_A_A:
      return "0x7F__LD_A_A";
    case Type::ADD_A_B:
      return "0x80__ADD_A_B";
    case Type::ADD_A_C:
      return "0x81__ADD_A_C";
    case Type::ADD_A_D:
      return "0x82__ADD_A_D";
    case Type::ADD_A_E:
      return "0x83__ADD_A_E";
    case Type::ADD_A_H:
      return "0x84__ADD_A_H";
    case Type::ADD_A_L:
      return "0x85__ADD_A_L";
    case Type::ADD_A_HL:
      return "0x86__ADD_A_HL";
    case Type::ADD_A_A:
      return "0x87__ADD_A_A";
    case Type::ADC_A_B:
      return "0x88__ADC_A_B";
    case Type::ADC_A_C:
      return "0x89__ADC_A_C";
    case Type::ADC_A_D:
      return "0x8A__ADC_A_D";
    case Type::ADC_A_E:
      return "0x8B__ADC_A_E";
    case Type::ADC_A_H:
      return "0x8C__ADC_A_H";
    case Type::ADC_A_L:
      return "0x8D__ADC_A_L";
    case Type::ADC_A_HL:
      return "0x8E__ADC_A_HL";
    case Type::ADC_A_A:
      return "0x8F__ADC_A_A";
    case Type::SUB_B:
      return "0x90__SUB_B";
    case Type::SUB_C:
      return "0x91__SUB_C";
    case Type::SUB_D:
      return "0x92__SUB_D";
    case Type::SUB_E:
      return "0x93__SUB_E";
    case Type::SUB_H:
      return "0x94__SUB_H";
    case Type::SUB_L:
      return "0x95__SUB_L";
    case Type::SUB_HL:
      return "0x96__SUB_HL";
    case Type::SUB_A:
      return "0x97__SUB_A";
    case Type::SBC_A_B:
      return "0x98__SBC_A_B";
    case Type::SBC_A_C:
      return "0x99__SBC_A_C";
    case Type::SBC_A_D:
      return "0x9A__SBC_A_D";
    case Type::SBC_A_E:
      return "0x9B__SBC_A_E";
    case Type::SBC_A_H:
      return "0x9C__SBC_A_H";
    case Type::SBC_A_L:
      return "0x9D__SBC_A_L";
    case Type::SBC_A_HL:
      return "0x9E__SBC_A_HL";
    case Type::SBC_A_A:
      return "0x9F__SBC_A_A";
    case Type::AND_B:
      return "0xA0__AND_B";
    case Type::AND_C:
      return "0xA1__AND_C";
    case Type::AND_D:
      return "0xA2__AND_D";
    case Type::AND_E:
      return "0xA3__AND_E";
    case Type::AND_H:
      return "0xA4__AND_H";
    case Type::AND_L:
      return "0xA5__AND_L";
    case Type::AND_HL:
      return "0xA6__AND_HL";
    case Type::AND_A:
      return "0xA7__AND_A";
    case Type::XOR_B:
      return "0xA8__XOR_B";
    case Type::XOR_C:
      return "0xA9__XOR_C";
    case Type::XOR_D:
      return "0xAA__XOR_D";
    case Type::XOR_E:
      return "0xAB__XOR_E";
    case Type::XOR_H:
      return "0xAC__XOR_H";
    case Type::XOR_L:
      return "0xAD__XOR_L";
    case Type::XOR_HL:
      return "0xAE__XOR_HL";
    case Type::XOR_A:
      return "0xAF__XOR_A";
    case Type::OR_B:
      return "0xB0__OR_B";
    case Type::OR_C:
      return "0xB1__OR_C";
    case Type::OR_D:
      return "0xB2__OR_D";
    case Type::OR_E:
      return "0xB3__OR_E";
    case Type::OR_H:
      return "0xB4__OR_H";
    case Type::OR_L:
      return "0xB5__OR_L";
    case Type::OR_HL:
      return "0xB6__OR_HL";
    case Type::OR_A:
      return "0xB7__OR_A";
    case Type::CP_B:
      return "0xB8__CP_B";
    case Type::CP_C:
      return "0xB9__CP_C";
    case Type::CP_D:
      return "0xBA__CP_D";
    case Type::CP_E:
      return "0xBB__CP_E";
    case Type::CP_H:
      return "0xBC__CP_H";
    case Type::CP_L:
      return "0xBD__CP_L";
    case Type::CP_HL:
      return "0xBE__CP_HL";
    case Type::CP_A:
      return "0xBF__CP_A";
    case Type::RET_NZ:
      return "0xC0__RET_NZ";
    case Type::POP_BC:
      return "0xC1__POP_BC";
    case Type::JP_NZ_a16:
      return "0xC2__JP_NZ_a16";
    case Type::JP_a16:
      return "0xC3__JP_a16";
    case Type::CALL_NZ_a16:
      return "0xC4__CALL_NZ_a16";
    case Type::PUSH_BC:
      return "0xC5__PUSH_BC";
    case Type::ADD_A_d8:
      return "0xC6__ADD_A_d8";
    case Type::RST_0:
      return "0xC7__RST_0";
    case Type::RET_Z:
      return "0xC8__RET_Z";
    case Type::RET:
      return "0xC9__RET";
    case Type::JP_Z_a16:
      return "0xCA__JP_Z_a16";
    case Type::CALL_Z_a16:
      return "0xCC__CALL_Z_a16";
    case Type::CALL_a16:
      return "0xCD__CALL_a16";
    case Type::ADC_A_d8:
      return "0xCE__ADC_A_d8";
    case Type::RST_1:
      return "0xCF__RST_1";
    case Type::RET_NC:
      return "0xD0__RET_NC";
    case Type::POP_DE:
      return "0xD1__POP_DE";
    case Type::JP_NC_a16:
      return "0xD2__JP_NC_a16";
    case Type::CALL_NC_a16:
      return "0xD4__CALL_NC_a16";
    case Type::PUSH_DE:
      return "0xD5__PUSH_DE";
    case Type::SUB_d8:
      return "0xD6__SUB_d8";
    case Type::RST_2:
      return "0xD7__RST_2";
    case Type::RET_C:
      return "0xD8__RET_C";
    case Type::RETI:
      return "0xD9__RETI";
    case Type::JP_C_a16:
      return "0xDA__JP_C_a16";
    case Type::CALL_C_a16:
      return "0xDC__CALL_C_a16";
    case Type::SBC_A_d8:
      return "0xDE__SBC_A_d8";
    case Type::RST_3:
      return "0xDF__RST_3";
    case Type::LD_a8_A:
      return "0xE0__LD_a8_A";
    case Type::POP_HL:
      return "0xE1__POP_HL";
    case Type::LD_mem_C_A:
      return "0xE2__LD_mem_C_A";
    case Type::PUSH_HL:
      return "0xE5__PUSH_HL";
    case Type::AND_d8:
      return "0xE6__AND_d8";
    case Type::RST_4:
      return "0xE7__RST_4";
    case Type::ADD_SP_s8:
      return "0xE8__ADD_SP_s8";
    case Type::JP_HL:
      return "0xE9__JP_HL";
    case Type::LD_a16_A:
      return "0xEA__LD_a16_A";
    case Type::XOR_d8:
      return "0xEE__XOR_d8";
    case Type::RST_5:
      return "0xEF__RST_5";
    case Type::LD_A_a8:
      return "0xF0__LD_A_a8";
    case Type::POP_AF:
      return "0xF1__POP_AF";
    case Type::LD_A_mem_C:
      return "0xF2__LD_A_mem_C";
    case Type::DI:
      return "0xF3__DI";
    case Type::PUSH_AF:
      return "0xF5__PUSH_AF";
    case Type::OR_d8:
      return "0xF6__OR_d8";
    case Type::RST_6:
      return "0xF7__RST_6";
    case Type::LD_HL_SP_inc_s8:
      return "0xF8__LD_HL_SP_inc_s8";
    case Type::LD_SP_HL:
      return "0xF9__LD_SP_HL";
    case Type::LD_A_a16:
      return "0xFA__LD_A_a16";
    case Type::EI:
      return "0xFB__EI";
    case Type::CP_d8:
      return "0xFE__CP_d8";
    case Type::RST_7:
      return "0xFF__RST_7";

    // Prefixed instructions
    case Type::RLC_B:
      return "0xCB00__RLC_B";
    case Type::RLC_C:
      return "0xCB01__RLC_C";
    case Type::RLC_D:
      return "0xCB02__RLC_D";
    case Type::RLC_E:
      return "0xCB03__RLC_E";
    case Type::RLC_H:
      return "0xCB04__RLC_H";
    case Type::RLC_L:
      return "0xCB05__RLC_L";
    case Type::RLC_HL:
      return "0xCB06__RLC_HL";
    case Type::RLC_A:
      return "0xCB07__RLC_A";
    case Type::RRC_B:
      return "0xCB08__RRC_B";
    case Type::RRC_C:
      return "0xCB09__RRC_C";
    case Type::RRC_D:
      return "0xCB0A__RRC_D";
    case Type::RRC_E:
      return "0xCB0B__RRC_E";
    case Type::RRC_H:
      return "0xCB0C__RRC_H";
    case Type::RRC_L:
      return "0xCB0D__RRC_L";
    case Type::RRC_HL:
      return "0xCB0E__RRC_HL";
    case Type::RRC_A:
      return "0xCB0F__RRC_A";
    case Type::RL_B:
      return "0xCB10__RL_B";
    case Type::RL_C:
      return "0xCB11__RL_C";
    case Type::RL_D:
      return "0xCB12__RL_D";
    case Type::RL_E:
      return "0xCB13__RL_E";
    case Type::RL_H:
      return "0xCB14__RL_H";
    case Type::RL_L:
      return "0xCB15__RL_L";
    case Type::RL_HL:
      return "0xCB16__RL_HL";
    case Type::RL_A:
      return "0xCB17__RL_A";
    case Type::RR_B:
      return "0xCB18__RR_B";
    case Type::RR_C:
      return "0xCB19__RR_C";
    case Type::RR_D:
      return "0xCB1A__RR_D";
    case Type::RR_E:
      return "0xCB1B__RR_E";
    case Type::RR_H:
      return "0xCB1C__RR_H";
    case Type::RR_L:
      return "0xCB1D__RR_L";
    case Type::RR_HL:
      return "0xCB1E__RR_HL";
    case Type::RR_A:
      return "0xCB1F__RR_A";
    case Type::SLA_B:
      return "0xCB20__SLA_B";
    case Type::SLA_C:
      return "0xCB21__SLA_C";
    case Type::SLA_D:
      return "0xCB22__SLA_D";
    case Type::SLA_E:
      return "0xCB23__SLA_E";
    case Type::SLA_H:
      return "0xCB24__SLA_H";
    case Type::SLA_L:
      return "0xCB25__SLA_L";
    case Type::SLA_HL:
      return "0xCB26__SLA_HL";
    case Type::SLA_A:
      return "0xCB27__SLA_A";
    case Type::SRA_B:
      return "0xCB28__SRA_B";
    case Type::SRA_C:
      return "0xCB29__SRA_C";
    case Type::SRA_D:
      return "0xCB2A__SRA_D";
    case Type::SRA_E:
      return "0xCB2B__SRA_E";
    case Type::SRA_H:
      return "0xCB2C__SRA_H";
    case Type::SRA_L:
      return "0xCB2D__SRA_L";
    case Type::SRA_HL:
      return "0xCB2E__SRA_HL";
    case Type::SRA_A:
      return "0xCB2F__SRA_A";
    case Type::SWAP_B:
      return "0xCB30__SWAP_B";
    case Type::SWAP_C:
      return "0xCB31__SWAP_C";
    case Type::SWAP_D:
      return "0xCB32__SWAP_D";
    case Type::SWAP_E:
      return "0xCB33__SWAP_E";
    case Type::SWAP_H:
      return "0xCB34__SWAP_H";
    case Type::SWAP_L:
      return "0xCB35__SWAP_L";
    case Type::SWAP_HL:
      return "0xCB36__SWAP_HL";
    case Type::SWAP_A:
      return "0xCB37__SWAP_A";
    case Type::SRL_B:
      return "0xCB38__SRL_B";
    case Type::SRL_C:
      return "0xCB39__SRL_C";
    case Type::SRL_D:
      return "0xCB3A__SRL_D";
    case Type::SRL_E:
      return "0xCB3B__SRL_E";
    case Type::SRL_H:
      return "0xCB3C__SRL_H";
    case Type::SRL_L:
      return "0xCB3D__SRL_L";
    case Type::SRL_HL:
      return "0xCB3E__SRL_HL";
    case Type::SRL_A:
      return "0xCB3F__SRL_A";
    case Type::BIT_0_B:
      return "0xCB40__BIT_0_B";
    case Type::BIT_0_C:
      return "0xCB41__BIT_0_C";
    case Type::BIT_0_D:
      return "0xCB42__BIT_0_D";
    case Type::BIT_0_E:
      return "0xCB43__BIT_0_E";
    case Type::BIT_0_H:
      return "0xCB44__BIT_0_H";
    case Type::BIT_0_L:
      return "0xCB45__BIT_0_L";
    case Type::BIT_0_HL:
      return "0xCB46__BIT_0_HL";
    case Type::BIT_0_A:
      return "0xCB47__BIT_0_A";
    case Type::BIT_1_B:
      return "0xCB48__BIT_1_B";
    case Type::BIT_1_C:
      return "0xCB49__BIT_1_C";
    case Type::BIT_1_D:
      return "0xCB4A__BIT_1_D";
    case Type::BIT_1_E:
      return "0xCB4B__BIT_1_E";
    case Type::BIT_1_H:
      return "0xCB4C__BIT_1_H";
    case Type::BIT_1_L:
      return "0xCB4D__BIT_1_L";
    case Type::BIT_1_HL:
      return "0xCB4E__BIT_1_HL";
    case Type::BIT_1_A:
      return "0xCB4F__BIT_1_A";
    case Type::BIT_2_B:
      return "0xCB50__BIT_2_B";
    case Type::BIT_2_C:
      return "0xCB51__BIT_2_C";
    case Type::BIT_2_D:
      return "0xCB52__BIT_2_D";
    case Type::BIT_2_E:
      return "0xCB53__BIT_2_E";
    case Type::BIT_2_H:
      return "0xCB54__BIT_2_H";
    case Type::BIT_2_L:
      return "0xCB55__BIT_2_L";
    case Type::BIT_2_HL:
      return "0xCB56__BIT_2_HL";
    case Type::BIT_2_A:
      return "0xCB57__BIT_2_A";
    case Type::BIT_3_B:
      return "0xCB58__BIT_3_B";
    case Type::BIT_3_C:
      return "0xCB59__BIT_3_C";
    case Type::BIT_3_D:
      return "0xCB5A__BIT_3_D";
    case Type::BIT_3_E:
      return "0xCB5B__BIT_3_E";
    case Type::BIT_3_H:
      return "0xCB5C__BIT_3_H";
    case Type::BIT_3_L:
      return "0xCB5D__BIT_3_L";
    case Type::BIT_3_HL:
      return "0xCB5E__BIT_3_HL";
    case Type::BIT_3_A:
      return "0xCB5F__BIT_3_A";
    case Type::BIT_4_B:
      return "0xCB60__BIT_4_B";
    case Type::BIT_4_C:
      return "0xCB61__BIT_4_C";
    case Type::BIT_4_D:
      return "0xCB62__BIT_4_D";
    case Type::BIT_4_E:
      return "0xCB63__BIT_4_E";
    case Type::BIT_4_H:
      return "0xCB64__BIT_4_H";
    case Type::BIT_4_L:
      return "0xCB65__BIT_4_L";
    case Type::BIT_4_HL:
      return "0xCB66__BIT_4_HL";
    case Type::BIT_4_A:
      return "0xCB67__BIT_4_A";
    case Type::BIT_5_B:
      return "0xCB68__BIT_5_B";
    case Type::BIT_5_C:
      return "0xCB69__BIT_5_C";
    case Type::BIT_5_D:
      return "0xCB6A__BIT_5_D";
    case Type::BIT_5_E:
      return "0xCB6B__BIT_5_E";
    case Type::BIT_5_H:
      return "0xCB6C__BIT_5_H";
    case Type::BIT_5_L:
      return "0xCB6D__BIT_5_L";
    case Type::BIT_5_HL:
      return "0xCB6E__BIT_5_HL";
    case Type::BIT_5_A:
      return "0xCB6F__BIT_5_A";
    case Type::BIT_6_B:
      return "0xCB70__BIT_6_B";
    case Type::BIT_6_C:
      return "0xCB71__BIT_6_C";
    case Type::BIT_6_D:
      return "0xCB72__BIT_6_D";
    case Type::BIT_6_E:
      return "0xCB73__BIT_6_E";
    case Type::BIT_6_H:
      return "0xCB74__BIT_6_H";
    case Type::BIT_6_L:
      return "0xCB75__BIT_6_L";
    case Type::BIT_6_HL:
      return "0xCB76__BIT_6_HL";
    case Type::BIT_6_A:
      return "0xCB77__BIT_6_A";
    case Type::BIT_7_B:
      return "0xCB78__BIT_7_B";
    case Type::BIT_7_C:
      return "0xCB79__BIT_7_C";
    case Type::BIT_7_D:
      return "0xCB7A__BIT_7_D";
    case Type::BIT_7_E:
      return "0xCB7B__BIT_7_E";
    case Type::BIT_7_H:
      return "0xCB7C__BIT_7_H";
    case Type::BIT_7_L:
      return "0xCB7D__BIT_7_L";
    case Type::BIT_7_HL:
      return "0xCB7E__BIT_7_HL";
    case Type::BIT_7_A:
      return "0xCB7F__BIT_7_A";
    case Type::RES_0_B:
      return "0xCB80__RES_0_B";
    case Type::RES_0_C:
      return "0xCB81__RES_0_C";
    case Type::RES_0_D:
      return "0xCB82__RES_0_D";
    case Type::RES_0_E:
      return "0xCB83__RES_0_E";
    case Type::RES_0_H:
      return "0xCB84__RES_0_H";
    case Type::RES_0_L:
      return "0xCB85__RES_0_L";
    case Type::RES_0_HL:
      return "0xCB86__RES_0_HL";
    case Type::RES_0_A:
      return "0xCB87__RES_0_A";
    case Type::RES_1_B:
      return "0xCB88__RES_1_B";
    case Type::RES_1_C:
      return "0xCB89__RES_1_C";
    case Type::RES_1_D:
      return "0xCB8A__RES_1_D";
    case Type::RES_1_E:
      return "0xCB8B__RES_1_E";
    case Type::RES_1_H:
      return "0xCB8C__RES_1_H";
    case Type::RES_1_L:
      return "0xCB8D__RES_1_L";
    case Type::RES_1_HL:
      return "0xCB8E__RES_1_HL";
    case Type::RES_1_A:
      return "0xCB8F__RES_1_A";
    case Type::RES_2_B:
      return "0xCB90__RES_2_B";
    case Type::RES_2_C:
      return "0xCB91__RES_2_C";
    case Type::RES_2_D:
      return "0xCB92__RES_2_D";
    case Type::RES_2_E:
      return "0xCB93__RES_2_E";
    case Type::RES_2_H:
      return "0xCB94__RES_2_H";
    case Type::RES_2_L:
      return "0xCB95__RES_2_L";
    case Type::RES_2_HL:
      return "0xCB96__RES_2_HL";
    case Type::RES_2_A:
      return "0xCB97__RES_2_A";
    case Type::RES_3_B:
      return "0xCB98__RES_3_B";
    case Type::RES_3_C:
      return "0xCB99__RES_3_C";
    case Type::RES_3_D:
      return "0xCB9A__RES_3_D";
    case Type::RES_3_E:
      return "0xCB9B__RES_3_E";
    case Type::RES_3_H:
      return "0xCB9C__RES_3_H";
    case Type::RES_3_L:
      return "0xCB9D__RES_3_L";
    case Type::RES_3_HL:
      return "0xCB9E__RES_3_HL";
    case Type::RES_3_A:
      return "0xCB9F__RES_3_A";
    case Type::RES_4_B:
      return "0xCBA0__RES_4_B";
    case Type::RES_4_C:
      return "0xCBA1__RES_4_C";
    case Type::RES_4_D:
      return "0xCBA2__RES_4_D";
    case Type::RES_4_E:
      return "0xCBA3__RES_4_E";
    case Type::RES_4_H:
      return "0xCBA4__RES_4_H";
    case Type::RES_4_L:
      return "0xCBA5__RES_4_L";
    case Type::RES_4_HL:
      return "0xCBA6__RES_4_HL";
    case Type::RES_4_A:
      return "0xCBA7__RES_4_A";
    case Type::RES_5_B:
      return "0xCBA8__RES_5_B";
    case Type::RES_5_C:
      return "0xCBA9__RES_5_C";
    case Type::RES_5_D:
      return "0xCBAA__RES_5_D";
    case Type::RES_5_E:
      return "0xCBAB__RES_5_E";
    case Type::RES_5_H:
      return "0xCBAC__RES_5_H";
    case Type::RES_5_L:
      return "0xCBAD__RES_5_L";
    case Type::RES_5_HL:
      return "0xCBAE__RES_5_HL";
    case Type::RES_5_A:
      return "0xCBAF__RES_5_A";
    case Type::RES_6_B:
      return "0xCBB0__RES_6_B";
    case Type::RES_6_C:
      return "0xCBB1__RES_6_C";
    case Type::RES_6_D:
      return "0xCBB2__RES_6_D";
    case Type::RES_6_E:
      return "0xCBB3__RES_6_E";
    case Type::RES_6_H:
      return "0xCBB4__RES_6_H";
    case Type::RES_6_L:
      return "0xCBB5__RES_6_L";
    case Type::RES_6_HL:
      return "0xCBB6__RES_6_HL";
    case Type::RES_6_A:
      return "0xCBB7__RES_6_A";
    case Type::RES_7_B:
      return "0xCBB8__RES_7_B";
    case Type::RES_7_C:
      return "0xCBB9__RES_7_C";
    case Type::RES_7_D:
      return "0xCBBA__RES_7_D";
    case Type::RES_7_E:
      return "0xCBBB__RES_7_E";
    case Type::RES_7_H:
      return "0xCBBC__RES_7_H";
    case Type::RES_7_L:
      return "0xCBBD__RES_7_L";
    case Type::RES_7_HL:
      return "0xCBBE__RES_7_HL";
    case Type::RES_7_A:
      return "0xCBBF__RES_7_A";
    case Type::SET_0_B:
      return "0xCBC0__SET_0_B";
    case Type::SET_0_C:
      return "0xCBC1__SET_0_C";
    case Type::SET_0_D:
      return "0xCBC2__SET_0_D";
    case Type::SET_0_E:
      return "0xCBC3__SET_0_E";
    case Type::SET_0_H:
      return "0xCBC4__SET_0_H";
    case Type::SET_0_L:
      return "0xCBC5__SET_0_L";
    case Type::SET_0_HL:
      return "0xCBC6__SET_0_HL";
    case Type::SET_0_A:
      return "0xCBC7__SET_0_A";
    case Type::SET_1_B:
      return "0xCBC8__SET_1_B";
    case Type::SET_1_C:
      return "0xCBC9__SET_1_C";
    case Type::SET_1_D:
      return "0xCBCA__SET_1_D";
    case Type::SET_1_E:
      return "0xCBCB__SET_1_E";
    case Type::SET_1_H:
      return "0xCBCC__SET_1_H";
    case Type::SET_1_L:
      return "0xCBCD__SET_1_L";
    case Type::SET_1_HL:
      return "0xCBCE__SET_1_HL";
    case Type::SET_1_A:
      return "0xCBCF__SET_1_A";
    case Type::SET_2_B:
      return "0xCBD0__SET_2_B";
    case Type::SET_2_C:
      return "0xCBD1__SET_2_C";
    case Type::SET_2_D:
      return "0xCBD2__SET_2_D";
    case Type::SET_2_E:
      return "0xCBD3__SET_2_E";
    case Type::SET_2_H:
      return "0xCBD4__SET_2_H";
    case Type::SET_2_L:
      return "0xCBD5__SET_2_L";
    case Type::SET_2_HL:
      return "0xCBD6__SET_2_HL";
    case Type::SET_2_A:
      return "0xCBD7__SET_2_A";
    case Type::SET_3_B:
      return "0xCBD8__SET_3_B";
    case Type::SET_3_C:
      return "0xCBD9__SET_3_C";
    case Type::SET_3_D:
      return "0xCBDA__SET_3_D";
    case Type::SET_3_E:
      return "0xCBDB__SET_3_E";
    case Type::SET_3_H:
      return "0xCBDC__SET_3_H";
    case Type::SET_3_L:
      return "0xCBDD__SET_3_L";
    case Type::SET_3_HL:
      return "0xCBDE__SET_3_HL";
    case Type::SET_3_A:
      return "0xCBDF__SET_3_A";
    case Type::SET_4_B:
      return "0xCBE0__SET_4_B";
    case Type::SET_4_C:
      return "0xCBE1__SET_4_C";
    case Type::SET_4_D:
      return "0xCBE2__SET_4_D";
    case Type::SET_4_E:
      return "0xCBE3__SET_4_E";
    case Type::SET_4_H:
      return "0xCBE4__SET_4_H";
    case Type::SET_4_L:
      return "0xCBE5__SET_4_L";
    case Type::SET_4_HL:
      return "0xCBE6__SET_4_HL";
    case Type::SET_4_A:
      return "0xCBE7__SET_4_A";
    case Type::SET_5_B:
      return "0xCBE8__SET_5_B";
    case Type::SET_5_C:
      return "0xCBE9__SET_5_C";
    case Type::SET_5_D:
      return "0xCBEA__SET_5_D";
    case Type::SET_5_E:
      return "0xCBEB__SET_5_E";
    case Type::SET_5_H:
      return "0xCBEC__SET_5_H";
    case Type::SET_5_L:
      return "0xCBED__SET_5_L";
    case Type::SET_5_HL:
      return "0xCBEE__SET_5_HL";
    case Type::SET_5_A:
      return "0xCBEF__SET_5_A";
    case Type::SET_6_B:
      return "0xCBF0__SET_6_B";
    case Type::SET_6_C:
      return "0xCBF1__SET_6_C";
    case Type::SET_6_D:
      return "0xCBF2__SET_6_D";
    case Type::SET_6_E:
      return "0xCBF3__SET_6_E";
    case Type::SET_6_H:
      return "0xCBF4__SET_6_H";
    case Type::SET_6_L:
      return "0xCBF5__SET_6_L";
    case Type::SET_6_HL:
      return "0xCBF6__SET_6_HL";
    case Type::SET_6_A:
      return "0xCBF7__SET_6_A";
    case Type::SET_7_B:
      return "0xCBF8__SET_7_B";
    case Type::SET_7_C:
      return "0xCBF9__SET_7_C";
    case Type::SET_7_D:
      return "0xCBFA__SET_7_D";
    case Type::SET_7_E:
      return "0xCBFB__SET_7_E";
    case Type::SET_7_H:
      return "0xCBFC__SET_7_H";
    case Type::SET_7_L:
      return "0xCBFD__SET_7_L";
    case Type::SET_7_HL:
      return "0xCBFE__SET_7_HL";
    case Type::SET_7_A:
      return "0xCBF__SET_7_A";
  }
}
