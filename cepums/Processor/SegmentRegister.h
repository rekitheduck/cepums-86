#pragma once

#include "Operand.h"

namespace Cepums {

    class SegmentRegister : public Operand
    {
    public:
        SegmentRegister(uint8_t bits) { m_segmentRegisterBits = bits; }

        virtual uint16_t valueWord(Processor*, MemoryManager&) override;
        virtual void updateWord(Processor*, MemoryManager&, uint16_t newValue) override;

        static const char* nameFromSEGREG(uint8_t SEGREG);

        virtual const char* name() override;
        virtual OperandType type() override { return OperandType::SegmentRegister; }
        virtual OperandSize size() override { return OperandSize::Word; }
    private:
        uint8_t m_segmentRegisterBits;
    };
}
