#include "cepumspch.h"
#include "MemoryManager.h"

#define KIBIBYTE 1024
#define MEBIBYTE 1048576


namespace Cepums {

    MemoryManager::MemoryManager()
    {
    }

    uint16_t MemoryManager::readByte(uint16_t segment, uint16_t offset)
    {
        auto physical = addresstoPhysical(segment, offset);

        // Check if it is inbounds
        if (physical > MEBIBYTE)
        {
            TODO();
            return 0;
        }

        TODO();
        return uint16_t();
    }

    uint16_t MemoryManager::writeByte(uint16_t segment, uint16_t offset, uint8_t value)
    {
        TODO()
        return uint16_t();
    }

    uint32_t MemoryManager::addresstoPhysical(const uint16_t& segment, const uint16_t& offset)
    {
        uint32_t result = (segment << 4) + offset;
        return result;
    }

    std::pair<uint16_t, uint16_t> MemoryManager::addressToLogical(const uint32_t & physicalAddress)
    {
        TODO();
        return std::pair<uint16_t, uint16_t>();
    }
}