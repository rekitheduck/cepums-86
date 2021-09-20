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
        case Seconds:
            m_seconds = value;
            break;
        case Minutes:
            m_minutes = value;
            break;
        case Hours:
            m_hours = value;
            break;
        case DateDay:
            m_day = value;
            break;
        case DateMonth:
            m_month = value;
            break;
        case DateYear:
            m_year = value;
            break;

        case StatusRegisterA:
            m_statusRegisterA = value;
            break;
        case StatusRegisterB:
            m_statusRegisterB = value;
            break;
        case StatusRegisterC:
            m_statusRegisterC = value;
            break;

        case FloppyDiskTypes:
            m_floppyDiskTypes = value;
            break;
        case SystemConfigurationSettings:
            m_systemConfigurationSettings = value;
            break;
        case HardDiskTypes:
            m_hardDiskTypes = value;
            break;
        case TypematicParameters:
            m_typematicParameters = value;
            break;
        case InstalledEquipment:
            m_installedEquipment = value;
            break;

        case BaseMemoryLSB:
            m_baseMemoryLSB = value;
            break;
        case BaseMemoryMSB:
            m_baseMemoryMSB = value;
            break;
        case ExtendedMemoryLSB:
            m_extendedMemoryLSB = value;
            break;
        case ExtendedMemoryMSB:
            m_extendedMemoryMSB = value;
            break;
        case HardDisk0Type:
            m_hardDisk0Type = value;
            break;
        case HardDisk1Type:
            m_hardDisk1Type = value;
            break;
        case ChecksumLSB:
            DC_CORE_WARN("[RTC]: New checksum LSB: {0}", value);
            m_checksumLSB = value;
            break;
        case ChecksumMSB:
            DC_CORE_WARN("[RTC]: New checksum MSB: {0}", value);
            m_checksumMSB = value;
            break;

        case Unused0:
        case Unused1:
        case Unused2:
        case Unused3:
        case Unused4:
        case Unused5:
            break;

        case DateCentury:
            m_century = value;
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
            byte = m_floppyDiskTypes; // Drive 0: 1.44MB | Drive 1: None
            break;
        case SystemConfigurationSettings:
            byte = m_systemConfigurationSettings;
            break;
        case HardDiskTypes:
            byte = m_hardDiskTypes;
            break;
        case TypematicParameters:
            byte = m_typematicParameters;
            break;
        case InstalledEquipment:
            byte = m_installedEquipment;
            break;

        case BaseMemoryLSB:
            byte = m_baseMemoryLSB;
            break;
        case BaseMemoryMSB:
            byte = m_baseMemoryMSB;
            break;

        case ExtendedMemoryLSB:
            byte = m_extendedMemoryLSB;
            break;
        case ExtendedMemoryMSB:
            byte = m_extendedMemoryMSB;
            break;

        case HardDisk0Type:
            byte = m_hardDisk0Type;
            break;
        case HardDisk1Type:
            byte = m_hardDisk1Type;
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
            byte = m_checksumLSB;
            break;
        case ChecksumMSB:
            byte = m_checksumMSB;
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
