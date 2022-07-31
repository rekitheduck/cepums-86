#pragma once

#include <utility>
#include <vector>

#include "Hardware/FloppyDiskController.h"
#include "Hardware/KeyboardController.h"
#include "Hardware/PIT.h"
#include "Hardware/RTC.h"

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
        FloppyDiskController m_floppy;
        KeyboardController m_8042KBC;
        PIT m_8254PIT;
        RTC m_RTC;

        // State stuff
        bool m_refreshRequest = false;
        bool m_pretendRetrace = false;
        std::mutex m_keyboardMutex;
        bool m_pendingInterrupt = false;
        uint8_t m_interrupt = 0;

        bool m_floppyDelayingForInterrupt = false;
        unsigned int m_floppyInterruptCounter;

        // DMA stuff
        uint8_t m_DMAPageChannel0{ 0 };
        uint8_t m_DMAPageChannel1{ 0 };
        uint8_t m_DMAPageChannel2{ 0 };
        uint8_t m_DMAPageChannel3{ 0 };
    };
}
