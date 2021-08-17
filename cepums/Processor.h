#pragma once

#include "MemoryManager.h"

#define HIGHER(word) (uint8_t)(word >> 8)
#define LOWER(word) word & 0xFF
#define HIGHER_HALFBYTE(byte) (byte & 0xF0) >> 4
#define LOWER_HALFBYTE(byte) byte & 0xF

#define REG(pos, byte) byte >>= pos; byte &= 0x7;
#define MOD(pos, byte) byte >>= pos; byte &= 0x3;
#define RM(pos, byte) byte >>= pos; byte &= 0x7;

#define SET8BITREGISTERHIGH(reg, data) reg &= 0x00FF; uint16_t temp = data << 8; reg |= temp & 0xFF00
#define SET8BITREGISTERLOW(reg, data) reg &= 0xFF00; reg |= data & 0x00FF;

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
        uint16_t SP() { return m_stackPointer; }
        uint16_t BP() { return m_basePointer; }
        uint16_t SI() { return m_sourceIndex; }
        uint16_t DI() { return m_destinationIndex; }

        void updateRegisterFromREG8(uint8_t REG, uint8_t data);
        uint8_t getRegisterValueFromREG8(uint8_t REG);
        uint16_t& getRegisterFromREG16(uint8_t REG);
        uint16_t getEffectiveAddressFromBits(uint8_t rmBits, uint8_t regBits, uint8_t modBits, uint8_t isWord, uint8_t displacementLow, uint8_t displacementHigh);
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