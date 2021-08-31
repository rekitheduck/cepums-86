#include "cepumspch.h"
#include "PIT.h"

#define CHANNEL_BITS(pos, byte) byte >>= pos; byte &= 0x3
#define READ_WRITE_BITS(pos, byte) byte >>= pos; byte &= 0x3
#define MODE_BITS(pos, byte) byte >>= pos; byte &= 0x7
#define BCD_BIT(pos, byte) byte >>= pos; byte &= 0x1
#define SET8BITREGISTERHIGH(reg, data) reg &= 0x00FF; uint16_t temp = data << 8; reg |= temp & 0xFF00
#define SET8BITREGISTERLOW(reg, data) reg &= 0xFF00; reg |= data & 0x00FF

#define PARSE_SC_RW_MODE_BCD_BITS(byte, sc, rw, mode, bcd) uint8_t bcd = byte; BCD_BIT(0, bcd); uint8_t mode = byte; MODE_BITS(1, mode); uint8_t rw = byte; READ_WRITE_BITS(4, rw); uint8_t sc = byte; CHANNEL_BITS(6, sc)

namespace Cepums {

    uint8_t PIT::readCounter0()
    {
        DC_CORE_TRACE("PIT counter 0 reading value");
        return readCounter(0);
    }

    uint8_t PIT::readCounter1()
    {
        DC_CORE_TRACE("PIT counter 1 reading value");
        return readCounter(1);
    }

    uint8_t PIT::readCounter2()
    {
        DC_CORE_TRACE("PIT counter 2 reading value");
        return readCounter(2);
    }

    void PIT::writeControlRegister(uint8_t value)
    {
        PARSE_SC_RW_MODE_BCD_BITS(value, selectCounterBits, readWriteBits,  modeBits, BCDbit);

        // Figure out which read/write mode we need to use
        CounterReadWriteMode rwMode;
        switch (readWriteBits)
        {
        case 0b00: // Latch 
            switch (selectCounterBits)
            {
            case 0:
            case 1:
            case 2:
                m_counter[selectCounterBits].latched = m_counter[selectCounterBits].current;
                m_counter[selectCounterBits].isLatched = true;
                return;
            default:
                VERIFY_NOT_REACHED();
                break;
            }
            break;

        case 0b01:
            rwMode = CounterReadWriteMode::LeastSignificantOnly;
            break;

        case 0b10:
            rwMode = CounterReadWriteMode::MostSignificantOnly;
            break;

        case 0b11:
            rwMode = CounterReadWriteMode::LeastSignificantFirstThenMostSignificant;
            break;

        default:
            VERIFY_NOT_REACHED();
            break;
        }

        // Figure out which mode we need to use
        int mode;
        switch (modeBits)
        {
        case 0b000:
            mode = 0;
            break;

        case 0b001:
            mode = 1;
            break;

        case 0b010:
        case 0b110:
            mode = 2;
            break;

        case 0b011:
        case 0b111:
            mode = 3;
            break;

        case 0b100:
            mode = 4;
            break;

        case 0b101:
            mode = 5;
            break;

        default:
            VERIFY_NOT_REACHED();
            break;
        }

        switch (selectCounterBits)
        {
        case 0:
        case 1:
        case 2:
            m_counter[selectCounterBits].isInBCDmode = BCDbit;
            m_counter[selectCounterBits].mode = mode;
            m_counter[selectCounterBits].readWriteMode = rwMode;
            break;
        case 3: // Read-Back Command
            TODO();
            break;
        default:
            VERIFY_NOT_REACHED();
            break;
        }
        
        // We're expecting a new initial count to be provided
        m_counter[selectCounterBits].isInitialized = false;

        DC_CORE_TRACE("PIT control register called with mode={0} for counter {1}", mode, selectCounterBits);
    }

    void PIT::writeCounter0(uint8_t value)
    {
        DC_CORE_TRACE("PIT counter 0 (counter divisor) update with value=0x{0:x}", value);
        writeCounter(0, value);
    }

