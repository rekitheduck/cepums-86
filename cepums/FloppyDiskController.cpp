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
        DC_CORE_INFO("[FDC]: Writing '0x{0:X}' to DOR", value);

        // If RESET, we can skip ignore the rest

        // Drive motors (bits 7-4)
        updateDriveMotor(IS_BIT_SET(value, 7), DRIVE_3);
        updateDriveMotor(IS_BIT_SET(value, 6), DRIVE_2);
        updateDriveMotor(IS_BIT_SET(value, 5), DRIVE_1);
        updateDriveMotor(IS_BIT_SET(value, 4), DRIVE_0);

        // Bit 3 - IRQ - is handled by the IOManager
        if (IS_BIT_SET(value, 3))
            m_IRQenabled = true;
        else
            m_IRQenabled = false;

        // Bit 2 not being set means reset mode
        if (IS_BIT_NOT_SET(value, 2))
        {
            DC_CORE_ERROR("[FDC]: Command to reset received");
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

        // IRQ enabled
        if (!m_IRQenabled)
        {
            SET_BIT(byte, 5);
        }


        if (m_outputBuffer.size() > 0)
        {
            SET_BIT(byte, 4);
            m_busy = false;
        }

        return byte;
    }

    uint8_t FloppyDiskController::readDataFIFO()
    {
        if (m_outputBuffer.size() > 0)
        {
            std::string mode;
            switch (m_FDCmode)
            {
            case FDCMode::Nothing:
                DC_CORE_ERROR("[FDC]: Reading FIFO in WAITING mode");
                mode = "WAITING";
                break;
            case FDCMode::Recalibrate:
                mode = "Recalibrate";
                break;
            case FDCMode::SenseInterruptStatus:
                mode = "SenseInterruptStatus";
                break;
            case FDCMode::ReadID:
                mode = "ReadID";
                break;
            default:
                TODO();
                break;
            }

            uint8_t byte = m_outputBuffer.back();
            DC_CORE_TRACE("[FDC]: Read data FIFO in {1} mode: '0x{0:x}'", byte, mode);
            m_outputBuffer.pop_back();

            // Clear our mode if the buffer is empty
            if (m_outputBuffer.size() == 0)
            {
                m_direction = TransferDirection::FromSystemToController;
                m_FDCmode = FDCMode::Nothing;
            }

            return byte;
        }
        DC_CORE_ERROR("[FDC]: Attempt to read data FIFO register but buffer is empty!");

        TODO();
        return 0;
    }

    void FloppyDiskController::writeDataFIFO(uint8_t data)
    {
        // Are we expecting a new command?
        if (m_FDCmode == FDCMode::Nothing)
        {
            switch (data)
            {
            case 0x03: // FDC Specify Step & Head Load
                DC_CORE_INFO("[FDC]: Sense command received");
                m_FDCmode = FDCMode::Specify;
                m_lastCommandByteWritten = 0; // We need more bytes
                return;

            case 0x07: // FDC Recalibrate
                DC_CORE_INFO("[FDC]: Recalibrate command received");
                // We still need to receive one more command byte
                m_FDCmode = FDCMode::Recalibrate;
                m_lastCommandByteWritten = 0; // We need more bytes
                return;

            case 0x08: // FDC Sense Interrupt Status
                DC_CORE_INFO("[FDC]: Sense Interrupt Status command received");
                // Result 1: Present cylinder number
                m_outputBuffer.push_back(80); // TODO: This needs to come from a mounted disk image
                // Result 0: Status register 0
                m_outputBuffer.push_back(m_statusRegister0);

                // We are ready to send data back
                m_direction = TransferDirection::FromControllerToSystem;
                m_FDCmode = FDCMode::SenseInterruptStatus;
                return;

            case 0x0A: // FDC Read ID but without MF
                TODO();
            case 0x4A: // FDC Read ID
                DC_CORE_INFO("[FDC]: ReadID command received");
                m_FDCmode = FDCMode::ReadID;
                m_lastCommandByteWritten = 0; // We need one more byte
                return;

            default:
                TODO();
                return;
            }
        }

        // We're in a command, so we're probably waiting for more command bytes
        switch (m_FDCmode)
        {
        case FDCMode::Nothing:
            DC_CORE_ERROR("[FDC]: Writing additional command bytes but not in any mode!");
            VERIFY_NOT_REACHED();
            break;

        case FDCMode::Specify:
            DC_CORE_TRACE("[FDC]: Sense{0}: Ignoring value 0x{1:X}", m_lastCommandByteWritten + 1, data);
            if (m_lastCommandByteWritten == 1)
            {
                m_FDCmode = FDCMode::Nothing;
            }
            break;

        case FDCMode::Recalibrate:
            // Only one additional byte - drive select
            if (data == 0)
                DC_CORE_TRACE("[FDC]: Recalibrate1: Working on drive 0");
            else
                TODO();
            m_FDCmode = FDCMode::Nothing;
            // We also need to set status register 0 here
            m_statusRegister0 = 0xD0;
            // We need to perform an interrupt here
            m_performInterruptAfterFIFO = true;
            m_busy = true;
            break;

        case FDCMode::ReadID:
            DC_CORE_TRACE("[FDC]: ReadID");
            m_direction = TransferDirection::FromControllerToSystem;
            // Result 6: Bytes per sector
            m_outputBuffer.push_back(0x02); // Somehow means 512k
            // Result 5: Sector number
            m_outputBuffer.push_back(1);
            // Result 4: Head number
            m_outputBuffer.push_back(0); // Uhh, head 0?
            // Result 3: Cylinder number
            m_outputBuffer.push_back(0); // The one we're on rn?
            // Result 2: Status register 2
            m_outputBuffer.push_back(m_statusRegister2);
            // Result 1: Status register 1
            m_outputBuffer.push_back(m_statusRegister1);
            // Result 0: Status register 0
            m_outputBuffer.push_back(m_statusRegister0);
            m_performInterruptAfterFIFO = true;
            break;

        case FDCMode::SenseInterruptStatus:
            DC_CORE_ERROR("[FDC]: SenseInterruptStatus doesn't require any more command writes, please read twice to reset");
            VERIFY_NOT_REACHED();
            break;
        default:
            TODO();
            break;
        }
        m_lastCommandByteWritten++;
    }

    uint8_t FloppyDiskController::readDigitalInputRegister()
    {
        DC_CORE_TRACE("[FDC]: Read digital input register");


        //return 0x81; // bit 7 and bit 0 set - diskette change and high-density select
        return 1;
    }

    void FloppyDiskController::writeConfigurationControlRegister(uint8_t value)
    {
        // Bits 0 and 1 specify the speed
        DC_CORE_TRACE("[FDC]: Writing configuration control register");

        if (value == 0)
            DC_CORE_TRACE("[FDC]: We have a 1.44 MB diskette :)");
        else
            TODO();

        // TODO: Figure out where this should be set
        m_direction = TransferDirection::FromSystemToController;
    }

    bool FloppyDiskController::performInterruptAfterFIFO()
    {
        if (m_performInterruptAfterFIFO)
        {
            m_performInterruptAfterFIFO = false;
            return true;
        }
        return false;
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
