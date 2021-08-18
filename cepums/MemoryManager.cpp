#include "cepumspch.h"
#include "MemoryManager.h"

#define KIBIBYTE 1024
#define MEBIBYTE 1048576

namespace Cepums {

    MemoryManager::MemoryManager()
    {
        m_RAM.reserve(640 * KIBIBYTE);
    }

    uint8_t MemoryManager::readByte(uint16_t segment, uint16_t offset)
    {
        uint32_t physical = addresstoPhysical(segment, offset);

        // Is this in RAM (lower 640k?)
        if (physical < 0xA0000)
        {
            return m_RAM.at(physical);
        }

        // Check if it is inbounds
        if (physical > MEBIBYTE)
        {
            TODO();
            return 0;
        }

        TODO();
        return 0;
    }

    void MemoryManager::writeByte(uint16_t segment, uint16_t offset, uint8_t value)
    {
        uint32_t physical = addresstoPhysical(segment, offset);

        // Is this in RAM (lower 640k?)
        if (physical < 0xA0000)
        {
            m_RAM.at(physical) = value;
            return;
        }

        TODO();
    }

    uint16_t MemoryManager::readWord(uint16_t segment, uint16_t offset)
    {
        uint32_t physical = addresstoPhysical(segment, offset);

        // Is this in RAM (lower 640k?)
        if (physical < 0xA0000)
        {
            // TODO: Maybe these are flipped
            uint16_t result = (m_RAM.at(physical) << 8) | (uint16_t)m_RAM.at(++physical);
            return m_RAM.at(physical);
        }

        TODO();
        return 0;
    }

    void MemoryManager::writeWord(uint16_t segment, uint16_t offset, uint16_t value)
    {
        uint32_t physical = addresstoPhysical(segment, offset);

        // Split into two
        uint8_t lower = value & 0x00FF;
        uint8_t higher = (value >> 8) & 0x00FF;

        // Is this in RAM (lower 640k?)
        if (physical < 0xA0000)
        {
            // TODO: Maybe these are flipped
            m_RAM.at(physical) = higher;
            m_RAM.at(++physical) = lower;
            return;
        }

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