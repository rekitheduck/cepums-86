#include "cepumspch.h"
#include "FloppyDiskController.h"

#define DRIVE_3 3
#define DRIVE_2 2
#define DRIVE_1 1
#define DRIVE_0 0

namespace Cepums {

    uint8_t FloppyDiskController::readStatusRegisterA()
    {
        TODO();
        return uint8_t();
    }

    uint8_t FloppyDiskController::readStatusRegisterB()
    {
        TODO();
        return uint8_t();
    }

    void FloppyDiskController::writeDigitalOutputRegister(uint8_t value)
    {
        DC_CORE_TRACE("FDC: Writing '0x{0:X}' to DOR", value);

        // If RESET, we can skip ignore the rest

        // Drive motors (bits 7-4)
        updateDriveMotor(IS_BIT_SET(value, 7), DRIVE_3);
        updateDriveMotor(IS_BIT_SET(value, 6), DRIVE_2);
        updateDriveMotor(IS_BIT_SET(value, 5), DRIVE_1);
        updateDriveMotor(IS_BIT_SET(value, 4), DRIVE_0);

        // Bit 3 - IRQ - is handled by the IOManager

        // Bit 2 not being set means reset mode
        if (IS_BIT_NOT_SET(value, 2))
        {
            DC_CORE_TRACE("[FDC]: Command to reset received");
            // I don't know exactly what to do in this situation
            m_direction = TransferDirection::FromSystemToController;
            m_outputBuffer.clear();
        }

        // Bits 0 and 1 select the drive for next access
        if (IS_BIT_SET(value, 1) || IS_BIT_SET(value, 0))
        {
            DC_CORE_TRACE("[FDC]: Drive select TODO");
            TODO();
        }
    }

    uint8_t FloppyDiskController::readMainStatusRegister()
    {
        /*
        bit 7 = 1  RQM data register is ready
                0  no access is permitted
        bit 6 = 1  transfer is from controller to system
                0  transfer is from system to controller
        bit 5 = 1  non - DMA mode
        bit 4 = 1  diskette controller is busy
        bit 3 = 1  drive 3 busy(reserved on PS / 2)
        bit 2 = 1  drive 2 busy(reserved on PS / 2)
        bit 1 = 1  drive 1 busy(= drive is in seek mode)
        bit 0 = 1  drive 0 busy(= drive is in seek mode)
        */
        uint8_t byte = 0x80; // bit 7 is set
        DC_CORE_TRACE("[FDC]: Read main status register");

        // Set direction bit
        if (m_direction == TransferDirection::FromControllerToSystem)
        {
            SET_BIT(byte, 6);
        }

        return byte;
    }

    uint8_t FloppyDiskController::readDataFIFO()
    {
        if (m_outputBuffer.size() > 0)
        {
            uint8_t byte = m_outputBuffer.back();
            DC_CORE_TRACE("[FDC]: Read data FIFO register '0x{0:x}'", byte);
            m_outputBuffer.pop_back();
            return byte;
        }
        DC_CORE_ERROR("[FDC]: Attempt to read data FIFO register but buffer is empty!");

        TODO();
        return 0;
    }

    void FloppyDiskController::writeDataFIFO(uint8_t data)
    {
        // TODO: Figure out when we are writing commands and not data
        switch (data)
        {
        case 0x08: // FDC Sense Interrupt Status command
            DC_CORE_TRACE("[FDC]: 0x{0:x} Write data FIFO register", data);
            // Result 1: Present cylinder number
            m_outputBuffer.push_back(80); // TODO: This needs to come from a mounted disk image
            // Result 0: Status register 0
            m_outputBuffer.push_back(0xC0);

            // We are ready to send data back
            m_direction = TransferDirection::FromControllerToSystem;
            return;

        default:
            TODO();
            break;
        }
    }

    uint8_t FloppyDiskController::readDigitalInputRegister()
    {
        DC_CORE_TRACE("[FDC]: Read digital input register");


        return 0x81; // bit 7 and bit 0 set - diskette change and high-density select
    }

    void FloppyDiskController::writeConfigurationControlRegister(uint8_t value)
    {
        // Bits 0 and 1 specify the speed
        DC_CORE_TRACE("[FDC]: Writing configuration control register");

        return;
    }

    void FloppyDiskController::updateDriveMotor(uint8_t boolean, unsigned int drivenum)
    {
        if (boolean)
        {
            if (!m_drives[drivenum].motorActive)
            {
                DC_CORE_TRACE("[FDC]: Turning on Drive {0} motor ON", drivenum);
                m_drives[drivenum].motorActive = true;
            }
        }
        else
        {
            if (m_drives[drivenum].motorActive)
            {
                DC_CORE_TRACE("[FDC]: Turning on Drive {0} motor OFF", drivenum);
                m_drives[drivenum].motorActive = false;
            }
        }
    }
}
