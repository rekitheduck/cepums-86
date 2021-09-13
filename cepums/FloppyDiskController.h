#pragma once

namespace Cepums {

    class FloppyDiskController
    {
    public:
        uint8_t readStatusRegisterA();
        uint8_t readStatusRegisterB();

        uint8_t readMainStatusRegister();

        uint8_t readDigitalInputRegister();
        void writeConfigurationControlRegister(uint8_t value);
    private:
    };
}
