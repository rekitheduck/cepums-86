#pragma once

#include "Operand.h"

namespace Cepums {

    class Immediate8 : public Operand
    {
    public:
        Immediate8(uint8_t immediate) { m_immediate = immediate; }

        virtual uint8_t valueByte(Processor*, MemoryManager&) override { return m_immediate; }
        virtual void updateByte(Processor*, MemoryManager&, uint8_t newValue) override { VERIFY_NOT_REACHED(); }

        virtual const char* name() override { return "IMMEDIATE TODO"; }
        virtual OperandType type() override { return OperandType::Immediate; }
        virtual OperandSize size() override { return OperandSize::Byte; }
    private:
        uint8_t m_immediate;
    };

    class Immediate16 : public Operand
    {
    public:
        Immediate16(uint16_t immediate) { m_immediate = immediate; }

        virtual uint16_t valueWord(Processor*, MemoryManager&) override { return m_immediate; }
        virtual void updateWord(Processor*, MemoryManager&, uint16_t newValue) override { VERIFY_NOT_REACHED(); }

        virtual const char* name() override { return "IMMEDIATE TODO"; }
        virtual OperandType type() override { return OperandType::Immediate; }
        virtual OperandSize size() override { return OperandSize::Word; }
    private:
        uint16_t m_immediate;
    };
}
