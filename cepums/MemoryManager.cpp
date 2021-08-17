#include "cepumspch.h"
#include "MemoryManager.h"

#define KIBIBYTE 1024
#define MEBIBYTE 1048576


namespace Cepums {

    MemoryManager::MemoryManager()
    {
    }

    uint8_t MemoryManager::readByte(uint16_t segment, uint16_t offset)
    {

        if (offset == 0x0)
        {
            return 0xB8;
        }
        return 0x00;
        auto physical = addresstoPhysical(segment, offset);

        // Check if it is inbounds
        if (physical > MEBIBYTE)
        {
            TODO();
            return 0;
        }

        TODO();
        return uint8_t();
    }

    void MemoryManager::writeByte(uint16_t segment, uint16_t offset, uint8_t value)
    {
        TODO();
    }

    uint16_t MemoryManager::readWord(uint16_t segment, uint16_t offset)
    {
        switch (offset)
        {
        case 0x1:
            return 0xEF;
        //case 0x1:
            //return 
        default:
            return 0x00;
        }
        return 0xB5;
        TODO();
        return uint16_t();
    }

    void MemoryManager::writeWord(uint16_t segment, uint16_t offset, uint16_t value)
    {
        TODO();
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