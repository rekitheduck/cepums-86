#pragma once

namespace Cepums {

    enum class CounterReadWriteMode
    {
        LeastSignificantOnly,
        MostSignificantOnly,
        LeastSignificantFirstThenMostSignificant
    };

    struct PITState
    {
        bool counter0output = false;
        bool counter1output = false;
        bool counter2output = false;
    };

    struct PITCounter
    {
        uint16_t current;
        uint16_t latched; // Basically a copy
        uint16_t initial;
        bool output = false; // TODO: Do something with the output
        bool isInitialized = false;
        bool isLatched = false;
        bool isHighLatchedByteRead = false;
        bool isLowLatchedByteRead = false;
        bool isInBCDmode = false;
        bool isUpdatingLowByte = true; // Used for CounterReadWriteMode::LeastSignificantFirstThenMostSignificant
        int mode = -1;
        CounterReadWriteMode readWriteMode;
    };

    class PIT
    {
    public:
        uint8_t readCounter0();
        uint8_t readCounter1();
        uint8_t readCounter2();

        void writeControlRegister(uint8_t value);
        void writeCounter0(uint8_t value);
        void writeCounter1(uint8_t value);
        void writeCounter2(uint8_t value);

        PITState& update();
    private:
        void writeCounter(size_t counter, uint8_t value);
        uint8_t readCounter(size_t counter);
    private:
        PITState m_state;
        // Counter 0: Counter divisor
        // Counter 1: RAM refresh counter
        // Counter 2: Cassette  and speaker
        PITCounter m_counter[3];
    };
}
