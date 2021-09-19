#include "cepumspch.h"
#include "Register.h"

#include "Processor.h"

namespace Cepums {

    uint8_t Register8::valueByte(Processor* processor, MemoryManager&)
    {
        DC_CORE_ASSERT(processor, "processor ptr");
        return processor->getRegisterValueFromREG8(m_registerBits);
    }

    void Register8::updateByte(Processor* processor, MemoryManager&, uint8_t newValue)
    {
        DC_CORE_ASSERT(processor, "processor ptr");
        processor->updateRegisterFromREG8(m_registerBits, newValue);
    }

    uint16_t Register16::valueWord(Processor* processor, MemoryManager&)
    {
        DC_CORE_ASSERT(processor, "processor ptr");
        return processor->getRegisterFromREG16(m_registerBits);
    }

    void Register16::updateWord(Processor* processor, MemoryManager&, uint16_t newValue)
    {
        DC_CORE_ASSERT(processor, "processor ptr");
        processor->updateRegisterFromREG16(m_registerBits, newValue);
    }
}
