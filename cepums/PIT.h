#pragma once

namespace Cepums {

    enum class CounterReadWriteMode
    {
        LeastSignificantOnly,
        MostSignificantOnly,
        LeastSignificantFirstThenMostSignificant
    };

    struct PITCounter
    {
        uint16_t counterCurrent;
        uint16_t counterInitial;
        bool isInitialized = false;
        bool isInBCDmode = false;
        bool isUpdatingLowByte = true; // Used for CounterReadWriteMode::LeastSignificantFirstThenMostSignificant
        int mode = -1;
        CounterReadWriteMode readWriteMode;
    };

    class PIT
    {
    public:
        void writeControlRegister(uint8_t value);
        void writeCounter0(uint8_t value);
        void writeCounter1(uint8_t value);
        void writeCounter2(uint8_t value);

        void update();
    private:
        void writeCounter(size_t counter, uint8_t value);
    private:
        uint16_t m_outputLatch;
        bool m_latched = false;

        // Counter 0: Counter divisor
        // Counter 1: RAM refresh counter
        // Counter 2: Cassette  and speaker
        PITCounter m_counter[3];
    };
}
