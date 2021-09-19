#pragma once

#include <cstdint>

namespace Cepums {

    class Processor;
    class MemoryManager;

    enum class OperandType
    {
        Immediate,
        Memory,
        Register,
        SegmentRegister
    };

    enum class OperandSize
    {
        Byte,
        Word
    };

    class Operand
    {
    public:
        virtual uint8_t valueByte(Processor*, MemoryManager&) { VERIFY_NOT_REACHED(); return 0; };
        virtual uint16_t valueWord(Processor*, MemoryManager&) { VERIFY_NOT_REACHED(); return 0; };

        virtual void updateByte(Processor*, MemoryManager&, uint8_t newValue) {}
        virtual void updateWord(Processor*, MemoryManager&, uint16_t newValue) {}
        virtual void handleSegmentOverridePrefix(Processor*) {}

        virtual const char* name() = 0;
        virtual OperandType type() = 0;
        virtual OperandSize size() = 0;
    };
}
