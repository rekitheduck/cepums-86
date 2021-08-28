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
        // I don't know how to deal with this anymore, so let's log and ignore
        if (address == 0x61)
        {
            DC_CORE_TRACE("Dummy read from Programmable Peripheral Interface (PPI) B control register");
            return 0;
        }

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

        // PIT counter 0
        if (address == 0x40)
        {
            DC_CORE_TRACE("PIT counter 0 (counter divisor) stub :(");
            return;
        }

        // PIT counter 1
        if (address == 0x41)
        {
            DC_CORE_TRACE("PIT counter 1 (RAM refresh counter) stub :(");
            return;
        }

        // PIT counter 2
        if (address == 0x42)
        {
            DC_CORE_TRACE("PIT counter 2 (cassette  and speaker) stub :(");
            return;
        }

        // PIT mode port
        if (address == 0x43)
        {
            DC_CORE_TRACE("PIT stub :(");
            return;
        }

        // POST register
        if (address == 0x80)
        {
            m_port0x80 = value;
            switch (value)
            {
            case 0x00:
                return DC_CORE_WARN("POST[{0}]: Boot the OS", value);
            case 0x01:
                return DC_CORE_WARN("POST[{0}]: Start of BIOS POST, CPU test", value);
            case 0x02:
                return DC_CORE_WARN("POST[{0}]: Initial chipset configuration: init PPI, disable NMI, disable turbo, disable display", value);
            case 0x03:
                return DC_CORE_WARN("POST[{0}]: Initialize DMA controller", value);
            case 0x04:
                return DC_CORE_WARN("POST[{0}]: Test low 32KiB of RAM", value);
            case 0x05:
                return DC_CORE_WARN("POST[{0}]: Initialize interrupt table", value);
            case 0x06:
                return DC_CORE_WARN("POST[{0}]: Initialize PIT (timer); Player power-on melody", value);
            case 0x07:
                return DC_CORE_WARN("POST[{0}]: Initialize PIC", value);
            case 0x08:
                return DC_CORE_WARN("POST[{0}]: Initialize KBC and keyboard", value);
            case 0x09:
                return DC_CORE_WARN("POST[{0}]: Enable interrupts", value);

            case 0x10:
                return DC_CORE_WARN("POST[{0}]: Locate video BIOS", value);
            case 0x11:
                return DC_CORE_WARN("POST[{0}]: Initialize video BIOS", value);
            case 0x12:
                return DC_CORE_WARN("POST[{0}]: No video BIOS, using MDA/CGA", value);

            case 0x20:
                return DC_CORE_WARN("POST[{0}]: Initialize RTC", value);
            case 0x21:
                return DC_CORE_WARN("POST[{0}]: Detect CPU type", value);
            case 0x22:
                return DC_CORE_WARN("POST[{0}]: Detect FPU", value);
            case 0x24:
                return DC_CORE_WARN("POST[{0}]: Detect serial ports", value);
            case 0x25:
                return DC_CORE_WARN("POST[{0}]: Detect parallel ports", value);

            case 0x30:
                return DC_CORE_WARN("POST[{0}]: Start RAM test", value);
            case 0x31:
                return DC_CORE_WARN("POST[{0}]: RAM test completed", value);
            case 0x32:
                return DC_CORE_WARN("POST[{0}]: RAM test cancelled", value);

            case 0x40:
                return DC_CORE_WARN("POST[{0}]: Start BIOS extension ROM scan", value);
            case 0x41:
                return DC_CORE_WARN("POST[{0}]: BIOS extension ROM found, initialize", value);
            case 0x42:
                return DC_CORE_WARN("POST[{0}]: BIOS extension ROM initialized", value);
            case 0x43:
                return DC_CORE_WARN("POST[{0}]: BIOS extension scan complete", value);

            case 0x52:
                return DC_CORE_WARN("POST[{0}]: CPU test failed", value);
            case 0x54:
                return DC_CORE_WARN("POST[{0}]: Low 32 KiB RAM test failed", value);
            case 0x55:
                return DC_CORE_WARN("POST[{0}]: RAM test failed", value);

            case 0x60:
                return DC_CORE_WARN("POST[{0}]: Unable to flush KBC output buffer", value);
            case 0x61:
                return DC_CORE_WARN("POST[{0}]: Unable to send command to KBC", value);
            case 0x62:
                return DC_CORE_WARN("POST[{0}]: Keyboard controller self test failed", value);
            case 0x63:
                return DC_CORE_WARN("POST[{0}]: Keyboard interface test failed", value);

            case 0x70:
                return DC_CORE_WARN("POST[{0}]: Keyboard BAT test failed", value);
            case 0x71:
                return DC_CORE_WARN("POST[{0}]: Keyboard disable command failed", value);
            case 0x72:
                return DC_CORE_WARN("POST[{0}]: Keyboard enable command failed", value);
            default:
                return DC_CORE_WARN("POST[{0}]: UNKNOWN POST code / possible bug", value);
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
