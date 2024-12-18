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
        // PIT counter 0
        if (address == 0x40)
            return m_8254PIT.readCounter0();

        // PIT counter 1
        if (address == 0x41)
            return m_8254PIT.readCounter1();

        // PIT counter 2
        if (address == 0x42)
            return m_8254PIT.readCounter2();

        // KBC Data Port
        if (address == 0x60)
            return m_8042KBC.readDataPort();

        // I don't know how to deal with this anymore, so let's log and ignore
        if (address == 0x61)
        {
            uint8_t data = 0;
            /*
            bit 7   parity check occurred
            bit 6   channel check occurred
            bit 5   mirrors timer 2 output condition
            bit 4   toggles with each refresh request
            bit 3   channel check status
            bit 2   parity check status
            bit 1   speaker data status
            bit 0   timer 2 gate to speaker status
            */
            if (m_refreshRequest)
            {
                SET_BIT(data, 4);
            }
            SET_BIT(data, 5);
            SET_BIT(data, 6);

            return data;
        }

        // XT: PPI Port C - read only
        if (address == 0x62)
        {
            uint8_t data = 0;

            // Video Switch
            // MDA
            SET_BIT(data, 0);
            SET_BIT(data, 1);

            return data;
        }

        // KBC Status Register
        if (address == 0x64)
            return m_8042KBC.readStatusRegister();

        // CMOS RAM/RTC data port
        if (address == 0x71)
            return m_RTC.readDataPort();

        // Serial port stuff? (not in PORTS.LST)
        if (address == 0x2E9)
            return 0;

        // Serial port stuff
        if (address == 0x2F9)
            return 0;

        // MDA Status register
        if (address == 0x3BA)
        {
            uint8_t data = 0;
            /*
            bit 4-7 always 1
            bit 3:  Video. This is 1 if a green or bright green pixel is being drawn on the screen at this moment
            bit 1-2 always 0
            bit 0   Retrace. This is 1 if the horizontal retrace is active
            */
            if (m_pretendRetrace)
            {
                SET_BIT(data, 0);
                m_pretendRetrace = false;
            }
            else
            {
                m_pretendRetrace = true;
            }
            SET_BIT(data, 4);
            SET_BIT(data, 5);
            SET_BIT(data, 6);
            SET_BIT(data, 7);

            return data;
        }

        // Parallel printer data port
        if (address == 0x03BC || address == 0x0378 || address == 0x0278)
            return 0;

        // CGA status register
        if (address == 0x3DA)
            DC_CORE_ERROR("Accessing a CGA status register - this shouldn't happen");

        // Serial port stuff? (not in PORTS.LST)
        if (address == 0x3E9)
            return 0;

        // Floppy status register A
        if (address == 0x3F0)
            return m_floppy.readStatusRegisterA();

        // Floppy status register B
        if (address == 0x3F1)
            return m_floppy.readStatusRegisterB();

        // Floppy main status register
        if (address == 0x3F4)
            return m_floppy.readMainStatusRegister();

        // Floppy data FIFO
        if (address == 0x3F5)
            return m_floppy.readDataFIFO();

        // Floppy digital input register
        if (address == 0x3F7)
            return m_floppy.readDigitalInputRegister();

        // Serial port stuff
        if (address == 0x3F9)
            return 0;

        DC_CORE_ERROR("IO: Attempt to read at 0x{0:X}", address);
        TODO();
        return 0;
    }

    void IOManager::writeByte(uint16_t address, uint8_t value)
    {

        // DMA Start Address Register channel 2/6 stub
        if (address == 0x04)
        {
            DC_CORE_TRACE("DMA: Start Address Register channel 2/6 : 0x{0:x}", value);
            return;
        }

        // DMA Count Register channel 2/6 stub
        if (address == 0x05)
        {
            DC_CORE_TRACE("DMA: Count Register channel 2/6: 0x{0:x}", value);
            return;
        }

        // DMA channel 0-3 command register stub
        if (address == 0x08)
        {
            // 7 bits literally control non-working functionality.

            // Bit 2 being 0 sets DMA on
            if (value) {
                // "Disable" DMA controller
                DC_CORE_WARN("DMA: Command Register: Disabling DMA Controller");
            }
            else
            {
                // Not handling this case
            }
            return;
        }

        // DMA Single channel mask register
        if (address == 0x0A)
        {
            DC_CORE_TRACE("DMA: Single channel mask register: 0x{0:x}", value);
            return;
        }

        // DMA channel 0-3 mode register
        if (address == 0x0B)
        {
            // If bits 2 and 3 are set, we are running a self test (nothing in our case)
            if (IS_BIT_NOT_SET(value, 2) && IS_BIT_NOT_SET(value, 3))
                return;

            if (IS_BIT_SET(value, 2) && IS_BIT_NOT_SET(value, 3))
            {
                DC_CORE_TRACE("DMA: \"Writing\" to memory");
                return;
            }

            if (IS_BIT_NOT_SET(value, 2) && IS_BIT_SET(value, 3))
            {
                DC_CORE_TRACE("DMA: \"Reading\" from memory");
                return;
            }

            if (IS_BIT_SET(value, 2) && IS_BIT_SET(value, 3))
            {

                DC_CORE_CRITICAL("DMA: Invalid transfer type 0b11");
                VERIFY_NOT_REACHED();
            }

            DC_CORE_ERROR("DMA: Unhandled mode register write");

            TODO();
            return;
        }

        // DMA flip-flop reset register stub
        if (address == 0x0C)
            return;

        // DMA master clear stub
        if (address == 0x0D)
            return;

        // PIC 1 register 0
        if (address == 0x20)
        {
            DC_CORE_TRACE("PIC1 register 0 stub");
            return;
        }

        // PIC 1 register 1
        if (address == 0x21)
        {
            DC_CORE_TRACE("PIC1 register 1 stub");
            return;
        }

        // PIT counter 0
        if (address == 0x40)
            return m_8254PIT.writeCounter0(value);

        // PIT counter 1
        if (address == 0x41)
            return m_8254PIT.writeCounter1(value);

        // PIT counter 2
        if (address == 0x42)
            return m_8254PIT.writeCounter2(value);

        // PIT control register
        if (address == 0x43)
            return m_8254PIT.writeControlRegister(value);

        // KBC Data Port
        if (address == 0x60)
            return m_8042KBC.writeDataPort(value);

        // KBC Command Register
        if (address == 0x64)
            return m_8042KBC.writeCommandRegister(value);

        // CMOS RAM/RTC index register
        if (address == 0x70)
            return m_RTC.writeIndexRegister(value);

        // CMOS RAM/RTC data port
        if (address == 0x71)
            return m_RTC.writeDataPort(value);

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

        // DMA page channel 2
        if (address == 0x81) {
            m_DMAPageChannel2 = value;
            return;
        }

        // DMA page channel 3
        if (address == 0x82) {
            m_DMAPageChannel3 = value;
            return;
        }

        // DMA page channel 0&1 (might just be 1?)
        if (address == 0x83) {
            m_DMAPageChannel0 = value;
            m_DMAPageChannel1 = value;
            return;
        }

        // I don't know how to deal with this anymore, so let's log and ignore
        if (address == 0x61)
        {
            DC_CORE_TRACE("Dummy write to PPI B control register with 0x{0:x}", value);
            return;
        }

        // XT: 8255 PPI control word register
        if (address == 0x63)
        {
            if (value != 0x99)
            {
                DC_CORE_TRACE("[PPI]: Unknown word register value - {0:X}", value);
                return;
            }
            return;
        }

        // PIC 2 register 0
        if (address == 0xA0)
        {
            DC_CORE_TRACE("PIC2 register 0 stub");
            return;
        }

        // PIC 2 register 1
        if (address == 0xA1)
        {
            DC_CORE_TRACE("PIC2 register 1 stub");
            return;
        }

        // Used as "Unused register", also 2nd DMA channel 4
        if (address == 0xC0)
            return;

        // DUCKMACHINE: Sector count
        if (address == 0xE0)
        {
            m_fakeFDC.setSectorCount(value);
            return;
        }

        // DUCKMACHINE: Command
        if (address == 0xE1)
        {
            m_fakeFDC.setCommand(value);
            return;
        }

        // DUCKMACHINE: Start cylinder
        if (address == 0xE6)
        {
            m_fakeFDC.setStartCylinder(value);
            return;
        }

        // DUCKMACHINE: Start sector
        if (address == 0xE7)
        {
            m_fakeFDC.setStartSector(value);
            return;
        }

        // DUCKMACHINE: Head
        if (address == 0xE8)
        {
            m_fakeFDC.setHead(value);
            return;
        }


        // Expansion unit (XT)
        if (address == 0x213)
        {
            switch (value)
            {
            case 0:
                DC_CORE_TRACE("[XT] Disabling expansion unit (fake)");
                return;
            case 1:
                DC_CORE_TRACE("[XT] Enabling expansion unit (fake)");
                return;
            default:
                DC_CORE_TRACE("[XT] Unknown value ({0}) for enable/disable expansion unit, ignored", value);
                return;
            }
        }

        // Serial port stuff? (not in PORTS.LST)
        if (address == 0x2E9)
            return;

        // Serial port stuff
        if (address == 0x2F9)
            return;

        // MDA Set current CRTC data register (accessable on 0x03B5 then)
        if (address == 0x03B4)
            return;

        // MDA Set current CRTC data register (accessable on 0x03B5 then)
        if (address == 0x03B5)
            return;

        // MDA mode control register
        if (address == 0x03B8)
        {
            // Disable video output and set the high-resolution bit
            if (value == 0x01)
            {
                DC_CORE_TRACE("[MDA]: Disable output and set high-res bit");
                return;
            }

            // Mode set
            if (value == 0x29)
            {
                DC_CORE_TRACE("[MDA]: Set mode");
                return;
            }
            DC_CORE_ERROR("[MDA]: Unhandled register value {0:X}", value);
            return;
        }

        // CGA color select register
        if (address == 0x3B9)
        {
            DC_CORE_TRACE("[CGA] Dummy color select register write with value {0}", value);
            return;
        }

        // Parallel printer data port
        if (address == 0x03BC || address == 0x0378 || address == 0x0278)
            return;

        // CGA
        if (address == 0x03D4 || address == 0x03D5 || address == 0x3D9)
            return;

        // CGA mode control register
        if (address == 0x03D8)
        {
            // Disable video output on CGA
            if (value == 0)
                return;

            // We're being asked to enable CGA. Uh oh
            if (value == 1)
                return;

            // Ignore the rest too
            return;
        }

        // Serial port stuff? (not in PORTS.LST)
        if (address == 0x3E9)
            return;

        // Floppy digital output register (motors, selection and reset)
        if (address == 0x3F2)
        {
            // Generate a FDC interrupt after some time if bit 3 is set
            if (IS_BIT_SET(value, 3))
            {
                m_floppyDelayingForInterrupt = true;
                m_floppyInterruptCounter = 100;
            }
            return m_floppy.writeDigitalOutputRegister(value);
        }

        // Floppy data FIFO
        if (address == 0x3F5)
        {
            m_floppy.writeDataFIFO(value);
            if (m_floppy.performInterruptAfterFIFO())
            {
                m_floppyDelayingForInterrupt = true;
                m_floppyInterruptCounter = 100;
            }
            return;
        }

        // Floppy configuration control register
        if (address == 0x3F7)
            return m_floppy.writeConfigurationControlRegister(value);

        // Serial port stuff
        if (address == 0x3F9)
            return;

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
        // DUCKMACHINE: Upper memory value
        if (address == 0xE2) {
            m_fakeFDC.setUpperAddress(value);
            return;
        }

        // DUCKMACHINE: Lower memory value
        if (address == 0xE4) {
            m_fakeFDC.setLowerAddress(value);
            return;
        }

        DC_CORE_ERROR("IO: Attempt to write at 0x{0:X} with data 0x{1:X}", address, value);
        TODO();
    }

    void IOManager::runPIT()
    {
        // Generate a floppy interrupt if we need to do that and the delay counter reaches 0
        if (m_floppyDelayingForInterrupt)
        {
            m_floppyInterruptCounter--;
            if (m_floppyInterruptCounter == 0)
            {
                m_pendingInterrupt = true;
                m_floppyDelayingForInterrupt = false;
                m_interrupt = 0xE; // IRQ6
            }
        }

        // Update the PIT
        PITState& state = m_8254PIT.update();
        if (state.counter1output)
            m_refreshRequest = true;
        else
            m_refreshRequest = false;
    }

    bool IOManager::hasPendingInterrupts()
    {
        return m_pendingInterrupt;
    }

    uint16_t IOManager::getPendingInterrupt()
    {
        m_pendingInterrupt = false;
        return m_interrupt;
    }

    void IOManager::onKeyPress(SDL_Scancode scancode)
    {
        std::lock_guard<std::mutex> guard(m_keyboardMutex);
        // Tell KBC to return a key press
        m_8042KBC.keyPressed(scancode);

        m_pendingInterrupt = true;
        m_interrupt = 9; // IRQ1
    }

    void IOManager::onKeyRelease(SDL_Scancode scancode)
    {
        std::lock_guard<std::mutex> guard(m_keyboardMutex);
        m_8042KBC.keyReleased(scancode);

        m_pendingInterrupt = true;
        m_interrupt = 9; // IRQ1
    }
}
