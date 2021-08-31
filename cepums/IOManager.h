#pragma once

#include <utility>
#include <vector>
#include "KeyboardController.h"
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
        KeyboardController m_8042KBC;
        PIT m_8254PIT;

        // State stuff
        bool m_refreshRequest = false;
    };
}
