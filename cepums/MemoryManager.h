#pragma once

#include <utility>

namespace Cepums {

    class MemoryManager
    {
    public:
        MemoryManager();

        uint8_t readByte(uint16_t segment, uint16_t offset);
        void writeByte(uint16_t segment, uint16_t offset, uint8_t value);

        uint16_t readWord(uint16_t segment, uint16_t offset);
        void writeWord(uint16_t segment, uint16_t offset, uint16_t value);

        uint32_t addresstoPhysical(const uint16_t& segment, const uint16_t& offset);
        std::pair<uint16_t, uint16_t> addressToLogical(const uint32_t& physicalAddress);
    };
}