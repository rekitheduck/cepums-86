#pragma once

#include "Operand.h"

namespace Cepums {

    class Memory8 : public Operand
    {
    public:
        Memory8(uint16_t segmentRegister, uint16_t effectiveAddress) { m_segmentRegister = segmentRegister, m_effectiveAddress = effectiveAddress; }

        virtual uint8_t valueByte(Processor*, MemoryManager&) override;
        virtual void updateByte(Processor*, MemoryManager&, uint8_t newValue) override;

        virtual void handleSegmentOverridePrefix(Processor*) override;

        virtual const char* name() override;
        virtual OperandType type() override { return OperandType::Memory; }
        virtual OperandSize size() override { return OperandSize::Byte; }
    private:
        uint16_t m_segmentRegister;
        uint16_t m_effectiveAddress;
        std::string m_name;
    };

    class Memory16 : public Operand
    {
    public:
        Memory16(uint16_t segmentRegister, uint16_t effectiveAddress) { m_segmentRegister = segmentRegister, m_effectiveAddress = effectiveAddress; }

        virtual uint16_t valueWord(Processor*, MemoryManager&) override;
        virtual void updateWord(Processor*, MemoryManager&, uint16_t newValue) override;

        virtual void handleSegmentOverridePrefix(Processor*) override;

        virtual const char* name() override;
        virtual OperandType type() override { return OperandType::Memory; }
        virtual OperandSize size() override { return OperandSize::Word; }
    private:
        uint16_t m_segmentRegister;
        uint16_t m_effectiveAddress;
        std::string m_name;
    };
}
