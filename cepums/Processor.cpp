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

        DC_CORE_INFO("{0}: ===== NEW CYCLE =====", m_currentCycleCounter++);
        DC_CORE_TRACE(" AX: 0x{0:x}   BX: 0x{1:x}   CX: 0x{2:x}   DX: 0x{3:x}", AX(), BX(), CX(), DX());
        DC_CORE_TRACE(" DS: 0x{0:x}   CS: 0x{1:x}   SS: 0x{2:x}   ES: 0x{3:x}", DS(), CS(), SS(), ES());
        DC_CORE_TRACE(" IP: 0x{0:x}", IP());

        uint8_t hopefully_an_instruction = memoryManager.readByte(m_codeSegment, m_instructionPointer);
        m_instructionPointer++;

        DC_CORE_TRACE("Fetched new instruction 0x{0:x}", hopefully_an_instruction);

#if 0
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
#endif

        switch (hopefully_an_instruction)
        {
        case 0x00: // ADD: 8-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$ADDregisterToRegisterByte(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte);

            return ins$ADDregisterToMemory(memoryManager, effectiveAddress, getRegisterValueFromREG8(regBits));
        }
        case 0x01: // ADD: 16-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$ADDregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits,  displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte);

            return ins$ADDregisterToMemory(memoryManager, effectiveAddress, getRegisterFromREG16(regBits));
        }
        case 0x02: // ADD: 8-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x03: // ADD: 16-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x04: // ADD: 8-bit immediate to AL
        {
            DC_CORE_WARN("ADD: 8-bit immediate to AL");
            TODO(); // flags
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            AL(byte + AL());

            return;
        }
        case 0x05: // ADD: 16-bit immediate to AX
        {
            DC_CORE_WARN("ADD: 16-bit immediate to AX");
            TODO(); // flags
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, word);
            AX() += word;

            return;
        }
        case 0x06: // PUSH: Push ES to stack
        {
            TODO();
            return;
        }
        case 0x07: // POP: Pop ES from stack
        {
            TODO();
            return;
        }
        case 0x08: // OR: 8-bit register with register/memory
        {
            TODO();
            return;
        }
        case 0x09: // OR: 16-bit register with register/memory
        {
            TODO();
            return;
        }
        case 0x0A: // OR: 8-bit register/memory with register
        {
            TODO();
            return;
        }
        case 0x0B: // OR: 16-bit register/memory with register
        {
            TODO();
            return;
        }
        case 0x0C: // OR: 8-bit immediate with AL
        {
            TODO();
            return;
        }
        case 0x0D: // OR: 16-bit immediate with AX
        {
            TODO();
            return;
        }
        case 0x0E: // PUSH: Push CS to stack
        {
            TODO();
            return;
        }
        case 0x10: // ADC: 8-bit from register to register/memory
        {
            TODO();
            return;
        }
        case 0x11: // ADC: 16-bit from register to register/memory
        {
            TODO();
            return;
        }
        case 0x12: // ADC: 8-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x13: // ADC: 16-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x14: // ADC: 8-bit immediate to AL
        {
            TODO();
            return;
        }
        case 0x15: // ADC: 16-bit immediate to AX
        {
            TODO();
            return;
        }
        case 0x16: // PUSH: Push SS to stack
        {
            TODO();
            return;
        }
        case 0x017: // POP: Pop SS from stack
        {
            TODO();
            return;
        }
        case 0x18: // SBB: 8-bit subtract with borrow from register to register/memory
        {
            TODO();
            return;
        }
        case 0x19: // SBB: 16-bit subtract with borrow from register to register/memory
        {
            TODO();
            return;
        }
        case 0x1A: // SBB: 8-bit subtract with borrow from register/memory to register
        {
            TODO();
            return;
        }
        case 0x1B: // SBB: 16-bit subtract with borrow from register/memory to register
        {
            TODO();
            return;
        }
        case 0x1C: // SBB: 8-bit subtract with borrow from immediate to AL
        {
            TODO();
            return;
        }
        case 0x1D: // SBB: 16-bit subtract with borrow from immediate to AX
        {
            TODO();
            return;
        }
        case 0x1E: // PUSH: Push DS to stack
        {
            TODO();
            return;
        }
        case 0x1F: // POP: Pop DS from stack
        {
            TODO();
            return;
        }
        case 0x20: // AND: 8-bit from register to register/memory
        {
            TODO();
            return;
        }
        case 0x21: // AND: 16-bit from register to register/memory
        {
            TODO();
            return;
        }
        case 0x22: // AND: 8-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x23: // AND: 16-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x24: // AND: 8-bit immediate with AL
        {
            TODO();
            return;
        }
        case 0x25: // AND: 16-bit immediate with AX
        {
            TODO();
            return;
        }
        case 0x26: // ES: Segment override prefix
        {
            // ???????????
            TODO();
            return;
        }
        case 0x27: // DAA: Decimal adjust for addition
        {
            TODO();
            return;
        }
        case 0x28: // SUB: 8-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$SUBregisterToRegisterByte(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte);

            return ins$SUBregisterToMemory(memoryManager, effectiveAddress, getRegisterValueFromREG8(regBits));
        }
        case 0x29: // SUB: 16-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$SUBregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte);

            return ins$SUBregisterToMemory(memoryManager, effectiveAddress, getRegisterFromREG16(regBits));
        }
        case 0x2A: // SUB: 8-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x2B: // SUB: 16-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x2C: // SUB: 8-bit immediate with AL
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            return ins$SUBimmediateToRegister(REGISTER_AL, byte);
        }
        case 0x2D: // SUB: 16-bit immediate with AX
        {
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, word);
            return ins$SUBimmediateToRegister(REGISTER_AL, word);
        }
        case 0x2E: // CS: Segment override prefix
        {
            // ???????????
            TODO();
            return;
        }
        case 0x2F: // DAS: Decimal adjust for subtraction
        {
            TODO();
            return;
        }
        case 0x30: // XOR: 8-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$XORregisterToRegisterByte(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte);

            return ins$XORregisterToMemory(memoryManager, effectiveAddress, getRegisterValueFromREG8(regBits));
        }
        case 0x31: // XOR: 16-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$XORregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte);

            return ins$XORregisterToMemory(memoryManager, effectiveAddress, getRegisterFromREG16(regBits));
        }
        case 0x32: // XOR: 8-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x33: // XOR: 16-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x34: // XOR: 8-bit immediate with AL
        {
            TODO();
            return;
        }
        case 0x35: // XOR: 16-bit immediate with AX
        {
            TODO();
            return;
        }
        case 0x36: // SS: Segment override prefix
        {
            // ???????????
            TODO();
            return;
        }
        case 0x37: // AAA: ASCII adjust for addition
        {
            TODO();
            return;
        }
        case 0x38: // CMP: 8-bit from register to register/memory
        {
            TODO();
            return;
        }
        case 0x39: // CMP: 16-bit from register to register/memory
        {
            TODO();
            return;
        }
        case 0x3A: // CMP: 8-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x3B: // CMP: 16-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x3C: // CMP: 8-bit immediate with AL
        {
            TODO();
            return;
        }
        case 0x3D: // CMP: 16-bit immediate with AX
        {
            TODO();
            return;
        }
        case 0x3E: // DS: Segment override prefix
        {
            // ???????????
            TODO();
            return;
        }
        case 0x3F: // AAS: ASCII adjust for subtraction
        {
            TODO();
            return;
        }
        case 0x40: // INC: AX
        {
            return ins$INC(IS_WORD, REGISTER_AX);
        }
        case 0x41: // INC: CX
        {
            TODO();
            return;
        }
        case 0x42: // INC: DX
        {
            TODO();
            return;
        }
        case 0x43: // INC: BX
        {
            TODO();
            return;
        }
        case 0x44: // INC: SP
        {
            TODO();
            return;
        }
        case 0x45: // INC: BP
        {
            TODO();
            return;
        }
        case 0x46: // INC: SI
        {
            TODO();
            return;
        }
        case 0x47: // INC: DI
        {
            TODO();
            return;
        }
        case 0x48: // DEC: AX
        {
            TODO();
            return;
        }
        case 0x49: // DEC: CX
        {
            TODO();
            return;
        }
        case 0x4A: // DEC: DX
        {
            TODO();
            return;
        }
        case 0x4B: // DEC: BX
        {
            TODO();
            return;
        }
        case 0x4C: // DEC: SP
        {
            TODO();
            return;
        }
        case 0x4D: // DEC: BP
        {
            TODO();
            return;
        }
        case 0x4E: // DEC: SI
        {
            TODO();
            return;
        }
        case 0x4F: // DEC: DI
        {
            TODO();
            return;
        }
        case 0x50: // PUSH: AX
        {
            TODO();
            return;
        }
        case 0x51: // PUSH: CX
        {
            TODO();
            return;
        }
        case 0x52: // PUSH: DX
        {
            TODO();
            return;
        }
        case 0x53: // PUSH: BX
        {
            TODO();
            return;
        }
        case 0x54: // PUSH: SP
        {
            TODO();
            return;
        }
        case 0x55: // PUSH: BP
        {
            TODO();
            return;
        }
        case 0x56: // PUSH: SI
        {
            TODO();
            return;
        }
        case 0x57: // PUSH: DI
        {
            TODO();
            return;
        }
        case 0x58: // POP: AX
        {
            TODO();
            return;
        }
        case 0x59: // POP: CX
        {
            TODO();
            return;
        }
        case 0x5A: // POP: DX
        {
            TODO();
            return;
        }
        case 0x5B: // POP: BX
        {
            TODO();
            return;
        }
        case 0x5C: // POP: SP
        {
            TODO();
            return;
        }
        case 0x5D: // POP: BP
        {
            TODO();
            return;
        }
        case 0x5E: // POP: SI
        {
            TODO();
            return;
        }
        case 0x5F: // POP: DI
        {
            TODO();
            return;
        }
        case 0x70: // JO: Jump if overflow
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if OF=1");
            if (IS_BIT_SET(m_flags, OVERFLOW_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x71: // JNO: Jump if no overflow
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if OF=0");
            if (IS_BIT_NOT_SET(m_flags, OVERFLOW_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x72: // JB/JNAE/JC: Jump if below / Jump if not above nor equal / Jump if carry
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if CF=1");
            if (IS_BIT_SET(m_flags, CARRY_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x73: // JNB/JAE/JNC: Jump if not below / Jump if above or equal / Jump if not carry
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if CF=0");
            if (IS_BIT_NOT_SET(m_flags, CARRY_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x74: // JE/JZ: Jump if equal / Jump if zero
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if ZF=1");
            if (IS_BIT_SET(m_flags, ZERO_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x75: // JNE/JNZ: Jump if not equal / Jump if not zero
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if ZF=0");
            if (IS_BIT_NOT_SET(m_flags, ZERO_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x76: // JBE/JNA: Jump if below or equal / Jump if not above
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if CF=1 || ZF=1");
            if (IS_BIT_SET(m_flags, CARRY_FLAG) || IS_BIT_SET(m_flags, ZERO_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x77: // JNBE/JA: Jump if not below nor equal / Jump if above
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if CF=0 && ZF=0");
            if (IS_BIT_NOT_SET(m_flags, CARRY_FLAG) && IS_BIT_NOT_SET(m_flags, ZERO_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x78: // JS: Jump if sign
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if SF=1");
            if (IS_BIT_SET(m_flags, SIGN_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x79: // JNS: Jump if not sign
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if SF=0");
            if (IS_BIT_NOT_SET(m_flags, SIGN_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x7A: // JP/JPE: Jump if parity / Jump if parity even
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if PF=1");
            if (IS_BIT_SET(m_flags, PARITY_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x7B: // JNP/NPO: Jump if not parity / Jump if parity odd
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if PF=0");
            if (IS_BIT_NOT_SET(m_flags, PARITY_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x7C: // JL/JNGE: Jump if less / Jump if not greater nor equal
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if SF!=OF");
            if (IS_BIT_SET(m_flags, SIGN_FLAG) != IS_BIT_SET(m_flags, OVERFLOW_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x7D: // JNL/JGE: Jump if greater or equal / Jump if not less
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if SF=OF");
            if (IS_BIT_SET(m_flags, SIGN_FLAG) == IS_BIT_SET(m_flags, OVERFLOW_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x7E: // JLE/JNG: Jump if less or equal / Jump if not greater
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if ZF=1 || (SF!=OF)");
            if (IS_BIT_SET(m_flags, ZERO_FLAG) || ( IS_BIT_SET(m_flags, SIGN_FLAG) != IS_BIT_SET(m_flags, OVERFLOW_FLAG)))
                return ins$JMPshort(increment);
            return;
        }
        case 0x7F: // JNLE/JG: Jump if not less nor equal / Jump if greater
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            DC_CORE_WARN("ins$JMP: Jumping if ZF=0 && (SF=OF)");
            if (IS_BIT_NOT_SET(m_flags, ZERO_FLAG) && (IS_BIT_SET(m_flags, SIGN_FLAG) == IS_BIT_SET(m_flags, OVERFLOW_FLAG)))
                return ins$JMPshort(increment);
            return;
        }
        case 0x80: // ADD/OR/ADC/SBB/AND/SUB/XOR/CMP: 8-bit immediate to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
            {
                LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
                switch (regBits)
                {
                case 0b000:
                    return ins$ADDimmediateToRegister(rmBits, immediate);
                case 0b001:
                    //return ins$ORimmediateToRegisterByte(rmBits, immediate);
                case 0b010:
                    //return ins$ADCimmediateToRegisterByte(rmBits, immediate);
                case 0b011:
                    //return ins$SBBimmediateToRegisterByte(rmBits, immediate);
                case 0b100:
                    //return ins$ANDimmediateToRegisterByte(rmBits, immediate);
                case 0b101:
                    //return ins$SUBimmediateToRegisterByte(rmBits, immediate);
                case 0b110:
                    //return ins$XORimmediateToRegisterByte(rmBits, immediate);
                case 0b111:
                    //return ins$CMPimmediateToRegisterByte(rmBits, immediate);
                default:
                    ILLEGAL_INSTRUCTION();
                    return;
                }
            }

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte);
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);

            switch (regBits)
            {
            case 0b000:
                return ins$ADDimmediateToMemory(memoryManager, effectiveAddress, immediate);
            case 0b001:
                //return ins$ORimmediateToMemory(memoryManager, effectiveAddress, immediate);
            case 0b010:
                //return ins$ADCimmediateToMemory(memoryManager, effectiveAddress, immediate);
            case 0b011:
                //return ins$SBBimmediateToMemory(memoryManager, effectiveAddress, immediate);
            case 0b100:
                //return ins$ANDimmediateToMemory(memoryManager, effectiveAddress, immediate);
            case 0b101:
                //return ins$SUBimmediateToMemory(memoryManager, effectiveAddress, immediate);
            case 0b110:
                //return ins$XORimmediateToMemory(memoryManager, effectiveAddress, immediate);
            case 0b111:
                //return ins$CMPimmediateToMemory(memoryManager, effectiveAddress, immediate);
            default:
                ILLEGAL_INSTRUCTION();
                return;
            }
        }
        case 0x81: // ADD/OR/ADC/SBB/AND/SUB/XOR/CMP: 16-bit immediate to register/memory
        {
            TODO();
            return;
        }
        case 0x82: // ADD/unused/ADC/SBB/unused/SUB/unused/CMP: 8-bit immediate to register/memory
        {
            TODO();
            return;
        }
        case 0x83: // ADD/unused/ADC/SBB/unused/SUB/unused/CMP: sign-extended 8-bit immediate to 16-bit register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
            {
                LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate_byte);
                // Sign-extend to word
                uint16_t immediate = signExtendByteToWord(immediate_byte);
                switch (regBits)
                {
                case 0b000:
                    return ins$ADDimmediateToRegister(rmBits, immediate);
                case 0b010:
                    //return ins$ADCimmediateToRegisterWord(rmBits, immediate);
                case 0b011:
                    //return ins$SBBimmediateToRegisterWord(rmBits, immediate);
                case 0b101:
                    //return ins$SUBimmediateToRegisterWord(rmBits, immediate);
                case 0b111:
                    //return ins$CMPimmediateToRegisterWord(rmBits, immediate);
                default:
                    ILLEGAL_INSTRUCTION();
                    return;
                }
            }

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte);
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate_byte);
            // Sign-extend to word
            uint16_t immediate = signExtendByteToWord(immediate_byte);
            switch (regBits)
            {
            case 0b000:
                return ins$ADDimmediateToMemory(memoryManager, effectiveAddress, immediate);
            case 0b010:
                //return ins$ADCimmediateToMemory(memoryManager, effectiveAddress, immediate);
            case 0b011:
                //return ins$SBBimmediateToMemory(memoryManager, effectiveAddress, immediate);
            case 0b101:
                //return ins$SUBimmediateToMemory(memoryManager, effectiveAddress, immediate);
            case 0b111:
                //return ins$CMPimmediateToMemory(memoryManager, effectiveAddress, immediate);
            default:
                ILLEGAL_INSTRUCTION();
                return;
            }
            TODO();
            return;
        }
        case 0x84: // TEST: 8-bit from register to register/memory
        {
            TODO();
            return;
        }
        case 0x85: // TEST: 16-bit from register to register/memory
        {
            TODO();
            return;
        }
        case 0x86: // XCHG: 8-bit exchange from register/memory to register
        {
            TODO();
            return;
        }
        case 0x87: // XCHG: 16-bit exchange from register/memory to register
        {
            TODO();
            return;
        }
        case 0x88: // MOV: 8-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$MOVregisterToRegisterByte(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte);

            return ins$MOVregisterToMemory(memoryManager, effectiveAddress, getRegisterValueFromREG8(regBits));
        }
        case 0x89: // MOV: 16-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$MOVregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte);

            return ins$MOVregisterToMemory(memoryManager, effectiveAddress, getRegisterFromREG16(regBits));
        }
        case 0x8A: // MOV: 8-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x8B: // MOV: 16-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x8C: // MOV/unused: 16-bit from segment register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, srBits, rmBits);

            // Are we doing a MOV (bit 2 is 0)?
            if (srBits & BIT(2))
            {
                ILLEGAL_INSTRUCTION();
                return;
            }

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$MOVsegmentRegisterToRegisterWord(rmBits, srBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte);

            return ins$MOVsegmentRegisterToMemoryWord(memoryManager, effectiveAddress, srBits);
        }
        case 0x8D: // LEA: Load effective address
        {
            TODO();
            return;
        }
        case 0x8E: // MOV/unused: 16-bit from register/memory to segment register
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, srBits, rmBits);

            // Are we doing a MOV (bit 2 is 0)?
            if (srBits & BIT(2))
            {
                ILLEGAL_INSTRUCTION();
                return;
            }

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$MOVregisterToSegmentRegisterWord(srBits, getRegisterFromREG16(rmBits));

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte);

            return ins$MOVmemoryToSegmentRegisterWord(memoryManager, srBits, effectiveAddress);
        }
        case 0x8F: // POP/unused/unused/unused/unused/unused/unused/unused: Pop 16-bit register/memory from stack
        {
            TODO();
            return;
        }
        case 0x90: // NOP
        {
            TODO();
            return;
        }
        case 0x91: // XCHG: Exchange AX and CX
        {
            TODO();
            return;
        }
        case 0x92: // XCHG: Exchange AX and DX
        {
            TODO();
            return;
        }
        case 0x93: // XCHG: Exchange AX and BX
        {
            TODO();
            return;
        }
        case 0x94: // XCHG: Exchange AX and SP
        {
            TODO();
            return;
        }
        case 0x95: // XCHG: Exchange AX and BP
        {
            TODO();
            return;
        }
        case 0x96: // XCHG: Exchange AX and SI
        {
            TODO();
            return;
        }
        case 0x97: // XCHG: Exchange AX and DI
        {
            TODO();
            return;
        }
        case 0x98: // CBW: Convert byte to word
        {
            TODO();
            return;
        }
        case 0x99: // CWD: Convert word to doubleword
        {
            TODO();
            return;
        }
        case 0x9A: // CALL: FAR_PROC
        {
            TODO();
            return;
        }
        case 0x9B: // WAIT: Wait
        {
            return ins$WAIT();
        }
        case 0x9C: // PUSHF: Push flags to stack
        {
            TODO();
            return;
        }
        case 0x9D: // POPF: Pop flags from stack
        {
            TODO();
            return;
        }
        case 0x9E: // SAHF: Store AH into flags
        {
            TODO();
            return;
        }
        case 0x9F: // LAHF: Load AH from flags
        {
            TODO();
            return;
        }
        case 0xA0: // MOV: 8-bit from memory to AL
        {
            TODO();
            return;
        }
        case 0xA1: // MOV: 16-bit from memory to AX
        {
            TODO();
            return;
        }
        case 0xA2: // MOV: 8-bit from AL to memory
        {
            TODO();
            return;
        }
        case 0xA3: // MOV: 16-bit from AX to memory
        {
            TODO();
            return;
        }
        case 0xA4: // MOVS: 8-bit move string from SRC-STR8 to DEST-STR8
        {
            TODO();
            return;
        }
        case 0xA5: // MOVS: 16-bit move string from SRC-STR16 to DEST-STR16
        {
            TODO();
            return;
        }
        case 0xA6: // CMPS: 8-bit compare string from SRC-STR8 to DEST-STR8
        {
            TODO();
            return;
        }
        case 0xA7: // 16-bit compare string from SRC-STR16 to DEST-STR16
        {
            TODO();
            return;
        }
        case 0xA8: // TEST: 8-bit from immediate to AL
        {
            TODO();
            return;
        }
        case 0xA9: // TEST: 16-bit from immediate to AX
        {
            TODO();
            return;
        }
        case 0xAA: // STOS: 8-bit store byte or word string to DEST-STR8
        {
            TODO();
            return;
        }
        case 0xAB: // STOS: 16-bit store byte or word string to DEST-STR8
        {
            TODO();
            return;
        }
        case 0xAC: // LODS: 8-bit load string to SRC-STR8
        {
            TODO();
            return;
        }
        case 0xAD: // LODS: 16-bit load string to SRC_STR16
        {
            TODO();
            return;
        }
        case 0xAE: // SCAS: 8-bit scan string to DEST-STR8
        {
            TODO();
            return;
        }
        case 0xAF: // SCAS: 16-bit scan string to DEST-STR16
        {
            TODO();
            return;
        }
        case 0xB0: // MOV: 8-bit from immediate to AL
        {
            DC_CORE_WARN("$MOV: 8-bit immediate to AL");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            AL(immediate);

            return;
        }
        case 0xB1: // MOV: 8-bit from immediate to CL
        {
            DC_CORE_WARN("$MOV: 8-bit immediate to CL");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            CL(immediate);

            return;
        }
        case 0xB2: // MOV: 8-bit from immediate to DL
        {
            DC_CORE_WARN("$MOV: 8-bit immediate to DL");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            DL(immediate);

            return;
        }
        case 0xB3: // MOV: 8-bit from immediate to BL
        {
            DC_CORE_WARN("$MOV: 8-bit immediate to BL");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            BL(immediate);

            return;
        }
        case 0xB4: // MOV: 8-bit from immediate to AH
        {
            DC_CORE_WARN("$MOV: 8-bit immediate to AH");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            AH(immediate);

            return;
        }
        case 0xB5: // MOV: 8-bit from immediate to CH
        {
            DC_CORE_WARN("$MOV: 8-bit immediate to CH");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            CH(immediate);

            return;
        }
        case 0xB6: // MOV: 8-bit from immediate to DH
        {
            DC_CORE_WARN("$MOV: 8-bit immediate to DH");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            DH(immediate);

            return;
        }
        case 0xB7: // MOV: 8-bit from immediate to BH
        {
            DC_CORE_WARN("$MOV: 8-bit immediate to BH");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            BH(immediate);

            return;
        }
        case 0xB8: // MOV: 16-bit from immediate to AX
        {
            DC_CORE_WARN("$MOV: 16-bit immediate to AX");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            AX() = immediate;

            return;
        }
        case 0xB9: // MOV: 16-bit from immediate to CX
        {
            DC_CORE_WARN("$MOV: 16-bit immediate to CX");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            CX() = immediate;

            return;
        }
        case 0xBA: // MOV: 16-bit from immediate to DX
        {
            DC_CORE_WARN("$MOV: 16-bit immediate to DX");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            DX() = immediate;

            return;
        }
        case 0xBB: // MOV: 16-bit from immediate to BX
        {
            DC_CORE_WARN("$MOV: 16-bit immediate to BX");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            BX() = immediate;

            return;
        }
        case 0xBC: // MOV: 16-bit from immediate to SP
        {
            DC_CORE_WARN("$MOV: 16-bit immediate to SP");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            SP() = immediate;

            return;
        }
        case 0xBD: // MOV: 16-bit from immediate to BP
        {
            DC_CORE_WARN("$MOV: 16-bit immediate to BP");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            BP() = immediate;

            return;
        }
        case 0xBE: // MOV: 16-bit from immediate to SI
        {
            DC_CORE_WARN("$MOV: 16-bit immediate to SI");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            SI() = immediate;

            return;
        }
        case 0xBF: // MOV: 16-bit from immediate to DI
        {
            DC_CORE_WARN("$MOV: 16-bit immediate to DI");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            DI() = immediate;

            return;
        }
        case 0xC2: // RET: Return within segment adding immediate to SP
        {
            TODO();
            return;
        }
        case 0xC3: // RET: Return within segment
        {
            TODO();
            return;
        }
        case 0xC4: // LES: Load pointer using ES
        {
            TODO();
            return;
        }
        case 0xC5: // LDS: Load pointer using DS
        {
            TODO();
            return;
        }
        case 0xC6: // MOV/unused/unused/unused/unused/unused/unused/unused: 8-bit from immediate to memory
        {
            TODO();
            return;
        }
        case 0xC7: // MOV/unused/unused/unused/unused/unused/unused/unused/unused/unused: 16-bit from immediate to memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, mustBeZeroBits, rmBits);

            // Instruction is only defined when these 3 bits are 0
            if (mustBeZeroBits != 0)
            {
                ILLEGAL_INSTRUCTION();
                return;
            }

            // We can go through this as no displacements will be loaded if we're in memory mode with no displacement
            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, 1, displacementLowByte, displacementHighByte);

            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);

            return ins$MOVimmediateToMemory(memoryManager, effectiveAddress, immediate);
        }
        case 0xCA: // RET: Return intersegment adding immediate to SP
        {
            TODO();
            return;
        }
        case 0xCB: // RET: Return intersegment
        {
            TODO();
            return;
        }
        case 0xCC: // INT: Interrupt 3
        {
            TODO();
            return;
        }
        case 0xCD: // INT: Interrupt based on 8-bit immediate
        {
            TODO();
            return;
        }
        case 0xCE: // INTO: Interrupt if overflow
        {
            TODO();
            return;
        }
        case 0xCF: // IRET: Interrupt return
        {
            TODO();
            return;
        }
        case 0xD0: // ROL/ROR/RCL/RCR/(SAL/SHL)/SHR/unused/SAR: 8-bit shift-like register/memory by 1
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);
            if (IS_IN_REGISTER_MODE(modBits))
            {
                switch (regBits)
                {
                case 0b000:
                    return ins$ROLregisterOnceByte(rmBits);
                case 0b001:
                    return ins$RORregisterOnceByte(rmBits);
                case 0b010:
                    return ins$RCLregisterOnceByte(rmBits);
                case 0b011:
                    return ins$RCRregisterOnceByte(rmBits);
                case 0b100:
                    return ins$SALregisterOnceByte(rmBits);
                case 0b101:
                    return ins$SHRregisterOnceByte(rmBits);
                case 0b111:
                    return ins$SARregisterOnceByte(rmBits);
                default:
                    ILLEGAL_INSTRUCTION();
                    return;
                }
            }
            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte);

            switch (regBits)
            {
            case 0b000:
                return ins$ROLmemoryOnceByte(memoryManager, effectiveAddress);
            case 0b001:
                return ins$RORmemoryOnceByte(memoryManager, effectiveAddress);
            case 0b010:
                return ins$RCLmemoryOnceByte(memoryManager, effectiveAddress);
            case 0b011:
                return ins$RCRmemoryOnceByte(memoryManager, effectiveAddress);
            case 0b100:
                return ins$SALmemoryOnceByte(memoryManager, effectiveAddress);
            case 0b101:
                return ins$SHRmemoryOnceByte(memoryManager, effectiveAddress);
            case 0b111:
                return ins$SARmemoryOnceByte(memoryManager, effectiveAddress);
            default:
                ILLEGAL_INSTRUCTION();
                return;
            }
        }
        case 0xD1: // ROL/ROR/RCL/RCR/(SAL/SHL)/SHR/unused/SAR: 16-bit shift-like register/memory by 1
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);
            if (IS_IN_REGISTER_MODE(modBits))
            {
                switch (regBits)
                {
                case 0b000:
                    return ins$ROLregisterOnceWord(rmBits);
                case 0b001:
                    return ins$RORregisterOnceWord(rmBits);
                case 0b010:
                    return ins$RCLregisterOnceWord(rmBits);
                case 0b011:
                    return ins$RCRregisterOnceWord(rmBits);
                case 0b100:
                    return ins$SALregisterOnceWord(rmBits);
                case 0b101:
                    return ins$SHRregisterOnceWord(rmBits);
                case 0b111:
                    return ins$SARregisterOnceWord(rmBits);
                default:
                    ILLEGAL_INSTRUCTION();
                    return;
                }
            }
            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte);

            switch (regBits)
            {
            case 0b000:
                return ins$ROLmemoryOnceWord(memoryManager, effectiveAddress);
            case 0b001:
                return ins$RORmemoryOnceWord(memoryManager, effectiveAddress);
            case 0b010:
                return ins$RCLmemoryOnceWord(memoryManager, effectiveAddress);
            case 0b011:
                return ins$RCRmemoryOnceWord(memoryManager, effectiveAddress);
            case 0b100:
                return ins$SALmemoryOnceWord(memoryManager, effectiveAddress);
            case 0b101:
                return ins$SHRmemoryOnceWord(memoryManager, effectiveAddress);
            case 0b111:
                return ins$SARmemoryOnceWord(memoryManager, effectiveAddress);
            default:
                ILLEGAL_INSTRUCTION();
                return;
            }
        }
        case 0xD2: // ROL/ROR/RCL/RCR/(SAL/SHL)/SHR/unused/SAR: 8-bit shift-like register/memory by CL
        {
            TODO();
            return;
        }
        case 0xD3: // ROL/ROR/RCL/RCR/(SAL/SHL)/SHR/unused/SAR: 16-bit shift-like register-memory by CL
        {
            TODO();
            return;
        }
        case 0xD4: // AAM: ASCII adjust for multiply
        {
            TODO();
            return;
        }
        case 0xD5: // AAD: ASCII adjust for division
        {
            TODO();
            return;
        }
        case 0xD7: // XLAT: Translate SOURCE-TABLE
        {
            TODO();
            return;
        }
        case 0xD8: // ESC: Escape to external device
        case 0xD9: // and variants
        case 0xDA:
        case 0xDB:
        case 0xDC:
        case 0xDD:
        case 0xDE:
        case 0xDF:
        {
            TODO();
            return;
        }
        case 0xE0: // LOOPNE/LOOPNZ: Loop if not equal / Loop if not zero
        {
            TODO();
            return;
        }
        case 0xE1: // LOOPE/LOOPZ: Loop if equal / Loop if zero
        {
            TODO();
            return;
        }
        case 0xE2: // LOOP: Loop
        {
            TODO();
            return;
        }
        case 0xE3: // JCXZ: Jump if CX is zero
        {
            TODO();
            return;
        }
        case 0xE4: // IN: 8-bit immediate and AL
        {
            TODO();
            return;
        }
        case 0xE5: // IN: 8-bit immediate and AX ??
        {
            TODO();
            return;
        }
        case 0xE6: // OUT: 8-bit immediate and AL
        {
            //TODO();
            DC_CORE_ERROR("ins$OUT is unimplemented so we'll skip over :(");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, data);

            return;
        }
        case 0xE7: // OUT: 8-bit immediate and AX ??
        {
            TODO();
            return;
        }
        case 0xE8: // CALL: Call NEAR-PROC
        {
            TODO();
            return;
        }
        case 0xE9: // JMP: Jump to NEAR-LABEL
        {
            TODO();
            return;
        }
        case 0xEA: // JMP: Jump to FAR-LABEL
        {
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, instructionPointer);
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, codeSegment);

            return ins$JMPinterSegment(codeSegment, instructionPointer);
        }
        case 0xEB: // JMP: Jump to SHORT-LABEL
        {
            TODO();
            return;
        }
        case 0xEC: // IN: AL and AX
        {
            TODO();
            return;
        }
        case 0xED: // UN: AX and DX
        {
            TODO();
            return;
        }
        case 0xEE: // OUT: AL and DX
        {
            TODO();
            return;
        }
        case 0xEF: // OUT: AX and DX
        {
            TODO();
            return;
        }
        case 0xF0: // LOCK: Lock bus
        {
            return ins$LOCK();
        }
        case 0xF2: // REPNE/REPNZ: Repeat string operation while not equal/not zero
        {
            TODO();
            return;
        }
        case 0xF3: // REP/REPE/REPZ: Repeat string operation/ Repeat string operation while equal / while zero
        {
            TODO();
            return;
        }
        case 0xF4: // HLT: Halt the processor
        {
            return ins$HLT();
        }
        case 0xF5: // CMC: Complement carry flag
        {
            return ins$CMC();
        }
        case 0xF6: // TEST/unused/NOT/NEG/MUL/IMUL/DIV/IDIV: (8-bit from immediate to register/memory)/(8-bit register/memory)
        {
            TODO();
            return;
        }
        case 0xF7: // TEST/unused/NOT/NEG/MUL/IMUL/DIV/IDIV: (16-bit from immediate to register/memory)/(16-bit register/memory)
        {
            TODO();
            return;
        }
        case 0xF8: // CLC: Clear carry bit
        {
            return ins$CLC();
        }
        case 0xF9: // STC: Set carry bit
        {
            return ins$STC();
        }
        case 0xFA: // CLI: Clear interrupt flag
        {
            return ins$CLI();
        }
        case 0xFB: // STI: Set interrupt flag
        {
            return ins$STI();
        }
        case 0xFC: // CLD: Clear direction flag
        {
            return ins$CLD();
        }
        case 0xFD: // STD: Set direction flag
        {
            return ins$STD();
        }
        case 0xFE: // INC/DEC/unused/unused/unused/unused/unused/unused: 8-bit register/memory
        {
            TODO();
            return;
        }
        case 0xFF: // INC/DEC/CALL/CALL/JMP/JMP/PUSH/unused: 16-bit (memory)/(intrasegment register/memory)/(intrasegment memory)/(intrasegment register/memory)/(intersegment memory)/(memory)
        {
            TODO();
            return;
        }

        // Known unused:
        case 0x0F:
        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
        case 0x67:
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
        case 0x6D:
        case 0x6E:
        case 0x6F:
        case 0xC0:
        case 0xC1:
        case 0xC8:
        case 0xC9:
        case 0xD6:
        case 0xF1:
        {
            DC_CORE_ERROR("Known unused instruction opcode hit :(");
            UNKNOWN_INSTRUCTION();
            return;
        }

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
        DC_CORE_WARN("ins$HLT: Halting");
        TODO();
    }

    void Processor::ins$CLC()
    {
        DC_CORE_WARN("ins$CLC: Clear carry flag");
        CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
    }

    void Processor::ins$CMC()
    {
        TODO();
    }

    void Processor::ins$STC()
    {
        DC_CORE_WARN("ins$STC: Set carry flag");
        SET_FLAG_BIT(m_flags, CARRY_FLAG);
    }

    void Processor::ins$CLD()
    {
        DC_CORE_WARN("ins$CLD: Clear direction flag");
        CLEAR_FLAG_BIT(m_flags, DIRECTION_fLAG);
    }

    void Processor::ins$STD()
    {
        DC_CORE_WARN("ins$STD: Set direction flag");
        SET_FLAG_BIT(m_flags, DIRECTION_fLAG);

    }

    void Processor::ins$CLI()
    {
        DC_CORE_WARN("ins$CLI: Disable interrupts");
        CLEAR_FLAG_BIT(m_flags, INTERRUPT_ENABLE_FLAG);
    }

    void Processor::ins$STI()
    {
        DC_CORE_WARN("ins$STI: Enabling interrupts");
        SET_FLAG_BIT(m_flags, INTERRUPT_ENABLE_FLAG);
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

    void Processor::ins$ADDimmediateToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint8_t immediate)
    {
        DC_CORE_WARN("ins$ADD: 8-bit imediate to memory");
        uint8_t memoryValue = memoryManager.readByte(m_dataSegment, effectiveAddress);
        
        // Note: this may be UB :(
        uint8_t result = memoryValue + immediate;

        // Carry (unsigned overflow)
        if (immediate > UCHAR_MAX - memoryValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            // Fix UB
            result = (UCHAR_MAX - memoryValue) + immediate;
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (immediate > SCHAR_MAX - memoryValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        memoryManager.writeByte(m_dataSegment, effectiveAddress, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADDimmediateToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint16_t immediate)
    {
        DC_CORE_WARN("ins$ADD: 16-bit immediate to memory");
        uint16_t memoryValue = memoryManager.readWord(m_dataSegment, effectiveAddress);

        // Note: this may be UB :(
        uint16_t result = memoryValue + immediate;

        // Carry (unsigned overflow)
        if (immediate > USHRT_MAX - memoryValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            // Fix UB
            result = (USHRT_MAX - memoryValue) + immediate;
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (immediate > SHRT_MAX - memoryValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        memoryManager.writeWord(m_codeSegment, effectiveAddress, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADDimmediateToRegister(uint8_t destREG, uint8_t value)
    {
        DC_CORE_WARN("ins$ADD: 8-bit immediate to register");
        uint8_t registerValue = getRegisterValueFromREG8(destREG);

        // Note: this may be UB :(
        uint8_t result = registerValue + value;
        
        // Carry (unsigned overflow)
        if (value > UCHAR_MAX - registerValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            // Fix UB
            result = (UCHAR_MAX - registerValue) + value;
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (value > SCHAR_MAX - registerValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        updateRegisterFromREG8(destREG, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADDimmediateToRegister(uint8_t destREG, uint16_t value)
    {
        DC_CORE_WARN("ins$ADD: 16-bit immediate to register");
        uint16_t registerValue = getRegisterFromREG16(destREG);

        // Note: this may be UB :(
        uint16_t result = registerValue + value;

        // Carry (unsigned overflow)
        if (value > USHRT_MAX - registerValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            // Fix UB
            result = (USHRT_MAX - registerValue) + value;
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (value > SHRT_MAX - registerValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        updateRegisterFromREG16(destREG, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADDregisterToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint8_t registerValue)
    {
        DC_CORE_WARN("ins$ADD: 8-bit register to memory");
        uint8_t memoryValue = memoryManager.readByte(m_dataSegment, effectiveAddress);

        // Note: this may be UB :(
        uint8_t result = memoryValue + registerValue;

        // Carry (unsigned overflow)
        if (registerValue > UCHAR_MAX - memoryValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            // Fix UB
            result = (UCHAR_MAX - memoryValue) + registerValue;
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (registerValue > SCHAR_MAX - memoryValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        memoryManager.writeByte(m_dataSegment, effectiveAddress, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADDregisterToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint16_t registerValue)
    {
        DC_CORE_WARN("ins$ADD: 16-bit register to memory");
        uint16_t memoryValue = memoryManager.readWord(m_dataSegment, effectiveAddress);

        // Note: this may be UB :(
        uint16_t result = memoryValue + registerValue;

        // Carry (unsigned overflow)
        if (registerValue > USHRT_MAX - memoryValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            // Fix UB
            result = (USHRT_MAX - memoryValue) + registerValue;
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (registerValue > SHRT_MAX - memoryValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        memoryManager.writeWord(m_codeSegment, effectiveAddress, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADDregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG)
    {
        DC_CORE_WARN("ins$ADD: 8-bit register to register");
        uint8_t registerValue = getRegisterValueFromREG8(destREG);
        uint8_t sourceValue = getRegisterValueFromREG8(sourceREG);

        // Note: this may be UB :(
        uint8_t result = registerValue + sourceValue;

        // Carry (unsigned overflow)
        if (sourceValue > UCHAR_MAX - registerValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            // Fix UB
            result = (UCHAR_MAX - registerValue) + sourceValue;
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (sourceValue > SCHAR_MAX - registerValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        updateRegisterFromREG8(destREG, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADDregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG)
    {
        DC_CORE_WARN("ins$ADD: 16-bit register to register");
        uint16_t registerValue = getRegisterFromREG16(destREG);
        uint16_t sourceValue = getRegisterFromREG16(sourceREG);

        // Note: this may be UB :(
        uint16_t result = registerValue + sourceValue;

        // Carry (unsigned overflow)
        if (sourceValue > USHRT_MAX - registerValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            // Fix UB
            result = (USHRT_MAX - registerValue) + sourceValue;
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (sourceValue > SHRT_MAX - registerValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        updateRegisterFromREG16(destREG, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$INC(uint8_t isWordBit, uint8_t REG)
    {
        if (isWordBit)
        {
            DC_CORE_WARN("ins$INC: INC {0}", getRegisterNameFromREG16(REG));
            uint16_t currentValue = getRegisterFromREG16(REG);
            uint16_t newValue = currentValue + 1;
            updateRegisterFromREG16(REG, newValue);

            // We shouldn't touch the CARRY_FLAG
            if (currentValue >= SHRT_MAX)
                SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
            else
                CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

            if (IS_BIT_SET(currentValue, 15))
                SET_FLAG_BIT(m_flags, SIGN_FLAG);
            else
                CLEAR_FLAG_BIT(m_flags, SIGN_FLAG);

            if (currentValue == 0)
                SET_FLAG_BIT(m_flags, ZERO_FLAG);
            else
                CLEAR_FLAG_BIT(m_flags, ZERO_FLAG);

            DO_PARITY_BYTE(currentValue);
            if (IS_PARITY_EVEN(currentValue))
                SET_FLAG_BIT(m_flags, PARITY_FLAG);
            else
                CLEAR_FLAG_BIT(m_flags, PARITY_FLAG);
        }
        else
        {
            DC_CORE_WARN("ins$INC: INC {0}", getRegisterNameFromREG8(REG));
            uint8_t currentValue = getRegisterValueFromREG8(REG);
            uint8_t newValue = currentValue + 1;
            updateRegisterFromREG8(REG, newValue);

            // We shouldn't touch the CARRY_FLAG
            if (currentValue >= SCHAR_MAX)
                SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
            else
                CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

            if (IS_BIT_SET(currentValue, 7))
                SET_FLAG_BIT(m_flags, SIGN_FLAG);
            else
                CLEAR_FLAG_BIT(m_flags, SIGN_FLAG);

            if (currentValue == 0)
                SET_FLAG_BIT(m_flags, ZERO_FLAG);
            else
                CLEAR_FLAG_BIT(m_flags, ZERO_FLAG);

            DO_PARITY_BYTE(currentValue);
            if (IS_PARITY_EVEN(currentValue))
                SET_FLAG_BIT(m_flags, PARITY_FLAG);
            else
                CLEAR_FLAG_BIT(m_flags, PARITY_FLAG);
        }
    }

    void Processor::ins$JMPinterSegment(uint16_t newCodeSegment, uint16_t newInstructionPointer)
    {
        DC_CORE_WARN("ins$JMP: Jumping to {0:x}:{1:x}", newCodeSegment, newInstructionPointer);

        // Debug: Print the BIOS ROM address
        if (newCodeSegment == 0xF000)
            DC_CORE_TRACE(".. which is at BIOS 0x{0:X} in HEX EDITOR or 0x{1:X} in the actual ROM", MemoryManager::addresstoPhysical(newCodeSegment, newInstructionPointer) - 0xF8000, MemoryManager::addresstoPhysical(newCodeSegment, newInstructionPointer) - 0xF0000);

        m_codeSegment = newCodeSegment;
        m_instructionPointer = newInstructionPointer;
    }

    void Processor::ins$JMPshort(int8_t increment)
    {
        DC_CORE_WARN("ins$JMP: Jumping to short");
        m_instructionPointer += increment;
    }

    void Processor::ins$MOVimmediateToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint16_t immediate)
    {
        DC_CORE_WARN("ins$MOV: 16-bit immediate to memory");
        memoryManager.writeWord(m_dataSegment, effectiveAddress, immediate);
    }

    void Processor::ins$MOVimmediateToRegisterByte(uint8_t reg, uint8_t immediate)
    {
        DC_CORE_WARN("ins$MOV: 8-bit immediate to register");
        updateRegisterFromREG8(reg, immediate);
    }

    void Processor::ins$MOVimmediateToRegisterWord(uint8_t reg, uint16_t immediate)
    {
        DC_CORE_WARN("ins$MOV: 16-bit immediate to register");
        updateRegisterFromREG16(reg, immediate);
    }

    void Processor::ins$MOVmemoryToSegmentRegisterWord(MemoryManager& memoryManager, uint8_t srBits, uint16_t effectiveAddress)
    {
        DC_CORE_WARN("ins$MOV: 16-bit memory to segment register");
        uint16_t value = memoryManager.readWord(m_codeSegment, effectiveAddress);

        switch (srBits)
        {
        case 0b00:
            m_extraSegment = value;
            return;

        case 0b01:
            m_codeSegment = value;
            return;

        case 0b10:
            m_stackSegment = value;
            return;

        case 0b11:
            m_dataSegment = value;
            return;

        default:
            DC_CORE_ERROR("Malformed segment register bits : 0b{0:b}", srBits);
            VERIFY_NOT_REACHED();
            return;
        }
    }

    void Processor::ins$MOVregisterToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint8_t registerValue)
    {
        DC_CORE_WARN("ins$MOV: 8-bit register to memory");
        memoryManager.writeByte(m_dataSegment, effectiveAddress, registerValue);
    }

    void Processor::ins$MOVregisterToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint16_t registerValue)
    {
        DC_CORE_WARN("ins$MOV: 16-bit register to memory");
        memoryManager.writeWord(m_dataSegment, effectiveAddress, registerValue);
    }

    void Processor::ins$MOVregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG)
    {
        DC_CORE_WARN("ins$MOV: 8-bit register to register");
        uint8_t sourceValue = getRegisterValueFromREG8(sourceREG);
        updateRegisterFromREG8(destREG, sourceValue);
    }

    void Processor::ins$MOVregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG)
    {
        DC_CORE_WARN("ins$MOV: 16-bit register to register");
        uint16_t sourceValue = getRegisterFromREG16(sourceREG);
        updateRegisterFromREG16(destREG, sourceValue);
    }

    void Processor::ins$MOVregisterToSegmentRegisterWord(uint8_t srBits, uint16_t value)
    {
        DC_CORE_WARN("ins$MOV: 16-bit register to segment register");
        switch (srBits)
        {
        case 0b00:
            m_extraSegment = value;
            return;

        case 0b01:
            m_codeSegment = value;
            return;

        case 0b10:
            m_stackSegment = value;
            return;

        case 0b11:
            m_dataSegment = value;
            return;

        default:
            DC_CORE_ERROR("Malformed segment register bits : 0b{0:b}", srBits);
            VERIFY_NOT_REACHED();
            return;
        }
    }

    void Processor::ins$MOVsegmentRegisterToMemoryWord(MemoryManager& memoryManager, uint16_t effectiveAddress, uint8_t SEGREG)
    {
        DC_CORE_WARN("ins$MOV: 16-bit segment register to memory");
        uint16_t segRegValue;
        switch (SEGREG)
        {
        case 0b00:
            segRegValue = m_extraSegment;
            return;

        case 0b01:
            segRegValue = m_codeSegment;
            return;

        case 0b10:
            segRegValue = m_stackSegment;
            return;

        case 0b11:
            segRegValue = m_dataSegment ;
            return;

        default:
            DC_CORE_ERROR("Malformed segment register bits : 0b{0:b}", SEGREG);
            VERIFY_NOT_REACHED();
            return;
        }

        memoryManager.writeWord(m_dataSegment, effectiveAddress, segRegValue);
    }

    void Processor::ins$MOVsegmentRegisterToRegisterWord(uint8_t REG, uint8_t SEGREG)
    {
        DC_CORE_WARN("ins$MOV: 16-bit segment register to register");
        uint16_t segRegValue;
        switch (SEGREG)
        {
        case 0b00:
            segRegValue = m_extraSegment;
            return;

        case 0b01:
            segRegValue = m_codeSegment;
            return;

        case 0b10:
            segRegValue = m_stackSegment;
            return;

        case 0b11:
            segRegValue = m_dataSegment;
            return;

        default:
            DC_CORE_ERROR("Malformed segment register bits : 0b{0:b}", SEGREG);
            VERIFY_NOT_REACHED();
            return;
        }

        updateRegisterFromREG16(REG, segRegValue);
    }

    void Processor::ins$RCLmemoryOnceByte(MemoryManager& memoryManager, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$RCLmemoryOnceWord(MemoryManager & memoryManager, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$RCLregisterOnceByte(uint8_t REG)
    {
        TODO();
    }

    void Processor::ins$RCLregisterOnceWord(uint8_t REG)
    {
        TODO();
    }

    void Processor::ins$RCRmemoryOnceByte(MemoryManager& memoryManager, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$RCRmemoryOnceWord(MemoryManager & memoryManager, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$RCRregisterOnceByte(uint8_t REG)
    {
        TODO();
    }

    void Processor::ins$RCRregisterOnceWord(uint8_t REG)
    {
        TODO();
    }

    void Processor::ins$ROLmemoryOnceByte(MemoryManager& memoryManager, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$ROLmemoryOnceWord(MemoryManager & memoryManager, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$ROLregisterOnceByte(uint8_t REG)
    {
        TODO();
    }

    void Processor::ins$ROLregisterOnceWord(uint8_t REG)
    {
        TODO();
    }

    void Processor::ins$RORmemoryOnceByte(MemoryManager& memoryManager, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$RORmemoryOnceWord(MemoryManager & memoryManager, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$RORregisterOnceByte(uint8_t REG)
    {
        TODO();
    }

    void Processor::ins$RORregisterOnceWord(uint8_t REG)
    {
        TODO();
    }

    void Processor::ins$SALmemoryOnceByte(MemoryManager& memoryManager, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$SALmemoryOnceWord(MemoryManager & memoryManager, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$SALregisterOnceByte(uint8_t rmBits)
    {
        DC_CORE_WARN("ins$SAL: {0},1", getRegisterNameFromREG8(rmBits));
        uint8_t registerValue = getRegisterValueFromREG8(rmBits);
        uint8_t bitZeroBefore = IS_BIT_SET(registerValue, 7);
        bool setCarry;
        if (registerValue > SCHAR_MAX)
            setCarry = true;
        else
            setCarry = false;

        registerValue <<= 1;
        setFlagsAfterLogicalOperation(registerValue);
        // Set carry flag
        if (setCarry)
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        // Set overflow flag
        if (bitZeroBefore == IS_BIT_SET(registerValue, 7))
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        updateRegisterFromREG8(rmBits, registerValue);
    }

    void Processor::ins$SALregisterOnceWord(uint8_t rmBits)
    {
        DC_CORE_WARN("ins$SAL: {0},1", getRegisterNameFromREG16(rmBits));
        uint16_t registerValue = getRegisterFromREG16(rmBits);
        uint8_t bitZeroBefore = IS_BIT_SET(registerValue, 15);
        bool setCarry;
        if (registerValue > SHRT_MAX)
            setCarry = true;
        else
            setCarry = false;

        registerValue <<= 1;
        setFlagsAfterLogicalOperation(registerValue);
        // Set carry flag
        if (setCarry)
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        // Set overflow flag
        if (bitZeroBefore == IS_BIT_SET(registerValue, 15))
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        updateRegisterFromREG16(rmBits, registerValue);
    }

    void Processor::ins$SARmemoryOnceByte(MemoryManager& memoryManager, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$SARmemoryOnceWord(MemoryManager & memoryManager, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$SARregisterOnceByte(uint8_t rmBits)
    {
        TODO();
    }

    void Processor::ins$SARregisterOnceWord(uint8_t rmBits)
    {
        TODO();
    }

    void Processor::ins$SHRmemoryOnceByte(MemoryManager& memoryManager, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$SHRmemoryOnceWord(MemoryManager & memoryManager, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$SHRregisterOnceByte(uint8_t rmBits)
    {
        TODO();
    }

    void Processor::ins$SHRregisterOnceWord(uint8_t rmBits)
    {
        TODO();
    }

    void Processor::ins$SUBimmediateToRegister(uint8_t destREG, uint8_t value)
    {
        DC_CORE_WARN("ins$SUB: 8-bit immediate to register");
        uint8_t registerValue = getRegisterValueFromREG8(destREG);

        // Note: this may be UB :(
        uint8_t result = registerValue - value;

        // Carry (unsigned overflow)
        if (value > registerValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (value > SCHAR_MAX - registerValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        updateRegisterFromREG8(destREG, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$SUBimmediateToRegister(uint8_t destREG, uint16_t value)
    {
        DC_CORE_WARN("ins$SUB: 16-bit immediate to register");
        uint16_t registerValue = getRegisterFromREG16(destREG);

        // Note: this may be UB :(
        uint16_t result = registerValue - value;

        // Carry (unsigned overflow)
        if (value > registerValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (value > SHRT_MAX - registerValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        updateRegisterFromREG16(destREG, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$SUBregisterToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint8_t registerValue)
    {
        DC_CORE_WARN("ins$SUB: 8-bit register to memory");
        uint8_t memoryValue = memoryManager.readByte(m_dataSegment, effectiveAddress);

        // Note: this may be UB :(
        uint8_t result = memoryValue - registerValue;

        // Carry (unsigned overflow)
        if (registerValue > memoryValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (registerValue > SCHAR_MAX - memoryValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        memoryManager.writeByte(m_dataSegment, effectiveAddress, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$SUBregisterToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint16_t registerValue)
    {
        DC_CORE_WARN("ins$SUB: 16-bit register to memory");
        uint16_t memoryValue = memoryManager.readWord(m_dataSegment, effectiveAddress);

        // Note: this may be UB :(
        uint16_t result = memoryValue - registerValue;

        // Carry (unsigned overflow)
        if (registerValue > memoryValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (registerValue > SHRT_MAX - memoryValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        memoryManager.writeWord(m_codeSegment, effectiveAddress, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$SUBregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG)
    {
        DC_CORE_WARN("ins$SUB: 8-bit register to register");
        uint8_t registerValue = getRegisterValueFromREG8(destREG);
        uint8_t sourceValue = getRegisterValueFromREG8(sourceREG);

        // Note: this may be UB :(
        uint8_t result = registerValue - sourceValue;

        // Carry (unsigned overflow)
        if (sourceValue > registerValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (sourceValue > SCHAR_MAX - registerValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        updateRegisterFromREG8(destREG, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$SUBregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG)
    {
        DC_CORE_WARN("ins$SUB: 16-bit register to register");
        uint16_t registerValue = getRegisterFromREG16(destREG);
        uint16_t sourceValue = getRegisterFromREG16(sourceREG);

        // Note: this may be UB :(
        uint16_t result = registerValue - sourceValue;

        // Carry (unsigned overflow)
        if (sourceValue > registerValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (sourceValue > SHRT_MAX - registerValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        updateRegisterFromREG16(destREG, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$XORregisterToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint8_t registerValue)
    {
        DC_CORE_WARN("ins$XOR: XOR-ing 8-bit register to memory");
        auto original = memoryManager.readByte(m_dataSegment, effectiveAddress);
        uint8_t result = original ^ registerValue;
        memoryManager.writeByte(m_dataSegment, effectiveAddress, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$XORregisterToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint16_t registerValue)
    {
        DC_CORE_WARN("ins$XOR: XOR-ing 16-bit register to memory");
        auto original = memoryManager.readWord(m_dataSegment, effectiveAddress);
        uint16_t result = original ^ registerValue;
        memoryManager.writeWord(m_dataSegment, effectiveAddress, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$XORregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG)
    {
        DC_CORE_WARN("ins$XOR: 8-bit register to register");
        auto operand = getRegisterValueFromREG8(sourceREG);
        auto operand2 = getRegisterValueFromREG8(destREG);
        uint8_t result = operand ^ operand2;
        updateRegisterFromREG8(destREG, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$XORregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG)
    {
        DC_CORE_WARN("ins$XOR: 16-bit register to register");
        auto operand = getRegisterFromREG16(sourceREG);
        auto operand2 = getRegisterFromREG16(destREG);
        uint8_t result = operand ^ operand2;
        updateRegisterFromREG16(destREG, result);
        setFlagsAfterLogicalOperation(result);
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
            VERIFY_NOT_REACHED();
            return;
        }
    }

    void Processor::updateRegisterFromREG16(uint8_t REG, uint16_t data)
    {
        switch (REG)
        {
        case 0x0:
            AX() = data;
            return;

        case 0x1:
            CX() = data;
            return;

        case 0x2:
            DX() = data;
            return;

        case 0x3:
            BX() = data;
            return;

        case 0x4:
            SP() = data;
            return;

        case 0x5:
            BP() = data;
            return;

        case 0x6:
            SI() = data;
            return;

        case 0x7:
            DI() = data;
            return;

        default:
            DC_CORE_ERROR("Malformed REG bits : 0b{0:b}", REG);
            VERIFY_NOT_REACHED();
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
            VERIFY_NOT_REACHED();
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
            VERIFY_NOT_REACHED();
            return m_AX;
        }
    }

    uint16_t Processor::getEffectiveAddressFromBits(uint8_t rmBits, uint8_t modBits, uint8_t isWord, uint8_t displacementLow, uint8_t displacementHigh)
    {
        switch (modBits)
        {
        //case 0b11:
        //    switch (rmBits)
        //    {
        //    case 0b000:
        //        return m_BX + m_sourceIndex;

        //    case 0b001:
        //        return m_BX + m_destinationIndex;

        //    case 0b010:
        //        return m_basePointer + m_sourceIndex;

        //    case 0b011:
        //        return m_basePointer + m_destinationIndex;

        //    case 0b100:
        //        return m_sourceIndex;

        //    case 0b101:
        //        return m_destinationIndex;

        //    case 0b110:
        //    {
        //        DC_CORE_TRACE("Getting EA from DIRECT ADDRESS");

        //        // TODO: Make sure these do the correct job
        //        uint16_t directAddress;
        //        SET8BITREGISTERLOW(directAddress, displacementLow);
        //        SET8BITREGISTERHIGH(directAddress, displacementHigh);
        //        return directAddress;
        //    }
        //    case 0b111:
        //        return m_BX;

        //    default:
        //        VERIFY_NOT_REACHED();
        //        return 0;
        //    }

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

                // We use the "displacement" directly as it acts like a direct address at this point
            case 0b110:
            {
                uint16_t address = (uint16_t)displacementHigh << 8;
                address |= displacementLow;

                return address;
            }

            case 0b111:
                return m_BX;

            default:
                VERIFY_NOT_REACHED();
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
                VERIFY_NOT_REACHED();
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
                VERIFY_NOT_REACHED();
                return 0;
            }
        }
        default:
            VERIFY_NOT_REACHED();
            return 0;
        }
    }

    void Processor::loadDisplacementsFromInstructionStream(MemoryManager & memoryManager, uint8_t modBits, uint8_t rmBits, uint8_t & displacementLowByte, uint8_t & displacementHighByte)
    {
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
        else
        {
            // Perhaps we do have a 16-bit "displacement" (target address actually) after all
            if (rmBits == 0b110)
            {
                displacementLowByte = memoryManager.readByte(m_codeSegment, m_instructionPointer);
                m_instructionPointer++;
                displacementHighByte = memoryManager.readByte(m_codeSegment, m_instructionPointer);
                m_instructionPointer++;
            }
            else
            {
                DC_CORE_WARN("Useless loadDisplacementsFromInstructionStream call - modBits = 0b{0:b}", modBits);
            }
        }
    }

    void Processor::setFlagsAfterLogicalOperation(uint8_t byte)
    {
        CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);

        if (IS_BIT_SET(byte, 7))
            SET_FLAG_BIT(m_flags, SIGN_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, SIGN_FLAG);

        if (byte == 0)
            SET_FLAG_BIT(m_flags, ZERO_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, ZERO_FLAG);

        DO_PARITY_BYTE(byte);
        if (IS_PARITY_EVEN(byte))
            SET_FLAG_BIT(m_flags, PARITY_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, PARITY_FLAG);
    }

    void Processor::setFlagsAfterLogicalOperation(uint16_t word)
    {
        CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);

        if (IS_BIT_SET(word, 15))
            SET_FLAG_BIT(m_flags, SIGN_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, SIGN_FLAG);

        if (word == 0)
            SET_FLAG_BIT(m_flags, ZERO_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, ZERO_FLAG);

        DO_PARITY_WORD(word);
        if (IS_PARITY_EVEN(word))
            SET_FLAG_BIT(m_flags, PARITY_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, PARITY_FLAG);
    }

    void Processor::setFlagsAfterArithmeticOperation(uint8_t byte)
    {
        if (IS_BIT_SET(byte, 7))
            SET_FLAG_BIT(m_flags, SIGN_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, SIGN_FLAG);

        if (byte == 0)
            SET_FLAG_BIT(m_flags, ZERO_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, ZERO_FLAG);

        DO_PARITY_BYTE(byte);
        if (IS_PARITY_EVEN(byte))
            SET_FLAG_BIT(m_flags, PARITY_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, PARITY_FLAG);
    }

    void Processor::setFlagsAfterArithmeticOperation(uint16_t word)
    {
        if (IS_BIT_SET(word, 15))
            SET_FLAG_BIT(m_flags, SIGN_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, SIGN_FLAG);

        if (word == 0)
            SET_FLAG_BIT(m_flags, ZERO_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, ZERO_FLAG);

        DO_PARITY_WORD(word);
        if (IS_PARITY_EVEN(word))
            SET_FLAG_BIT(m_flags, PARITY_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, PARITY_FLAG);
    }
    const char* Processor::getRegisterNameFromREG8(uint8_t REG)
    {
        switch (REG)
        {
        case 0x0:
            return REGISTER_AL_NAME;

        case 0x1:
            return REGISTER_CL_NAME;

        case 0x2:
            return REGISTER_DL_NAME;

        case 0x3:
            return REGISTER_BL_NAME;

        case 0x4:
            return REGISTER_AH_NAME;

        case 0x5:
            return REGISTER_CH_NAME;

        case 0x6:
            return REGISTER_DH_NAME;

        case 0x7:
            return REGISTER_BH_NAME;

        default:
            DC_CORE_ERROR("Malformed REG bits : 0b{0:b}", REG);
            VERIFY_NOT_REACHED();
            return "ERROR";
        }
    }
    const char* Processor::getRegisterNameFromREG16(uint8_t REG)
    {
        switch (REG)
        {
        case 0x0:
            return REGISTER_AX_NAME;

        case 0x1:
            return REGISTER_CX_NAME;

        case 0x2:
            return REGISTER_DX_NAME;

        case 0x3:
            return REGISTER_BX_NAME;

        case 0x4:
            return REGISTER_SP_NAME;

        case 0x5:
            return REGISTER_BP_NAME;

        case 0x6:
            return REGISTER_SI_NAME;

        case 0x7:
            return REGISTER_DI_NAME;

        default:
            DC_CORE_ERROR("Malformed REG bits : 0b{0:b}", REG);
            VERIFY_NOT_REACHED();
            return "ERROR";
        }
    }
}
