#pragma once

namespace Cepums {

    enum class TransferDirection
    {
        FromSystemToController,
        FromControllerToSystem
    };

    struct FloppyDiskDrive
    {
        bool motorActive = false;
    };

    class FloppyDiskController
    {
    public:
        uint8_t readStatusRegisterA();
        uint8_t readStatusRegisterB();
        void writeDigitalOutputRegister(uint8_t value);

        uint8_t readMainStatusRegister();

        uint8_t readDataFIFO();
        void writeDataFIFO(uint8_t data);

        uint8_t readDigitalInputRegister();
        void writeConfigurationControlRegister(uint8_t value);
    private:
        void updateDriveMotor(uint8_t boolean, unsigned int drivenum);
    private:
        bool m_returnSenseInterruptStatus = false;
        TransferDirection m_direction = TransferDirection::FromSystemToController;
        std::vector<uint8_t> m_outputBuffer;
        FloppyDiskDrive m_drives[4];
    };
}
