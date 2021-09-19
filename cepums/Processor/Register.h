#pragma once

#include "Operand.h"

namespace Cepums {

    class Register8 : public Operand
    {
    public:
        Register8(uint8_t bits) { m_registerBits = bits; }

        virtual uint8_t valueByte(Processor*, MemoryManager&) override;
        virtual void updateByte(Processor*, MemoryManager&, uint8_t newValue) override;

        virtual OperandType type() override { return OperandType::Register; }
        virtual OperandSize size() override { return OperandSize::Byte; }
    private:
        uint8_t m_registerBits;
    };

    class Register16 : public Operand
    {
    public:
        Register16(uint8_t bits) { m_registerBits = bits; }

        virtual uint16_t valueWord(Processor*, MemoryManager&) override;
        virtual void updateWord(Processor*, MemoryManager&, uint16_t newValue) override;

        virtual OperandType type() override { return OperandType::Register; }
        virtual OperandSize size() override { return OperandSize::Word; }
    private:
        uint8_t m_registerBits;
    };
}
