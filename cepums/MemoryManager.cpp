#include "cepumspch.h"
#include "MemoryManager.h"

#define KIBIBYTE 1024
#define MEBIBYTE 1048576

namespace Cepums {

    MemoryManager::MemoryManager()
    {
        m_RAM.resize(640 * KIBIBYTE);
        m_BIOS_F0000.resize(32 * KIBIBYTE);
        m_BIOS_F8000.resize(32 * KIBIBYTE);

        // Load BIOS
        std::ifstream firstBIOSbinary;
        std::ifstream secondBIOSbinary;

        // Read 1st binary
        firstBIOSbinary.open("BIOS_5160_F000.BIN", std::ios::binary);
        if (firstBIOSbinary.is_open())
        {
            firstBIOSbinary.read((char*)m_BIOS_F0000.data(), 32 * KIBIBYTE);
            firstBIOSbinary.close();
        }
        else
        {
            DC_CORE_CRITICAL("Error reading first BIOS binary");
        }

        // Read 2nd binary
        //secondBIOSbinary.open("BIOS_5160_F800.BIN", std::ios::binary);
        secondBIOSbinary.open("bios.bin", std::ios::binary); // Load xi_8088 bios instead
        if (secondBIOSbinary.is_open())
        {
            secondBIOSbinary.read((char*)m_BIOS_F8000.data(), 32 * KIBIBYTE);
            secondBIOSbinary.close();
        }
        else
        {
            DC_CORE_CRITICAL("Error reading second BIOS binary");
        }
    }

    uint8_t MemoryManager::readByte(uint16_t segment, uint16_t offset)
    {
        uint32_t physical = addresstoPhysical(segment, offset);

        // Is this in RAM (lower 640k?)
        if (physical < 0xA0000)
            return m_RAM.at(physical);

        if (physical < 0xF0000)
        {
            TODO();
        }

        // First BIOS binary
        if (physical < 0xF8000)
            return m_BIOS_F0000.at(physical - 0xF0000);

        // Second BIOS binary
        if (physical < 0x100000)
        {
            if (physical - 0xF8000 > 32 * KIBIBYTE)
            {
                TODO();
                return 0;
            }
            return m_BIOS_F8000.at(physical - 0xF8000);
        }

        // If we got here, we're probably out of bounds
        DC_CORE_CRITICAL("Accessing memory out of bounds 0x{0:x}", physical);
        TODO();

        // Check if it is inbounds
        if (physical > MEBIBYTE)
        {
            TODO();
            return 0;
        }

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
            return (m_RAM.at(physical) << 8) | (uint16_t)m_RAM.at(++physical);
        }

        if (physical < 0xF0000)
        {
            TODO();
        }

        // First BIOS binary
        if (physical < 0xF8000)
            return (m_BIOS_F0000.at(physical - 0xF0000) << 8) | (uint16_t)m_BIOS_F0000.at(++physical - 0xF0000);

        // Second BIOS binary
        if (physical < 0x100000)
        {
            auto first = m_BIOS_F8000.at(physical - 0xF8000);
            auto second = m_BIOS_F8000.at(++physical - 0xF8000);
            uint16_t result = (uint16_t)second << 8 | first;

            // NOTE: For some reason putting it all into one line makes it return the second byte twice
            //return ((uint16_t)m_BIOS_F8000.at(physical - 0xF8000) << 8) | m_BIOS_F8000.at(++physical - 0xF8000);
            return result;
        }

        // If we got here, we're probably out of bounds
        DC_CORE_CRITICAL("Accessing memory out of bounds 0x{0:x}", physical);
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