#pragma once

namespace Cepums {

    class RTC
    {
    public:
        void writeIndexRegister(uint8_t value) { m_mode = value; }
        void writeDataPort(uint8_t value);
        uint8_t readDataPort();
    private:
        // 12:48:20
        uint8_t m_seconds = 20;
        uint8_t m_minutes = 48;
        uint8_t m_hours = 12;

        // 09.09.2021
        uint8_t m_day = 9;
        uint8_t m_month = 9;
        uint8_t m_year = 21;
        uint8_t m_century = 32; // TODO: Figure out if should be doing this

        uint8_t m_statusRegisterA = 0;
        uint8_t m_statusRegisterB = 4; // Enable DST and 24hr clock
        uint8_t m_statusRegisterC = 0; // System status
        int m_mode = -1;
    };
}
