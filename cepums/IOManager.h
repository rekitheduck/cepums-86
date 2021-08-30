#pragma once

#include <utility>
#include <vector>
#include "PIT.h"

namespace Cepums {

    class IOManager
    {
    public:
        IOManager();

        uint8_t readByte(uint16_t address);
        void writeByte(uint16_t address, uint8_t value);

        uint16_t readWord(uint16_t address);
        void writeWord(uint16_t address, uint16_t value);

        void runPIT();
    private:
        uint8_t m_port0x80;
        PIT m_8254PIT;
    };
}
