#include "cepumspch.h"
#include "IOManager.h"

#define KIBIBYTE 1024
#define MEBIBYTE 1048576

namespace Cepums {

    IOManager::IOManager()
    {
    }

    uint8_t IOManager::readByte(uint16_t address)
    {
        // Dummy read from CMOS RAM/RTC
        if (address == 0x71)
        {
            DC_CORE_TRACE("Dummy read from CMOS");
            return 0;
        }

        TODO();
        return 0;
    }

    void IOManager::writeByte(uint16_t address, uint8_t value)
    {
        // DMA channel 0-3 command register stub
        if (address == 0x08)
            return;

        // DMA channel 0-3 mode register stub
        if (address == 0x0B)
            return;

        // DMA master clear stub
        if (address == 0x0D)
            return;

        // POST register
        if (address == 0x80)
        {
            m_port0x80 = value;

            DC_CORE_WARN("POST CODE - {0:X}", value);
            switch (value)
            {
            case 0x00:
                return DC_CORE_TRACE("Boot the OS");
            case 0x01:
                return DC_CORE_TRACE("Start of BIOS POST, CPU test");
            case 0x02:
                return DC_CORE_TRACE("Initial chipset configuration: init PPI, disable NMI, disable turbo, disable display");
            case 0x03:
                return DC_CORE_TRACE("Initialize DMA controller");
            case 0x04:
                return DC_CORE_TRACE("Test low 32KiB of RAM");
            case 0x05:
                return DC_CORE_TRACE("Initialize interrupt table");
            case 0x06:
                return DC_CORE_TRACE("Initialize PIT (timer); Player power-on melody");
            case 0x07:
                return DC_CORE_TRACE("Initialize PIC");
            case 0x08:
                return DC_CORE_TRACE("Initialize KBC and keyboard");
            case 0x09:
                return DC_CORE_TRACE("Enable interrupts");

            case 0x10:
                return DC_CORE_TRACE("Locate video BIOS");
            case 0x11:
                return DC_CORE_TRACE("Initialize video BIOS");
            case 0x12:
                return DC_CORE_TRACE("No video BIOS, using MDA/CGA");

            case 0x20:
                return DC_CORE_TRACE("Initialize RTC");
            case 0x21:
                return DC_CORE_TRACE("Detect CPU type");
            case 0x22:
                return DC_CORE_TRACE("Detect FPU");
            case 0x24:
                return DC_CORE_TRACE("Detect serial ports");
            case 0x25:
                return DC_CORE_TRACE("Detect parallel ports");

            case 0x30:
                return DC_CORE_TRACE("Start RAM test");
            case 0x31:
                return DC_CORE_TRACE("RAM test completed");
            case 0x32:
                return DC_CORE_TRACE("RAM test cancelled");

            case 0x40:
                return DC_CORE_TRACE("Start BIOS extension ROM scan");
            case 0x41:
                return DC_CORE_TRACE("BIOS extension ROM found, initialize");
            case 0x42:
                return DC_CORE_TRACE("BIOS extension ROM initialized");
            case 0x43:
                return DC_CORE_TRACE("BIOS extension scan complete");

            case 0x52:
                return DC_CORE_TRACE("CPU test failed");
            case 0x54:
                return DC_CORE_TRACE("Low 32 KiB RAM test failed");
            case 0x55:
                return DC_CORE_TRACE("RAM test failed");

            case 0x60:
                return DC_CORE_TRACE("Unable to flush KBC output buffer");
            case 0x61:
                return DC_CORE_TRACE("Unable to send command to KBC");
            case 0x62:
                return DC_CORE_TRACE("Keyboard controller self test failed");
            case 0x63:
                return DC_CORE_TRACE("Keyboard interface test failed");

            case 0x70:
                return DC_CORE_TRACE("Keyboard BAT test failed");
            case 0x71:
                return DC_CORE_TRACE("Keyboard disable command failed");
            case 0x72:
                return DC_CORE_TRACE("Keyboard enable command failed");
            default:
                return DC_CORE_TRACE("UNKNOWN POST code / possible bug");
            }
            return;
        }

        // DMA page channel 2, 3, (0,1) stub
        if (address == 0x81 || address == 0x82 || address == 0x83)
            return;

        // I don't know how to deal with this anymore, so let's log and ignore
        if (address == 0x61)
        {
            DC_CORE_TRACE("Dummy write to Programmable Peripheral Interface (PPI) B control register");
            return;
        }

        // Ignore CMOS RAM/RTC calls for now :(
        if (address == 0x70 || address == 0x71)
        {
            DC_CORE_TRACE("Dummy write to CMOS");
            return;
        }

        // MDA mode control register
        if (address == 0x03B8)
        {
            // Disable video output and set the high-resolution bit
            if (value == 0x01)
                return;
        }

        // CGA mode control register
        if (address == 0x03D8)
        {
            // Disable video output on CGA
            if (value == 0)
                return;
        }

        DC_CORE_ERROR("IO: Attempt to write at 0x{0:X} with data 0x{1:X}", address, value);
        TODO();
    }

    uint16_t IOManager::readWord(uint16_t address)
    {
        TODO();
        return 0;
    }

    void IOManager::writeWord(uint16_t address, uint16_t value)
    {
        TODO();
    }
}
