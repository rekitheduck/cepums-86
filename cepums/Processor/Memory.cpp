#include "cepumspch.h"
#include "Memory.h"

#include "MemoryManager.h"

namespace Cepums {

    uint8_t Memory8::valueByte(Processor*, MemoryManager& mm)
    {
        return mm.readByte(m_segmentRegister, m_effectiveAddress);
    }

    void Memory8::updateByte(Processor*, MemoryManager& mm, uint8_t newValue)
    {
        mm.writeByte(m_segmentRegister, m_effectiveAddress, newValue);
    }

    uint16_t Memory16::valueWord(Processor*, MemoryManager& mm)
    {
        return mm.readWord(m_segmentRegister, m_effectiveAddress);
    }

    void Memory16::updateWord(Processor*, MemoryManager& mm, uint16_t newValue)
    {
        mm.writeWord(m_segmentRegister, m_effectiveAddress, newValue);
    }
}
