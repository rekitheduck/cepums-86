#include "cepumspch.h"
#include "Memory.h"

#include "MemoryManager.h"
#include "Processor.h"

namespace Cepums {

    uint8_t Memory8::valueByte(Processor*, MemoryManager& mm)
    {
        return mm.readByte(m_segmentRegister, m_effectiveAddress);
    }

    void Memory8::updateByte(Processor*, MemoryManager& mm, uint8_t newValue)
    {
        mm.writeByte(m_segmentRegister, m_effectiveAddress, newValue);
    }

    void Memory8::handleSegmentOverridePrefix(Processor* processor)
    {
        DC_CORE_ASSERT(processor, "processor ptr");

        // Handle segment override prefix
        if (processor->hasSegmentOverridePrefix())
            m_segmentRegister = processor->getSegmentRegisterValueAndResetOverride();
    }

    const char* Memory8::name()
    {
        return "MEMORY TODO";
    }

    uint16_t Memory16::valueWord(Processor*, MemoryManager& mm)
    {
        return mm.readWord(m_segmentRegister, m_effectiveAddress);
    }

    void Memory16::handleSegmentOverridePrefix(Processor* processor)
    {
        DC_CORE_ASSERT(processor, "processor ptr");

        // Handle segment override prefix
        if (processor->hasSegmentOverridePrefix())
            m_segmentRegister = processor->getSegmentRegisterValueAndResetOverride();
    }

    const char * Memory16::name()
    {
        return "MEMORY TODO";
    }

    void Memory16::updateWord(Processor*, MemoryManager& mm, uint16_t newValue)
    {
        mm.writeWord(m_segmentRegister, m_effectiveAddress, newValue);
    }
}
