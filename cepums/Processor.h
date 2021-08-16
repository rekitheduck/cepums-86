#pragma once

#include "MemoryManager.h"

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

        uint16_t DS() { return m_dataSegment; }
        uint16_t CS() { return m_codeSegment; }
        uint16_t SS() { return m_stackSegment; }
        uint16_t ES() { return m_extraSegment; }

        // 16-bit General Registers (data)
        uint16_t AX() { return m_AX; };
        uint16_t BX() { return m_BX; };
        uint16_t CX() { return m_CX; };
        uint16_t DX() { return m_DX; };

        // 8-bit General Register halves
        uint8_t AH() { return (uint8_t)(m_AX >> 8); }
        uint8_t AL() { return m_AX & 0xFF; }
        uint8_t BH() { return (uint8_t)(m_BX >> 8); }
        uint8_t BL() { return m_BX & 0xFF; }
        uint8_t CH() { return (uint8_t)(m_CX >> 8); }
        uint8_t CL() { return m_CX & 0xFF; }
        uint8_t DH() { return (uint8_t)(m_DX >> 8); }
        uint8_t DL() { return m_DX & 0xFF; }

        // Pointer and Index Registers
        uint16_t SP() { return m_stackPointer; }
        uint16_t BP() { return m_basePointer; }
        uint16_t SI() { return m_sourceIndex; }
        uint16_t DI() { return m_destinationIndex; }
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