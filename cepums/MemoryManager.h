#pragma once

#include <utility>
#include <vector>

namespace Cepums {

    class MemoryManager
    {
    public:
        MemoryManager();

        uint8_t readByte(uint16_t segment, uint16_t offset);
        void writeByte(uint16_t segment, uint16_t offset, uint8_t value);

        uint16_t readWord(uint16_t segment, uint16_t offset);
        void writeWord(uint16_t segment, uint16_t offset, uint16_t value);

        static uint32_t addresstoPhysical(const uint16_t& segment, const uint16_t& offset);
        std::pair<uint16_t, uint16_t> addressToLogical(const uint32_t& physicalAddress);
    private:
        std::vector<uint8_t> m_RAM;
        std::vector<uint8_t> m_BIOS_F0000;
        std::vector<uint8_t> m_BIOS_F8000;
    };
}