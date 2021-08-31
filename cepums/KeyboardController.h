#pragma once

namespace Cepums {

    class KeyboardController
    {
    public:
        void writeCommandRegister(uint8_t value);
        void writeDataPort(uint8_t value);

        uint8_t readDataPort();
        uint8_t readStatusRegister();
    private:
    };
}
