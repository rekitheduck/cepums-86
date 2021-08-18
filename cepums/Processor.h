#pragma once

#include "MemoryManager.h"

#define HIGHER(word) (uint8_t)(word >> 8)
#define LOWER(word) word & 0xFF
#define HIGHER_HALFBYTE(byte) (byte & 0xF0) >> 4
#define LOWER_HALFBYTE(byte) byte & 0xF

#define REGBITS(pos, byte) byte >>= pos; byte &= 0x7
#define MODBITS(pos, byte) byte >>= pos; byte &= 0x3
#define RMBITS(pos, byte) byte >>= pos; byte &= 0x7

#define SET8BITREGISTERHIGH(reg, data) reg &= 0x00FF; uint16_t temp = data << 8; reg |= temp & 0xFF00
#define SET8BITREGISTERLOW(reg, data) reg &= 0xFF00; reg |= data & 0x00FF

#define LOAD_NEXT_INSTRUCTION_BYTE(mm, byte) uint8_t byte = mm.readByte(m_codeSegment, m_instructionPointer); m_instructionPointer++
#define LOAD_NEXT_INSTRUCTION_WORD(mm, word) uint16_t word = mm.readWord(m_codeSegment, m_instructionPointer); m_instructionPointer +=2;
#define PARSE_REG_MOD_RM_BITS(byte, rm, mod, reg) uint8_t rmBits = byte; MODBITS(0, rmBits); uint8_t regBits = byte; REGBITS(3, regBits); uint8_t modBits = byte; MODBITS(6, modBits)
#define LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(mm, modBits, displLow, displHigh) uint8_t displacementLowByte = 0; uint8_t displacementHighByte = 0; loadDisplacementsFromInstructionStream(mm, modBits, displLow, displHigh)
#define CALCULATE_EFFECTIVE_ADDRESS(ea, rmBits, regBits, modBits, isWord, displLow, displHigh) uint16_t ea = getEffectiveAddressFromBits(rmBits, regBits, modBits, isWord, displLow, displHigh)

#define IS_IN_REGISTER_MODE(byte) byte == 0b11

namespace Cepums {

    class Processor
    {
    public:
        Processor();

        void reset();

        void execute(MemoryManager& memoryManager);

        // Large pile of instructions
        void ins$HLT();
        void ins$CLC();
        void ins$CMC();
        void ins$STC();
        void ins$CLD();
        void ins$STD();
        void ins$CLI();
        void ins$STI();
        void ins$WAIT();
        void ins$LOCK();
        void ins$SEGMENT(); // ??

        void ins$ADDregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG);
        void ins$ADDregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG);
        void ins$ADDregisterToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint8_t sourceByte);
        void ins$ADDregisterToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint16_t sourceWord);

        void ins$JMPinterSegment(uint16_t newCodeSegment, uint16_t newInstructionPointer);

        void ins$MOVimmediateToRegisterWord(uint16_t& reg, uint16_t value);
        void ins$MOVimmediateToRegisterByte(uint8_t& reg, uint8_t value);

        uint16_t DS() { return m_dataSegment; }
        uint16_t CS() { return m_codeSegment; }
        uint16_t SS() { return m_stackSegment; }
        uint16_t ES() { return m_extraSegment; }

        // 16-bit General Registers (data)
        uint16_t& AX() { return m_AX; };
        uint16_t& BX() { return m_BX; };
        uint16_t& CX() { return m_CX; };
        uint16_t& DX() { return m_DX; };

        // 8-bit General Register halves
        uint8_t AH() { return HIGHER(m_AX); }
        uint8_t AL() { return LOWER(m_AX); }
        uint8_t BH() { return HIGHER(m_BX); }
        uint8_t BL() { return LOWER(m_BX); }
        uint8_t CH() { return HIGHER(m_CX); }
        uint8_t CL() { return LOWER(m_CX); }
        uint8_t DH() { return HIGHER(m_DX); }
        uint8_t DL() { return LOWER(m_DX); }

        // Setting 8-bit registers
        void AH(uint8_t ah) { SET8BITREGISTERHIGH(m_AX, ah); }
        void AL(uint8_t al) { SET8BITREGISTERLOW(m_AX, al); }
        void BH(uint8_t bh) { SET8BITREGISTERHIGH(m_BX, bh); }
        void BL(uint8_t bl) { SET8BITREGISTERLOW(m_BX, bl); }
        void CH(uint8_t ch) { SET8BITREGISTERHIGH(m_CX, ch); }
        void CL(uint8_t cl) { SET8BITREGISTERLOW(m_CX, cl); }
        void DH(uint8_t dh) { SET8BITREGISTERHIGH(m_DX, dh); }
        void DL(uint8_t dl) { SET8BITREGISTERLOW(m_DX, dl); }

        // Pointer and Index Registers
        uint16_t& SP() { return m_stackPointer; }
        uint16_t& BP() { return m_basePointer; }
        uint16_t& SI() { return m_sourceIndex; }
        uint16_t& DI() { return m_destinationIndex; }
        uint16_t& IP() { return m_instructionPointer; }

        void updateRegisterFromREG8(uint8_t REG, uint8_t data);
        uint8_t getRegisterValueFromREG8(uint8_t REG);
        uint16_t& getRegisterFromREG16(uint8_t REG);
        uint16_t getEffectiveAddressFromBits(uint8_t rmBits, uint8_t regBits, uint8_t modBits, uint8_t isWord, uint8_t displacementLow, uint8_t displacementHigh);

        void loadDisplacementsFromInstructionStream(MemoryManager& memoryManager, uint8_t modBits, uint8_t& displacementLowByte, uint8_t& displacementHighByte);

    private:
        int m_cyclesToWait = 0;

        // TODO: 6-byte instruction queue

        // Flags Register
        uint16_t m_flags = 0;

        uint16_t m_instructionPointer = 0;

        // Segment Registers
        uint16_t m_dataSegment = 0;
        uint16_t m_codeSegment = 0xFFFF;
        uint16_t m_stackSegment = 0;
        uint16_t m_extraSegment = 0;

        // General Registers (data)
        uint16_t m_AX = 0;
        uint16_t m_BX = 0;
        uint16_t m_CX = 0;
        uint16_t m_DX = 0;

        // General Registers (pointers and indices)
        uint16_t m_stackPointer = 0;
        uint16_t m_basePointer = 0;
        uint16_t m_sourceIndex = 0;
        uint16_t m_destinationIndex = 0;
    };
}