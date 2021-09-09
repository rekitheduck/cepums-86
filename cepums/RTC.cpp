#include "cepumspch.h"
#include "RTC.h"

namespace Cepums {

    enum RTCMode : int
    {
        Unknown = -1,
        Seconds = 0x00,
        Minutes = 0x02,
        Hours = 0x04,
        DateDay = 0x07,
        DateMonth = 0x08,
        DateYear = 0x09,

        StatusRegisterA = 0x0A,
        StatusRegisterB = 0x0B,
        StatusRegisterC = 0x0C,
        StatusRegisterD = 0x0D,

        FloppyDiskTypes = 0x10,
        SystemConfigurationSettings = 0x11,
        HardDiskTypes = 0x12,
        TypematicParameters = 0x13,
        InstalledEquipment = 0x14,
        BaseMemoryLSB = 0x15,
        BaseMemoryMSB = 0x16,
        ExtendedMemoryLSB = 0x17,
        ExtendedMemoryMSB = 0x18,
        HardDisk0Type = 0x19,
        HardDisk1Type = 0x1A,
        Unused0 = 0x1B,
        Unused1 = 0x1C,
        Unused2 = 0x1D,
        Unused3 = 0x1E,
        Unused4 = 0x1F,
        Unused5 = 0x20,

        SystemOperationalFlags = 0x2D,
        ChecksumMSB = 0x2E,
        ChecksumLSB = 0x2F,

        DateCentury = 0x32,
    };

    void RTC::writeDataPort(uint8_t value)
    {
        switch (m_mode)
        {
        case StatusRegisterA:
            m_statusRegisterA = value;
            break;
        case StatusRegisterB:
            m_statusRegisterB = value;
            break;
        case StatusRegisterC:
            m_statusRegisterC = value;
            break;
        case RTCMode::Unknown:
            DC_CORE_ERROR("Yikes, RTC has been accessed with no mode specified");
            TODO();
            break;
        default:
            // Uh oh
            TODO();
            break;
        }
    }

    uint8_t RTC::readDataPort()
    {
        uint8_t byte = 0;
        switch (m_mode)
        {
        case Seconds:
            byte = m_seconds;
            break;
        case Minutes:
            byte = m_minutes;
            break;
        case Hours:
            byte = m_hours;
            break;
        case DateDay:
            byte = m_day;
            break;
        case DateMonth:
            byte = m_month;
            break;
        case DateYear:
            byte = m_year;
            break;

        case StatusRegisterA:
            byte = m_statusRegisterA;
            break;
        case StatusRegisterB:
            byte = m_statusRegisterB;
            break;
        case StatusRegisterC:
            byte = m_statusRegisterB;
            break;
        case StatusRegisterD: // Valid CMOR RAM flag on bit 7
            byte = 0x80;
            break;
        case FloppyDiskTypes:
            byte = 0x40; // Drive 0: 1.44MB | Drive 1: None
            break;
        case SystemConfigurationSettings:
            byte = 0x8B; // A bunch of things
            break;
        case HardDiskTypes:
            byte = 0x00; // No hard disks
            break;
        case TypematicParameters:
            byte = 0x00; // No typematic stuff
            break;
        case InstalledEquipment:
            byte = 0x3D; // A bunch of things (including display adapter)
            break;

        case BaseMemoryLSB:
            byte = 0x80; // 640KB base memory
            break;
        case BaseMemoryMSB:
            byte = 0x02; // 640KB base memory
            break;

        case ExtendedMemoryLSB:
            byte = 0x00; // 0KB extended memory
            break;
        case ExtendedMemoryMSB:
            byte = 0x00; // 0KB extended memory
            break;

        case HardDisk0Type:
        case HardDisk1Type:
            byte = 0x00; // No hard disks
            break;

        case Unused0:
        case Unused1:
        case Unused2:
        case Unused3:
        case Unused4:
        case Unused5:
            byte = 0x00; // These actually have hard drive info
            break;

        case SystemOperationalFlags:
            byte = 0x74; // A bunch of stuff
            break;

        case ChecksumLSB:
            byte = 0xF3;
            break;
        case ChecksumMSB:
            byte = 0x00;
            break;

        case DateCentury:
            byte = m_century;
            break;

        case RTCMode::Unknown:
            DC_CORE_ERROR("Yikes, RTC has been accessed with no mode specified");
            TODO();
            break;
        default:
            // Uh oh
            DC_CORE_ERROR("Unhandled mode: {0:X} :(", m_mode);
            TODO();
            break;
        }
        // Reset RTC access mode
        m_mode = RTCMode::Unknown;
        return byte;
    }
}
