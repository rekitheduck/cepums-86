#include "cepumspch.h"
#include "Processor.h"

namespace Cepums {

    Processor::Processor()
    {
        reset();
    }

    void Processor::reset()
    {
        m_flags = 0;
        m_instructionPointer = 0;
        m_codeSegment = 0xFFFF;
        m_dataSegment = 0;
        m_stackSegment = 0;
        m_extraSegment = 0;
        // TODO: Empty the queue here as well once that's implemented
    }

    void Processor::execute(MemoryManager& memoryManager)
    {
        if (m_cyclesToWait > 0)
        {
            m_cyclesToWait--;
            return;
        }
        auto hopefully_an_instruction = memoryManager.readByte(m_codeSegment, m_instructionPointer);
        m_instructionPointer++;

        m_AX = 0x1234;
        DC_CORE_WARN("Test02: Testing AH updating ...");
        DC_CORE_TRACE("AX: 0x{0:x}  AH: 0x{1:x},  AL 0x{2:x}", m_AX, AH(), AL());
        AH(0xEF);
        DC_CORE_TRACE("AX: 0x{0:x}  AH: 0x{1:x},  AL 0x{2:x}", m_AX, AH(), AL());
        if (m_AX == 0xEF34)
            DC_CORE_INFO("Test passed :)");
        else
            DC_CORE_ERROR("Test failed");

        DC_CORE_WARN("Test03: Testing AL updating ...");
        DC_CORE_TRACE("AX: 0x{0:x}  AH: 0x{1:x},  AL 0x{2:x}", m_AX, AH(), AL());
        AL(0x42);
        DC_CORE_TRACE("AX: 0x{0:x}  AH: 0x{1:x},  AL 0x{2:x}", m_AX, AH(), AL());
        if (m_AX == 0xEF42)
            DC_CORE_INFO("Test passed :)");
        else
            DC_CORE_ERROR("Test failed");

        // Don't forget to clean up after ourselves
        m_AX = 0;

        DC_CORE_INFO("CPU will now execute 0x{0:x}", hopefully_an_instruction);

        // Let's do some MOV instructions before the switch
        if (HIGHER_HALFBYTE(hopefully_an_instruction) == 0xB)
        {
            DC_CORE_INFO("Hit immediate MOV :)");

            // Decode the W-bit
            uint8_t isWord = (hopefully_an_instruction & BIT(3)) >> 3;

            // Get the 3 REG bits
            uint8_t regBits = hopefully_an_instruction & 0x7;

            if (isWord)
            {
                // Fetch a word of data
                uint16_t word = memoryManager.readWord(m_codeSegment, m_instructionPointer);
                m_instructionPointer += 2;

                // Get the actual register
                uint16_t& reg = getRegisterFromREG16(regBits);

                // And put the data where it belongs
                reg = word;
            }
            else
            {
                // Fetch a byte of data
                uint8_t byte = memoryManager.readByte(m_codeSegment, m_instructionPointer);
                m_instructionPointer++;

                updateRegisterFromREG8(regBits, byte);
            }

            return;
        }

        switch (hopefully_an_instruction)
        {
        case 0x0: // ADD: 8-bit from register to register/memory
        {
            auto byte = memoryManager.readByte(m_codeSegment, m_instructionPointer);
            m_instructionPointer++;

            uint8_t regBits = byte;
            REG(3, regBits);

            uint8_t modBits = byte;
            MOD(6, modBits);

            uint8_t rmBits = byte;
            MOD(0, rmBits);

            // Are we in register mode?
            if (modBits == 0b11)
            {
                m_instructionPointer++;
                return ins$ADDregisterToRegisterByte(rmBits, regBits);
            }

            uint8_t displacementLowByte = 0;
            uint8_t displacementHighByte = 0;

            // Do we have 8- or 16-bit displacement
            if (modBits == 0b01)
            {
                displacementLowByte = memoryManager.readByte(m_codeSegment, m_instructionPointer);
                m_instructionPointer++;
            }
            else if (modBits == 0b10)
            {
                displacementLowByte = memoryManager.readByte(m_codeSegment, m_instructionPointer);
                m_instructionPointer++;
                displacementHighByte = memoryManager.readByte(m_codeSegment, m_instructionPointer);
                m_instructionPointer++;
            }

            uint8_t reg = getRegisterValueFromREG8(regBits);
            uint16_t effectiveAddress = getEffectiveAddressFromBits(rmBits, regBits, modBits, 0, displacementLowByte, displacementHighByte);

            return ins$ADDregisterToMemory(memoryManager, effectiveAddress, reg);
        }

        case 0x1: // ADD: 16-bit from register to register/memory
        {
        }
        case 0x2:

            // HLT: Halt
        case 0xF4:
            return ins$HLT();

            // CLC: Clear carry bit
        case 0xF8:
            return ins$CLC();

            // CMC: Complement carry
        case 0xF5:
            return ins$CMC();

            // STC: Set carry
        case 0xF9:
            return ins$STC();

            // CLD: Clear direction
        case 0xFC:
            return ins$CLD();

            // STD: Set direction
        case 0xFD:
            return ins$STD();

            // CLI: Clear interrupt
        case 0xFA:
            return ins$CLI();

            // STI: Set interrupt
        case 0xFB:
            return ins$STI();

            // WAIT: Wait
        case 0x9B:
            return ins$WAIT();

        case 0xF0:
            // LOCK: Bus lock prefix
            return ins$LOCK();

        default:
            UNKNOWN_INSTRUCTION();
            break;
        }
        
        // TODO: Figure out what instruction we need to call
        TODO();

        m_cyclesToWait =+ 4; // This instruction takes 4 clock cycles to do its job

        // TODO: Add 4 clock cycles if accessing uneven address for operand
    }

