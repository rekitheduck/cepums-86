#include "cepumspch.h"
#include "SegmentRegister.h"

#include "Processor.h"

namespace Cepums {

    uint16_t SegmentRegister::valueWord(Processor* processor, MemoryManager&)
    {
        DC_CORE_ASSERT(processor, "processor ptr");
        return processor->getSegmentRegisterValue(m_segmentRegisterBits);
    }

    void SegmentRegister::updateWord(Processor* processor, MemoryManager&, uint16_t newValue)
    {
        DC_CORE_ASSERT(processor, "processor ptr");
        processor->updateSegmentRegister(m_segmentRegisterBits, newValue);
    }

    const char* SegmentRegister::nameFromSEGREG(uint8_t SEGREG)
    {
        switch (SEGREG)
        {
        case REGISTER_ES:
            return "ES";
        case REGISTER_CS:
            return "CS";
        case REGISTER_SS:
            return "SS";
        case REGISTER_DS:
            return "DS";
        default:
            DC_CORE_ERROR("Malformed SegReg bits : 0b{0:b}", SEGREG);
            VERIFY_NOT_REACHED();
            return "ERROR";
        }
    }

    const char* SegmentRegister::name()
    {
        return SegmentRegister::nameFromSEGREG(m_segmentRegisterBits);
    }
}
