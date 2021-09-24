#pragma once

namespace Cepums {

    enum class TransferDirection
    {
        FromSystemToController,
        FromControllerToSystem
    };

    enum class FDCMode
    {
        Nothing,
        Specify = 0x03,
        Recalibrate = 0x07,
        SenseInterruptStatus = 0x08,
        ReadID = 0x4A
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

        bool performInterruptAfterFIFO();
    private:
        void updateDriveMotor(uint8_t boolean, unsigned int drivenum);
    private:
        int m_lastCommandByteWritten = 0;
        bool m_performInterruptAfterFIFO = false;
        bool m_returnSenseInterruptStatus = false;
        bool m_busy = false;
        TransferDirection m_direction = TransferDirection::FromSystemToController;
        std::vector<uint8_t> m_outputBuffer;
        FloppyDiskDrive m_drives[4];
        FDCMode m_FDCmode = FDCMode::Nothing;
        uint8_t m_statusRegister0 = 0xC0;
        uint8_t m_statusRegister1 = 0x00;
        uint8_t m_statusRegister2 = 0x00;
        bool m_IRQenabled = false;
    };
}