    void PIT::writeCounter1(uint8_t value)
    {
        DC_CORE_TRACE("PIT counter 1 (RAM refresh counter) update with value=0x{0:x}", value);
        writeCounter(1, value);
    }

    void PIT::writeCounter2(uint8_t value)
    {
        DC_CORE_TRACE("PIT counter 2 (cassette  and speaker) update with value=0x{0:x}", value);
        writeCounter(2, value);
    }

    PITState& PIT::update()
    {
        for (auto counter = 0; counter < 3; counter++)
        {
            if (!m_counter[counter].isInitialized)
                continue;

            // BCD mode
            if (m_counter[counter].isInBCDmode)
            {
                TODO();
            }

            switch (m_counter[counter].mode)
            {
            case 0:
                TODO();
                m_counter[counter].current--;
                // OUT0 is high when counterCurrent == 0
                break;

            case 1:
                TODO();
                m_counter[counter].current--;
                break;

            case 2: // I don't know the difference between this and #3 so lets pretend it works the same for now
                if (m_counter[counter].current != 0)
                    m_counter[counter].current--;
                else
                    m_counter[counter].current = m_counter[counter].initial;

                // This should turn on the output every 65536 if initial is set to 0
                if (m_counter[counter].current == m_counter[counter].initial)
                    m_counter[counter].output = true;
                else
                    m_counter[counter].output = false;
                break;

            case 3:
                if (m_counter[counter].current != 0)
                    m_counter[counter].current--;
                else
                    m_counter[counter].current = m_counter[counter].initial;

                // This should turn on the output every 65536 if initial is set to 0
                if (m_counter[counter].current == m_counter[counter].initial)
                    m_counter[counter].output = true;
                else
                    m_counter[counter].output = false;
                break;

            case 4:
                TODO();
                m_counter[counter].current--;
                break;

            case 5:
                TODO();
                m_counter[counter].current--;
                break;

            default:
                VERIFY_NOT_REACHED();
                break;
            }
        }
        
        // Update state
        if (m_counter[0].output)
            m_state.counter0output = true;
        else
            m_state.counter0output = false;

        if (m_counter[1].output)
            m_state.counter1output = true;
        else
            m_state.counter1output = false;

        if (m_counter[2].output)
            m_state.counter2output = true;
        else
            m_state.counter2output = false;

        return m_state;
    }

    void PIT::writeCounter(size_t counter, uint8_t value)
    {
        if (m_counter[counter].isInitialized)
        {
            // Bug in code somewhere (or weirdness to work around)
            VERIFY_NOT_REACHED();
            return;
        }

        switch (m_counter[counter].readWriteMode)
        {
        case CounterReadWriteMode::LeastSignificantOnly:
        {
            SET8BITREGISTERLOW(m_counter[counter].initial, value);
            m_counter[counter].isInitialized = true;
            m_counter[counter].current = 0xFFFF;
            break;
        }
        case CounterReadWriteMode::MostSignificantOnly:
        {
            SET8BITREGISTERHIGH(m_counter[counter].initial, value);
            m_counter[counter].isInitialized = true;
            m_counter[counter].current = 0xFFFF;
            break;
        }
        case CounterReadWriteMode::LeastSignificantFirstThenMostSignificant:
        {
            if (m_counter[counter].isUpdatingLowByte)
            {
                SET8BITREGISTERLOW(m_counter[counter].initial, value);
            }
            else
            {
                SET8BITREGISTERHIGH(m_counter[counter].initial, value);
                m_counter[counter].isInitialized = true;
                m_counter[counter].current = 0xFFFF;
            }
            m_counter[counter].isUpdatingLowByte = !m_counter[counter].isUpdatingLowByte;
            break;
        }
        default:
            VERIFY_NOT_REACHED();
            break;
        }
    }

    uint8_t PIT::readCounter(size_t counter)
    {
        TODO();
        return 0;
    }
}
