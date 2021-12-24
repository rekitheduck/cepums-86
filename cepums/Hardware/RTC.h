#pragma once

namespace Cepums {

    class RTC
    {
    public:
        void writeIndexRegister(uint8_t value) { m_mode = value; }
        void writeDataPort(uint8_t value);
        uint8_t readDataPort();
    private:
        // 12:48:20
        uint8_t m_seconds = 0x20;
        uint8_t m_minutes = 0x48;
        uint8_t m_hours = 0x12;

        // 09.09.2021
        uint8_t m_day = 0x9;
        uint8_t m_month = 0x9;
        uint8_t m_year = 0x21;
        uint8_t m_century = 0x20;

        uint8_t m_statusRegisterA = 0;
        uint8_t m_statusRegisterB = 4; // Enable DST and 24hr clock
        uint8_t m_statusRegisterC = 0; // System status

        uint8_t m_floppyDiskTypes = 0x40; // Drive 0: 1.44MB | Drive 1: None
        uint8_t m_systemConfigurationSettings = 0x8B; // A bunch of things
        uint8_t m_hardDiskTypes = 0x00; // No hard disks
        uint8_t m_typematicParameters = 0x00; // No typematic stuff
        uint8_t m_installedEquipment = 0x3D; // A bunch of things (including display adapter)
        uint8_t m_baseMemoryLSB = 0x80; // 640KB base memory
        uint8_t m_baseMemoryMSB = 0x02; // 640KB base memory
        uint8_t m_extendedMemoryLSB = 0x00; // 0KB extended memory
        uint8_t m_extendedMemoryMSB = 0x00; // 0KB extended memory
        uint8_t m_hardDisk0Type = 0x00; // No hard disk 0
        uint8_t m_hardDisk1Type = 0x00; // No hard disk 1

        uint8_t m_systemOperationalFlags = 0x74; // A bunch of stuff
        uint8_t m_checksumLSB = 0x0E;
        uint8_t m_checksumMSB = 0x02;

        int m_mode = -1;
    };
}