    void Processor::ins$HLT()
    {
        DC_CORE_TRACE("Hit Halt");
        TODO();
    }

    void Processor::ins$CLC()
    {
        TODO();
    }

    void Processor::ins$CMC()
    {
        TODO();
    }

    void Processor::ins$STC()
    {
        TODO();
    }

    void Processor::ins$CLD()
    {
        TODO();
    }

    void Processor::ins$STD()
    {
        TODO();
    }

    void Processor::ins$CLI()
    {
        TODO();
    }

    void Processor::ins$STI()
    {
        TODO();
    }

    void Processor::ins$WAIT()
    {
        TODO();
    }

    void Processor::ins$LOCK()
    {
        TODO();
    }

    void Processor::ins$SEGMENT()
    {
        TODO();
    }

    void Processor::ins$ADDregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG)
    {
        auto operand = getRegisterValueFromREG8(sourceREG);
        auto operand2 = getRegisterValueFromREG8(destREG);
        updateRegisterFromREG8(destREG, operand + operand2);
    }

    void Processor::ins$ADDregisterToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint8_t sourceByte)
    {
        memoryManager.writeByte(m_dataSegment, effectiveAddress, sourceByte);
    }

    void Processor::ins$ADDregisterToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint16_t sourceWord)
    {
        memoryManager.writeWord(m_dataSegment, effectiveAddress, sourceWord);
    }

    void Processor::ins$MOVimmediateToRegisterWord(uint16_t& reg, uint16_t value)
    {
        reg = value;
    }

    void Processor::ins$MOVimmediateToRegisterByte(uint8_t& reg, uint8_t value)
    {
        reg = value;
    }

    void Processor::updateRegisterFromREG8(uint8_t REG, uint8_t data)
    {
        switch (REG)
        {
        case 0x0:
            return AL(data);

        case 0x1:
            return CL(data);

        case 0x2:
            return DL(data);

        case 0x3:
            return BL(data);

        case 0x4:
            return AH(data);

        case 0x5:
            return CH(data);

        case 0x6:
            return DH(data);

        case 0x7:
            return BH(data);

        default:
            DC_CORE_ERROR("Malformed REG bits : 0b{0:b}", REG);
            TODO();
            return;
        }
    }

    uint8_t Processor::getRegisterValueFromREG8(uint8_t REG)
    {
        switch (REG)
        {
        case 0x0:
            return AL();

        case 0x1:
            return CL();

        case 0x2:
            return DL();

        case 0x3:
            return BL();

        case 0x4:
            return AH();

        case 0x5:
            return CH();

        case 0x6:
            return DH();

        case 0x7:
            return BH();

        default:
            DC_CORE_ERROR("Malformed REG bits : 0b{0:b}", REG);
            TODO();
            return AL();
        }
    }

    uint16_t& Processor::getRegisterFromREG16(uint8_t REG)
    {
        switch (REG)
        {
        case 0x0:
            return m_AX;

        case 0x1:
            return m_CX;

        case 0x2:
            return m_DX;

        case 0x3:
            return m_BX;

        case 0x4:
            return m_stackPointer;

        case 0x5:
            return m_basePointer;

        case 0x6:
            return m_sourceIndex;

        case 0x7:
            return m_destinationIndex;

        default:
            DC_CORE_ERROR("Malformed REG bits : 0b{0:b}", REG);
            TODO();
            return m_AX;
        }
    }

    uint16_t Processor::getEffectiveAddressFromBits(uint8_t rmBits, uint8_t regBits, uint8_t modBits, uint8_t isWord, uint8_t displacementLow, uint8_t displacementHigh)
    {
        switch (modBits)
        {
        case 0b11:
            switch (rmBits)
            {
            case 0b000:
                return m_BX + m_sourceIndex;

            case 0b001:
                return m_BX + m_destinationIndex;

            case 0b010:
                return m_basePointer + m_sourceIndex;

            case 0b011:
                return m_basePointer + m_destinationIndex;

            case 0b100:
                return m_sourceIndex;

            case 0b101:
                return m_destinationIndex;

            case 0b110:
            {
                DC_CORE_TRACE("Getting EA from DIRECT ADDRESS");

                // TODO: Make sure these do the correct job
                uint16_t directAddress;
                SET8BITREGISTERLOW(directAddress, displacementLow);
                SET8BITREGISTERHIGH(directAddress, displacementHigh);
                return directAddress;
            }
            case 0b111:
                return m_BX;

            default:
                ILLEGAL_INSTRUCTION();
                return 0;
            }

        case 0b00:
            switch (rmBits)
            {
            case 0b000:
                return m_BX + m_sourceIndex;

            case 0b001:
                return m_BX + m_destinationIndex;

            case 0b010:
                return m_basePointer + m_sourceIndex;

            case 0b011:
                return m_basePointer + m_destinationIndex;

            case 0b100:
                return m_sourceIndex;

            case 0b101:
                return m_destinationIndex;

            case 0b110:
                TODO();
                return 0;

            case 0b111:
                return m_BX;

            default:
                ILLEGAL_INSTRUCTION();
                return 0;
            }

        case 0b01:
            DC_CORE_TRACE("R/M field decoding has entered the 'sign extension' branch. Currently that isn't done. Might cause issues");
            switch (rmBits)
            {
            case 0b000:
                return m_BX + m_sourceIndex + displacementLow;

            case 0b001:
                return m_BX + m_destinationIndex + displacementLow;

            case 0b010:
                return m_basePointer + m_sourceIndex + displacementLow;

            case 0b011:
                return m_basePointer + m_destinationIndex + displacementLow;

            case 0b100:
                return m_sourceIndex + displacementLow;

            case 0b101:
                return m_destinationIndex + displacementLow;

            case 0b110:
                return m_basePointer + displacementLow;

            case 0b111:
                return m_BX + displacementLow;

            default:
                ILLEGAL_INSTRUCTION();
                return 0;
            }

        case 0b10:
        {
            DC_CORE_TRACE("R/M field decoding has entered the 'displacement' branch. This might be implemented incorrectly");

            uint16_t fullDisplacement;

            // TODO: Make sure these do the correct job
            SET8BITREGISTERLOW(fullDisplacement, displacementLow);
            SET8BITREGISTERHIGH(fullDisplacement, displacementHigh);

            switch (rmBits)
            {
            case 0b000:
                return m_BX + m_sourceIndex + fullDisplacement;

            case 0b001:
                return m_BX + m_destinationIndex + fullDisplacement;

            case 0b010:
                return m_basePointer + m_sourceIndex + fullDisplacement;

            case 0b011:
                return m_basePointer + m_destinationIndex + fullDisplacement;

            case 0b100:
                return m_sourceIndex + fullDisplacement;

            case 0b101:
                return m_destinationIndex + fullDisplacement;

            case 0b110:
                return m_basePointer + fullDisplacement;

            case 0b111:
                return m_BX + fullDisplacement;

            default:
                ILLEGAL_INSTRUCTION();
                return 0;
            }
        }
        default:
            ILLEGAL_INSTRUCTION();
            return 0;
        }
    }
}
