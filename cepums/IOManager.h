#pragma once

#include <utility>
#include <vector>

#include "KeyboardController.h"
#include "PIT.h"
#include "RTC.h"

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
        bool hasPendingInterrupts();
        uint16_t getPendingInterrupt();

        void onKeyPress(SDL_Scancode scancode);
        void onKeyRelease(SDL_Scancode scancode);
    private:
        uint8_t m_port0x80;
        KeyboardController m_8042KBC;
        PIT m_8254PIT;
        RTC m_RTC;

        // State stuff
        bool m_refreshRequest = false;
        bool m_pretendRetrace = false;
        std::mutex m_keyboardMutex;
        bool m_pendingInterrupt = false;
    };
}
