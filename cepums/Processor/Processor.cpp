#include "cepumspch.h"
#include "Processor.h"

// Uncomment to force strict original 8086 instruction set
// Note: This may not be needed, but I'm not 100% sure. This is necessary
//  because 80186 added new instruction variants (for example, OR 0x83/1)
//  that somehow ended up on the 8086 instruction list. These need testing
//  on a real machine as they might just work on an 8086
//#define STRICT8086INSTRUCTIONSET

namespace Cepums {

    static bool s_debugSpam = false;

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

    void Processor::execute(MemoryManager& memoryManager, IOManager& io)
    {
        if (m_cyclesToWait > 0)
        {
            m_cyclesToWait--;
            return;
        }

        // Increment segment prefix counter if it's being used
        if (m_segmentPrefix != EMPTY_SEGMENT_OVERRIDE)
            m_segmentPrefixCounter++;

        // If the previous instruction hasn't reset the segment prefix (and counter), it means it hasn't handled it
        if (m_segmentPrefixCounter == 2)
        {
            VERIFY_NOT_REACHED();
        }

        // Handle external interrupts
        if (IS_BIT_SET(m_flags, INTERRUPT_ENABLE_FLAG) && io.hasPendingInterrupts())
        {
            // Use our existing interrupt handler
            uint16_t interrupt = io.getPendingInterrupt();
            if (interrupt == 0xE)
            {
                //s_debugSpam = true;
                DC_CORE_TRACE("int0E: IRQ6 AH={0:x} ", AH());
            }

            return ins$INT(memoryManager, interrupt);
        }

        uint8_t hopefully_an_instruction = memoryManager.readByte(m_codeSegment, m_instructionPointer);
        m_instructionPointer++;
        if (s_debugSpam)
        {
            DC_CORE_INFO("{0}: ===== Fetched new instruction: 0x{1:x} =====", m_currentCycleCounter++, hopefully_an_instruction);
            DC_CORE_TRACE(" AX: 0x{0:x}   BX: 0x{1:x}   CX: 0x{2:x}   DX: 0x{3:x}", AX(), BX(), CX(), DX());
            DC_CORE_TRACE(" DS: 0x{0:x}   CS: 0x{1:x}   SS: 0x{2:x}   ES: 0x{3:x}", DS(), CS(), SS(), ES());
            DC_CORE_TRACE(" IP: 0x{0:x}   BP: 0x{1:X}   SI: 0x{2:x}   DI: 0x{3:X}", IP(), BP(), SI(), DI());
        }
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
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$ADDregisterToMemory(memoryManager, segment, effectiveAddress, getRegisterValueFromREG8(regBits));
        }
        case 0x01: // ADD: 16-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$ADDregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits,  displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$ADDregisterToMemory(memoryManager, segment, effectiveAddress, getRegisterFromREG16(regBits));
        }
        case 0x02: // ADD: 8-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x03: // ADD: 16-bit from register/memory to register
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$ADDregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$ADDmemoryToRegisterWord(memoryManager, regBits, segment, effectiveAddress);
        }
        case 0x04: // ADD: 8-bit immediate to AL
        {
            INSTRUCTION_TRACE("ins$ADD: 8-bit immediate to AL");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            return ins$ADDimmediateToRegister(REGISTER_AL, byte);
        }
        case 0x05: // ADD: 16-bit immediate to AX
        {
            INSTRUCTION_TRACE("ins$ADD: 16-bit immediate to AX");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, word);
            return ins$ADDimmediateToRegister(REGISTER_AX, word);
        }
        case 0x06: // PUSH: Push ES to stack
        {
            return ins$PUSHsegmentRegister(memoryManager, REGISTER_ES);
        }
        case 0x07: // POP: Pop ES from stack
        {
            return ins$POPsegmentRegister(memoryManager, REGISTER_ES);
        }
        case 0x08: // OR: 8-bit register with register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$ORregisterToRegisterByte(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$ORregisterToMemory(memoryManager, segment, effectiveAddress, getRegisterValueFromREG8(regBits));
        }
        case 0x09: // OR: 16-bit register with register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$ORregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$ORregisterToMemory(memoryManager, segment, effectiveAddress, getRegisterFromREG16(regBits));
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
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            return ins$ORimmediateToRegister(REGISTER_AL, immediate);
        }
        case 0x0D: // OR: 16-bit immediate with AX
        {
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            return ins$ORimmediateToRegister(REGISTER_AX, immediate);
        }
        case 0x0E: // PUSH: Push CS to stack
        {
            return ins$PUSHsegmentRegister(memoryManager, REGISTER_CS);
        }
        case 0x10: // ADC: 8-bit from register to register/memory
        {
            TODO();
            return;
        }
        case 0x11: // ADC: 16-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$ADCregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$ADCregisterToMemory(memoryManager, segment, effectiveAddress, getRegisterFromREG16(regBits));
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
            return ins$PUSHsegmentRegister(memoryManager, REGISTER_SS);
        }
        case 0x017: // POP: Pop SS from stack
        {
            return ins$POPsegmentRegister(memoryManager, REGISTER_SS);
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
            return ins$PUSHsegmentRegister(memoryManager, REGISTER_DS);
        }
        case 0x1F: // POP: Pop DS from stack
        {
            return ins$POPsegmentRegister(memoryManager, REGISTER_DS);
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
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            return ins$ANDimmediateToRegister(REGISTER_AL, byte);
            //return ins$AND(memoryManager, createRef<Register8>(REGISTER_AL), createRef<Immediate8>(byte));
        }
        case 0x25: // AND: 16-bit immediate with AX
        {
            TODO();
            return;
        }
        case 0x26: // ES: Segment override prefix
        {
            INSTRUCTION_TRACE("ins$ES: Override segment prefix to ES for next instruction");
            m_segmentPrefix = REGISTER_ES;
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
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$SUBregisterToMemory(memoryManager, segment, effectiveAddress, getRegisterValueFromREG8(regBits));
        }
        case 0x29: // SUB: 16-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$SUBregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$SUBregisterToMemory(memoryManager, segment, effectiveAddress, getRegisterFromREG16(regBits));
        }
        case 0x2A: // SUB: 8-bit from register/memory to register
        {
            TODO();
            return;
        }
        case 0x2B: // SUB: 16-bit from register/memory to register
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$SUBregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$SUBmemoryToRegisterWord(memoryManager, regBits, segment, effectiveAddress);
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
            INSTRUCTION_TRACE("ins$CS: Override segment prefix to CS for next instruction");
            m_segmentPrefix = REGISTER_CS;
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
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$XORregisterToMemory(memoryManager, segment, effectiveAddress, getRegisterValueFromREG8(regBits));
        }
        case 0x31: // XOR: 16-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$XORregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$XORregisterToMemory(memoryManager, segment, effectiveAddress, getRegisterFromREG16(regBits));
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
            INSTRUCTION_TRACE("ins$SS: Override segment prefix to SS for next instruction");
            m_segmentPrefix = REGISTER_SS;
            return;
        }
        case 0x37: // AAA: ASCII adjust for addition
        {
            TODO();
            return;
        }
        case 0x38: // CMP: 8-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$CMPregisterToRegisterByte(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$CMPregisterToMemory(memoryManager, segment, effectiveAddress, getRegisterValueFromREG8(regBits));
        }
        case 0x39: // CMP: 16-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$CMPregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$CMPregisterToMemory(memoryManager, segment, effectiveAddress, getRegisterFromREG16(regBits));
        }
        case 0x3A: // CMP: 8-bit from register/memory to register
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$CMPregisterToRegisterByte(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$CMPmemoryToRegisterByte(memoryManager, regBits, segment, effectiveAddress);
        }
        case 0x3B: // CMP: 16-bit from register/memory to register
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$CMPregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$CMPmemoryToRegisterWord(memoryManager, regBits, segment, effectiveAddress);
        }
        case 0x3C: // CMP: 8-bit immediate with AL
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediateByte);
            return ins$CMPimmediateToRegister(REGISTER_AL, immediateByte);
        }
        case 0x3D: // CMP: 16-bit immediate with AX
        {
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediateWord);
            return ins$CMPimmediateToRegister(REGISTER_AX, immediateWord);
        }
        case 0x3E: // DS: Segment override prefix
        {
            INSTRUCTION_TRACE("ins$DS: Override segment prefix to DS for next instruction");
            m_segmentPrefix = REGISTER_DS;
            return;
        }
        case 0x3F: // AAS: ASCII adjust for subtraction
        {
            TODO();
            return;
        }
        case 0x40: // INC: AX
        {
            return ins$INCregister(IS_WORD, REGISTER_AX);
        }
        case 0x41: // INC: CX
        {
            return ins$INCregister(IS_WORD, REGISTER_CX);
        }
        case 0x42: // INC: DX
        {
            return ins$INCregister(IS_WORD, REGISTER_DX);
        }
        case 0x43: // INC: BX
        {
            return ins$INCregister(IS_WORD, REGISTER_BX);
        }
        case 0x44: // INC: SP
        {
            return ins$INCregister(IS_WORD, REGISTER_SP);
        }
        case 0x45: // INC: BP
        {
            return ins$INCregister(IS_WORD, REGISTER_BP);
        }
        case 0x46: // INC: SI
        {
            return ins$INCregister(IS_WORD, REGISTER_SI);
        }
        case 0x47: // INC: DI
        {
            return ins$INCregister(IS_WORD, REGISTER_DI);
        }
        case 0x48: // DEC: AX
        {
            return ins$DECregister(IS_WORD, REGISTER_AX);
        }
        case 0x49: // DEC: CX
        {
            return ins$DECregister(IS_WORD, REGISTER_CX);
        }
        case 0x4A: // DEC: DX
        {
            return ins$DECregister(IS_WORD, REGISTER_DX);
        }
        case 0x4B: // DEC: BX
        {
            return ins$DECregister(IS_WORD, REGISTER_BX);
        }
        case 0x4C: // DEC: SP
        {
            return ins$DECregister(IS_WORD, REGISTER_SP);
        }
        case 0x4D: // DEC: BP
        {
            return ins$DECregister(IS_WORD, REGISTER_BP);
        }
        case 0x4E: // DEC: SI
        {
            return ins$DECregister(IS_WORD, REGISTER_SI);
        }
        case 0x4F: // DEC: DI
        {
            return ins$DECregister(IS_WORD, REGISTER_DI);
        }
        case 0x50: // PUSH: AX
        {
            return ins$PUSHregisterWord(memoryManager, REGISTER_AX);
        }
        case 0x51: // PUSH: CX
        {
            return ins$PUSHregisterWord(memoryManager, REGISTER_CX);
        }
        case 0x52: // PUSH: DX
        {
            return ins$PUSHregisterWord(memoryManager, REGISTER_DX);
        }
        case 0x53: // PUSH: BX
        {
            return ins$PUSHregisterWord(memoryManager, REGISTER_BX);
        }
        case 0x54: // PUSH: SP
        {
            return ins$PUSHregisterWord(memoryManager, REGISTER_SP);
        }
        case 0x55: // PUSH: BP
        {
            return ins$PUSHregisterWord(memoryManager, REGISTER_BP);
        }
        case 0x56: // PUSH: SI
        {
            return ins$PUSHregisterWord(memoryManager, REGISTER_SI);
        }
        case 0x57: // PUSH: DI
        {
            return ins$PUSHregisterWord(memoryManager, REGISTER_DI);
        }
        case 0x58: // POP: AX
        {
            return ins$POPregisterWord(memoryManager, REGISTER_AX);
        }
        case 0x59: // POP: CX
        {
            return ins$POPregisterWord(memoryManager, REGISTER_CX);
        }
        case 0x5A: // POP: DX
        {
            return ins$POPregisterWord(memoryManager, REGISTER_DX);
        }
        case 0x5B: // POP: BX
        {
            return ins$POPregisterWord(memoryManager, REGISTER_BX);
        }
        case 0x5C: // POP: SP
        {
            return ins$POPregisterWord(memoryManager, REGISTER_SP);
        }
        case 0x5D: // POP: BP
        {
            return ins$POPregisterWord(memoryManager, REGISTER_BP);
        }
        case 0x5E: // POP: SI
        {
            return ins$POPregisterWord(memoryManager, REGISTER_SI);
        }
        case 0x5F: // POP: DI
        {
            return ins$POPregisterWord(memoryManager, REGISTER_DI);
        }
        case 0x70: // JO: Jump if overflow
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if OF=1");
            if (IS_BIT_SET(m_flags, OVERFLOW_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x71: // JNO: Jump if no overflow
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if OF=0");
            if (IS_BIT_NOT_SET(m_flags, OVERFLOW_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x72: // JB/JNAE/JC: Jump if below / Jump if not above nor equal / Jump if carry
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if CF=1");
            if (IS_BIT_SET(m_flags, CARRY_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x73: // JNB/JAE/JNC: Jump if not below / Jump if above or equal / Jump if not carry
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if CF=0");
            if (IS_BIT_NOT_SET(m_flags, CARRY_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x74: // JE/JZ: Jump if equal / Jump if zero
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if ZF=1");
            if (IS_BIT_SET(m_flags, ZERO_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x75: // JNE/JNZ: Jump if not equal / Jump if not zero
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if ZF=0");
            if (IS_BIT_NOT_SET(m_flags, ZERO_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x76: // JBE/JNA: Jump if below or equal / Jump if not above
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if CF=1 || ZF=1");
            if (IS_BIT_SET(m_flags, CARRY_FLAG) || IS_BIT_SET(m_flags, ZERO_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x77: // JNBE/JA: Jump if not below nor equal / Jump if above
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if CF=0 && ZF=0");
            if (IS_BIT_NOT_SET(m_flags, CARRY_FLAG) && IS_BIT_NOT_SET(m_flags, ZERO_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x78: // JS: Jump if sign
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if SF=1");
            if (IS_BIT_SET(m_flags, SIGN_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x79: // JNS: Jump if not sign
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if SF=0");
            if (IS_BIT_NOT_SET(m_flags, SIGN_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x7A: // JP/JPE: Jump if parity / Jump if parity even
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if PF=1");
            if (IS_BIT_SET(m_flags, PARITY_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x7B: // JNP/NPO: Jump if not parity / Jump if parity odd
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if PF=0");
            if (IS_BIT_NOT_SET(m_flags, PARITY_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x7C: // JL/JNGE: Jump if less / Jump if not greater nor equal
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if SF!=OF");
            if (IS_BIT_SET(m_flags, SIGN_FLAG) != IS_BIT_SET(m_flags, OVERFLOW_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x7D: // JNL/JGE: Jump if greater or equal / Jump if not less
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if SF=OF");
            if (IS_BIT_SET(m_flags, SIGN_FLAG) == IS_BIT_SET(m_flags, OVERFLOW_FLAG))
                return ins$JMPshort(increment);
            return;
        }
        case 0x7E: // JLE/JNG: Jump if less or equal / Jump if not greater
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if ZF=1 || (SF!=OF)");
            if (IS_BIT_SET(m_flags, ZERO_FLAG) || ( IS_BIT_SET(m_flags, SIGN_FLAG) != IS_BIT_SET(m_flags, OVERFLOW_FLAG)))
                return ins$JMPshort(increment);
            return;
        }
        case 0x7F: // JNLE/JG: Jump if not less nor equal / Jump if greater
        {
            LOAD_INCREMENT_BYTE(memoryManager, increment);
            INSTRUCTION_TRACE("ins$JMP: Jumping if ZF=0 && (SF=OF)");
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
                    return ins$ORimmediateToRegister(rmBits, immediate);
                case 0b010:
                    return ins$ADCimmediateToRegister(rmBits, immediate);
                case 0b011:
                    //return ins$SBBimmediateToRegister(rmBits, immediate);
                    TODO();
                case 0b100:
                    return ins$ANDimmediateToRegister(rmBits, immediate);
                case 0b101:
                    return ins$SUBimmediateToRegister(rmBits, immediate);
                case 0b110:
                    //return ins$XORimmediateToRegister(rmBits, immediate);
                    TODO();
                case 0b111:
                    return ins$CMPimmediateToRegister(rmBits, immediate);
                default:
                    ILLEGAL_INSTRUCTION();
                    return;
                }
            }

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);

            switch (regBits)
            {
            case 0b000:
                return ins$ADDimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b001:
                return ins$ORimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b010:
                //return ins$ADCimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b011:
                //return ins$SBBimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
                TODO();
            case 0b100:
                return ins$ANDimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b101:
                //return ins$SUBimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b110:
                //return ins$XORimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
                TODO();
            case 0b111:
                return ins$CMPimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            default:
                ILLEGAL_INSTRUCTION();
                return;
            }
        }
        case 0x81: // ADD/OR/ADC/SBB/AND/SUB/XOR/CMP: 16-bit immediate to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
            {
                LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
                switch (regBits)
                {
                case 0b000:
                    return ins$ADDimmediateToRegister(rmBits, immediate);
                case 0b001:
                    //return ins$ORimmediateToRegister(rmBits, immediate);
                case 0b010:
                    //return ins$ADCimmediateToRegister(rmBits, immediate);
                case 0b011:
                    //return ins$SBBimmediateToRegister(rmBits, immediate);
                case 0b100:
                    //return ins$ANDimmediateToRegister(rmBits, immediate);
                case 0b101:
                    //return ins$SUBimmediateToRegister(rmBits, immediate);
                case 0b110:
                    //return ins$XORimmediateToRegister(rmBits, immediate);
                    TODO();
                case 0b111:
                    return ins$CMPimmediateToRegister(rmBits, immediate);
                default:
                    ILLEGAL_INSTRUCTION();
                    return;
                }
            }

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);

            switch (regBits)
            {
            case 0b000:
                return ins$ADDimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b001:
                //return ins$ORimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b010:
                //return ins$ADCimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b011:
                //return ins$SBBimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b100:
                //return ins$ANDimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b101:
                //return ins$SUBimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b110:
                //return ins$XORimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
                TODO();
            case 0b111:
                return ins$CMPimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            default:
                ILLEGAL_INSTRUCTION();
                return;
            }
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
                    TODO();
                case 0b100:
#ifdef STRICT8086INSTRUCTIONSET
                    ILLEGAL_INSTRUCTION();
#endif
                    return ins$ANDimmediateToRegister(rmBits, immediate);
                case 0b101:
                    //return ins$SUBimmediateToRegisterWord(rmBits, immediate);
                    TODO();
                case 0b111:
                    return ins$CMPimmediateToRegister(rmBits, immediate);
                default:
                    ILLEGAL_INSTRUCTION();
                    return;
                }
            }

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate_byte);
            // Sign-extend to word
            uint16_t immediate = signExtendByteToWord(immediate_byte);
            switch (regBits)
            {
            case 0b000:
                return ins$ADDimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b001:
#ifdef STRICT8086INSTRUCTIONSET
                ILLEGAL_INSTRUCTION();
#endif
                return ins$ORimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b010:
                //return ins$ADCimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b011:
                //return ins$SBBimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            case 0b101:
                //return ins$SUBimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
                TODO();
            case 0b111:
                return ins$CMPimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
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
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$TESTregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            TODO();
            //return ins$TESTregisterToMemory(memoryManager, segment, effectiveAddress, getRegisterFromREG16(regBits));
            return;
        }
        case 0x86: // XCHG: 8-bit exchange from register/memory to register
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$XCHGregisterToRegisterByte(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$XCHGmemoryToRegisterByte(memoryManager, regBits, segment, effectiveAddress);
        }
        case 0x87: // XCHG: 16-bit exchange from register/memory to register
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$XCHGregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$XCHGmemoryToRegisterWord(memoryManager, regBits, segment, effectiveAddress);
        }
        case 0x88: // MOV: 8-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$MOV(memoryManager, createRef<Register8>(rmBits), createRef<Register8>(regBits));

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$MOV(memoryManager, createRef<Memory8>(segment, effectiveAddress), createRef<Register8>(regBits));
        }
        case 0x89: // MOV: 16-bit from register to register/memory
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$MOVregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$MOVregisterToMemory(memoryManager, segment, effectiveAddress, getRegisterFromREG16(regBits));
        }
        case 0x8A: // MOV: 8-bit from register/memory to register
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$MOVregisterToRegisterByte(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$MOVmemoryToRegisterByte(memoryManager, regBits, segment, effectiveAddress);
        }
        case 0x8B: // MOV: 16-bit from register/memory to register
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
                return ins$MOVregisterToRegisterWord(rmBits, regBits);

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$MOVmemoryToRegisterWord(memoryManager, regBits, segment, effectiveAddress);
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
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$MOVsegmentRegisterToMemoryWord(memoryManager, segment, effectiveAddress, srBits);
        }
        case 0x8D: // LEA: Load effective address
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            // I don't know if this is reachable
            if (IS_IN_REGISTER_MODE(modBits))
            {
                VERIFY_NOT_REACHED();
            }

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$LEA(regBits, effectiveAddress);
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
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            return ins$MOVmemoryToSegmentRegisterWord(memoryManager, srBits, segment, effectiveAddress);
        }
        case 0x8F: // POP/unused/unused/unused/unused/unused/unused/unused: Pop 16-bit register/memory from stack
        {
            TODO();
            return;
        }
        case 0x90: // NOP
        {
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
            return ins$XCHGregisterToRegisterWord(REGISTER_AX, REGISTER_SI);
        }
        case 0x97: // XCHG: Exchange AX and DI
        {
            TODO();
            return;
        }
        case 0x98: // CBW: Convert byte to word
        {
            return ins$CBW();
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
            return ins$PUSHF(memoryManager);
        }
        case 0x9D: // POPF: Pop flags from stack
        {
            return ins$POPF(memoryManager);
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
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, word);
            return ins$MOVmemoryToRegisterByte(memoryManager, REGISTER_AL, DATA_SEGMENT, word);
        }
        case 0xA1: // MOV: 16-bit from memory to AX
        {
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, word);
            return ins$MOVmemoryToRegisterWord(memoryManager, REGISTER_AX, DATA_SEGMENT, word);
        }
        case 0xA2: // MOV: 8-bit from AL to memory
        {
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, address);
            return ins$MOVregisterToMemory(memoryManager, DATA_SEGMENT, address, getRegisterValueFromREG8(REGISTER_AL));
        }
        case 0xA3: // MOV: 16-bit from AX to memory
        {
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, address);
            return ins$MOVregisterToMemory(memoryManager, DATA_SEGMENT, address, getRegisterFromREG16(REGISTER_AX));
        }
        case 0xA4: // MOVS: 8-bit move string from SRC-STR8 to DEST-STR8
        {
            TODO();
            return;
        }
        case 0xA5: // MOVS: 16-bit move string from SRC-STR16 to DEST-STR16
        {
            return ins$MOVSword(memoryManager);
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
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            return ins$TESTimmediateToRegister(REGISTER_AL, immediate);
        }
        case 0xA9: // TEST: 16-bit from immediate to AX
        {
            TODO();
            return;
        }
        case 0xAA: // STOS: 8-bit store byte or word string to DEST-STR8
        {
            return ins$STOSbyte(memoryManager);
        }
        case 0xAB: // STOS: 16-bit store byte or word string to DEST-STR8
        {
            return ins$STOSword(memoryManager);
        }
        case 0xAC: // LODS: 8-bit load string to SRC-STR8
        {
            return ins$LODSbyte(memoryManager);
        }
        case 0xAD: // LODS: 16-bit load string to SRC_STR16
        {
            return ins$LODSword(memoryManager);
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
            INSTRUCTION_TRACE("ins$MOV: 8-bit immediate to AL");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            AL(immediate);

            return;
        }
        case 0xB1: // MOV: 8-bit from immediate to CL
        {
            INSTRUCTION_TRACE("ins$MOV: 8-bit immediate to CL");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            CL(immediate);

            return;
        }
        case 0xB2: // MOV: 8-bit from immediate to DL
        {
            INSTRUCTION_TRACE("ins$MOV: 8-bit immediate to DL");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            DL(immediate);

            return;
        }
        case 0xB3: // MOV: 8-bit from immediate to BL
        {
            INSTRUCTION_TRACE("ins$MOV: 8-bit immediate to BL");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            BL(immediate);

            return;
        }
        case 0xB4: // MOV: 8-bit from immediate to AH
        {
            INSTRUCTION_TRACE("ins$MOV: 8-bit immediate to AH");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            AH(immediate);

            return;
        }
        case 0xB5: // MOV: 8-bit from immediate to CH
        {
            INSTRUCTION_TRACE("ins$MOV: 8-bit immediate to CH");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            CH(immediate);

            return;
        }
        case 0xB6: // MOV: 8-bit from immediate to DH
        {
            INSTRUCTION_TRACE("ins$MOV: 8-bit immediate to DH");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            DH(immediate);

            return;
        }
        case 0xB7: // MOV: 8-bit from immediate to BH
        {
            INSTRUCTION_TRACE("ins$MOV: 8-bit immediate to BH");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            BH(immediate);

            return;
        }
        case 0xB8: // MOV: 16-bit from immediate to AX
        {
            INSTRUCTION_TRACE("ins$MOV: 16-bit immediate to AX");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            AX() = immediate;

            return;
        }
        case 0xB9: // MOV: 16-bit from immediate to CX
        {
            INSTRUCTION_TRACE("ins$MOV: 16-bit immediate to CX");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            CX() = immediate;

            return;
        }
        case 0xBA: // MOV: 16-bit from immediate to DX
        {
            INSTRUCTION_TRACE("ins$MOV: 16-bit immediate to DX");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            DX() = immediate;

            return;
        }
        case 0xBB: // MOV: 16-bit from immediate to BX
        {
            INSTRUCTION_TRACE("ins$MOV: 16-bit immediate to BX");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            BX() = immediate;

            return;
        }
        case 0xBC: // MOV: 16-bit from immediate to SP
        {
            INSTRUCTION_TRACE("ins$MOV: 16-bit immediate to SP");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            SP() = immediate;

            return;
        }
        case 0xBD: // MOV: 16-bit from immediate to BP
        {
            INSTRUCTION_TRACE("ins$MOV: 16-bit immediate to BP");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            BP() = immediate;

            return;
        }
        case 0xBE: // MOV: 16-bit from immediate to SI
        {
            INSTRUCTION_TRACE("ins$MOV: 16-bit immediate to SI");
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            SI() = immediate;

            return;
        }
        case 0xBF: // MOV: 16-bit from immediate to DI
        {
            INSTRUCTION_TRACE("ins$MOV: 16-bit immediate to DI");
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
            return ins$RETnear(memoryManager);
        }
        case 0xC4: // LES: Load pointer using ES
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
            {
                DC_CORE_WARN("LES: I don't know what to do in this case");
                TODO();
            }

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, EXTRA_SEGMENT, segment);
            return ins$LEStoRegister(memoryManager, rmBits, segment, effectiveAddress);
        }
        case 0xC5: // LDS: Load pointer using DS
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
            {
                DC_CORE_WARN("LDS: I don't know what to do in this case");
                TODO();
            }

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);
            return ins$LEStoRegister(memoryManager, rmBits, segment, effectiveAddress); // Not a problem - just reusing implementation
        }
        case 0xC6: // MOV/unused/unused/unused/unused/unused/unused/unused: 8-bit from immediate to memory
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
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            return ins$MOVimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
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
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
            return ins$MOVimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
        }
        case 0xCA: // RET: Return intersegment adding immediate to SP
        {
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, word);
            return ins$RETfarAddImmediateToSP(memoryManager, word);
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
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
            if (immediate == 0x15)
            {
                //s_debugSpam = false;
            }
            if (immediate == 0x13)
            {
                DC_CORE_TRACE("int13: AH={0:x} ", AH());
                //TODO();
                //s_debugSpam = true;
            }
            return ins$INT(memoryManager, immediate);
        }
        case 0xCE: // INTO: Interrupt if overflow
        {
            TODO();
            return;
        }
        case 0xCF: // IRET: Interrupt return
        {
            return ins$IRET(memoryManager);
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
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            switch (regBits)
            {
            case 0b000:
                return ins$ROLmemoryOnceByte(memoryManager, segment, effectiveAddress);
            case 0b001:
                return ins$RORmemoryOnceByte(memoryManager, segment, effectiveAddress);
            case 0b010:
                return ins$RCLmemoryOnceByte(memoryManager, segment, effectiveAddress);
            case 0b011:
                return ins$RCRmemoryOnceByte(memoryManager, segment, effectiveAddress);
            case 0b100:
                return ins$SALmemoryOnceByte(memoryManager, segment, effectiveAddress);
            case 0b101:
                return ins$SHRmemoryOnceByte(memoryManager, segment, effectiveAddress);
            case 0b111:
                return ins$SARmemoryOnceByte(memoryManager, segment, effectiveAddress);
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
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            switch (regBits)
            {
            case 0b000:
                return ins$ROLmemoryOnceWord(memoryManager, segment, effectiveAddress);
            case 0b001:
                return ins$RORmemoryOnceWord(memoryManager, segment, effectiveAddress);
            case 0b010:
                return ins$RCLmemoryOnceWord(memoryManager, segment, effectiveAddress);
            case 0b011:
                return ins$RCRmemoryOnceWord(memoryManager, segment, effectiveAddress);
            case 0b100:
                return ins$SALmemoryOnceWord(memoryManager, segment, effectiveAddress);
            case 0b101:
                return ins$SHRmemoryOnceWord(memoryManager, segment, effectiveAddress);
            case 0b111:
                return ins$SARmemoryOnceWord(memoryManager, segment, effectiveAddress);
            default:
                ILLEGAL_INSTRUCTION();
                return;
            }
        }
        case 0xD2: // ROL/ROR/RCL/RCR/(SAL/SHL)/SHR/unused/SAR: 8-bit shift-like register/memory by CL
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);
            if (IS_IN_REGISTER_MODE(modBits))
            {
                switch (regBits)
                {
                case 0b000:
                    //return ins$ROLregisterByCLByte(rmBits);
                case 0b001:
                    //return ins$RORregisterByCLByte(rmBits);
                case 0b010:
                    //return ins$RCLregisterByCLByte(rmBits);
                case 0b011:
                    //return ins$RCRregisterByCLByte(rmBits);
                    TODO();
                case 0b100:
                    return ins$SALregisterByCLByte(rmBits);
                case 0b101:
                    return ins$SHRregisterByCLByte(rmBits);
                case 0b111:
                    //return ins$SARregisterByCLByte(rmBits);
                    TODO();
                default:
                    ILLEGAL_INSTRUCTION();
                    return;
                }
            }
            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            switch (regBits)
            {
            case 0b000:
                //return ins$ROLmemoryByCLByte(memoryManager, segment, effectiveAddress);
            case 0b001:
                //return ins$RORmemoryByCLByte(memoryManager, segment, effectiveAddress);
            case 0b010:
                //return ins$RCLmemoryByCLByte(memoryManager, segment, effectiveAddress);
            case 0b011:
                //return ins$RCRmemoryByCLByte(memoryManager, segment, effectiveAddress);
            case 0b100:
                //return ins$SALmemoryByCLByte(memoryManager, segment, effectiveAddress);
            case 0b101:
                //return ins$SHRmemoryByCLByte(memoryManager, segment, effectiveAddress);
            case 0b111:
                //return ins$SARmemoryByCLByte(memoryManager, segment, effectiveAddress);
                TODO();
            default:
                ILLEGAL_INSTRUCTION();
                return;
            }
        }
        case 0xD3: // ROL/ROR/RCL/RCR/(SAL/SHL)/SHR/unused/SAR: 16-bit shift-like register-memory by CL
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);
            if (IS_IN_REGISTER_MODE(modBits))
            {
                switch (regBits)
                {
                case 0b000:
                    //return ins$ROLregisterByCLWord(rmBits);
                case 0b001:
                    //return ins$RORregisterByCLWord(rmBits);
                case 0b010:
                    //return ins$RCLregisterByCLWord(rmBits);
                    TODO();
                case 0b011:
                    return ins$RCRregisterByCLWord(rmBits);
                case 0b100:
                    return ins$SALregisterByCLWord(rmBits);
                case 0b101:
                    return ins$SHRregisterByCLWord(rmBits);
                case 0b111:
                    //return ins$SARregisterByCLByte(rmBits);
                    TODO();
                default:
                    ILLEGAL_INSTRUCTION();
                    return;
                }
            }
            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            switch (regBits)
            {
            case 0b000:
                //return ins$ROLmemoryByCLWord(memoryManager, segment, effectiveAddress);
            case 0b001:
                //return ins$RORmemoryByCLWord(memoryManager, segment, effectiveAddress);
            case 0b010:
                //return ins$RCLmemoryByCLWord(memoryManager, segment, effectiveAddress);
            case 0b011:
                //return ins$RCRmemoryByCLWord(memoryManager, segment, effectiveAddress);
            case 0b100:
                //return ins$SALmemoryByCLWord(memoryManager, segment, effectiveAddress);
            case 0b101:
                //return ins$SHRmemoryByCLWord(memoryManager, segment, effectiveAddress);
            case 0b111:
                //return ins$SARmemoryByCLWord(memoryManager, segment, effectiveAddress);
                TODO();
            default:
                ILLEGAL_INSTRUCTION();
                return;
            }
        }
        case 0xD4: // AAM: ASCII adjust for multiply
        {
            TODO();
            return;
        }
        case 0xD5: // AAD: ASCII adjust for division
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            return ins$AAD(byte);
        }
        case 0xD7: // XLAT: Translate SOURCE-TABLE
        {
            TODO();
            return;
        }
        case 0xD8: // ESC: Escape to external device and variants
            TODO();
        case 0xD9: // FNSTCW: Store control word
        {
            // I think at least the word variant of this is 3 bytes
            m_instructionPointer += 3;
            return;
        }
        case 0xDA:
            TODO();
        case 0xDB: // FNINIT/FINIT (if WAIT 0x9B in front): Initialize FPU
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            return;
        }
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
            LOAD_INCREMENT_BYTE(memoryManager, byte);
            return ins$LOOP(byte);
        }
        case 0xE3: // JCXZ: Jump if CX is zero
        {
            TODO();
            return;
        }
        case 0xE4: // IN: 8-bit immediate and AL
        {
            INSTRUCTION_TRACE("ins$IN: Data from port immediate into AL");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, data);
            AL(io.readByte(data));
            return;
        }
        case 0xE5: // IN: 8-bit immediate and AX ??
        {
            TODO();
            return;
        }
        case 0xE6: // OUT: 8-bit immediate and AL
        {
            INSTRUCTION_TRACE("ins$OUT: immediate data to port AL");
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, data);
            io.writeByte(data, AL());
            return;
        }
        case 0xE7: // OUT: 8-bit immediate and AX ??
        {
            TODO();
            return;
        }
        case 0xE8: // CALL: Call NEAR-PROC
        {
            LOAD_INCREMENT_WORD(memoryManager, word);
            return ins$CALLnear(memoryManager, word);
        }
        case 0xE9: // JMP: Jump to NEAR-LABEL
        {
            LOAD_INCREMENT_WORD(memoryManager, word);
            return ins$JMPshortWord(word);
        }
        case 0xEA: // JMP: Jump to FAR-LABEL
        {
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, instructionPointer);
            LOAD_NEXT_INSTRUCTION_WORD(memoryManager, codeSegment);

            return ins$JMPinterSegment(codeSegment, instructionPointer);
        }
        case 0xEB: // JMP: Jump to SHORT-LABEL
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            return ins$JMPshort(byte);
        }
        case 0xEC: // IN: AL and DX
        {
            INSTRUCTION_TRACE("ins$IN: 8-bit data from port DX into AL");
            AL(io.readByte(DX()));
            return;
        }
        case 0xED: // IN: AX and DX
        {
            INSTRUCTION_TRACE("ins$IN: 16-bit data from port DX into AX");
            AX() = io.readWord(DX());
            return;
        }
        case 0xEE: // OUT: AL and DX
        {
            INSTRUCTION_TRACE("ins$OUT: DX to port AL");
            io.writeByte(DX(), AL());
            return;
        }
        case 0xEF: // OUT: AX and DX
        {
            TODO();
            // TODO: This needs to do 16-bit transfers I think
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
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            // Now find the real instruction :)
            switch (byte)
            {
            case 0xA4: // REP MOVS: 8-bit memory to memory
                TODO();
            case 0xA5: // REP MOVS: 16-bit memory to memory
                return ins$REP_MOVSword(memoryManager);
            case 0xA6: // CMPS: 8-bit compare string from SRC-STR8 to DEST-STR8
            case 0xA7: // CMPS: 16-bit compare string from SRC-STR16 to DEST-STR16
            case 0xAA: // REP STOS: 8-bit string
                TODO();
            case 0xAB: // REP STOS: 16-bit string
                return ins$REP_STOSword(memoryManager);
            case 0xAC: // REP LODS: 8-bit load string to SRC-STR8
            case 0xAD: // REP LODS: 16-bit load string to SRC-STR16
            case 0xAE: // REPE SCAS: 8-bit scan string to DEST-STR8
            case 0xAF: // REPE SCAS: 16-bit scan string to DEST-STR16
                TODO();
            default:
                ILLEGAL_INSTRUCTION();
                return;
            }
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
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
            {
                switch (regBits)
                {
                case 0b000:
                {
                    LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
                    return ins$TESTimmediateToRegister(rmBits, immediate);
                }
                case 0b010:
                    //return ins$NOTregisterByte(rmBits);
                case 0b011:
                    //return ins$NEGregisterByte(rmBits);
                    TODO();
                case 0b100:
                    return ins$MULregisterByte(rmBits);
                case 0b101:
                    //return ins$IMULergisterByte(rmBits);
                case 0b110:
                    //return ins$DIVregisterByte(rmBits);
                case 0b111:
                    //return ins$IDIVregisterByte(rmBits);
                    TODO();
                default:
                    ILLEGAL_INSTRUCTION();
                    return;
                }
            }

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            switch (regBits)
            {
            case 0b000:
            {
                LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, immediate);
                return ins$TESTimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            }
            case 0b010:
                //return ins$NOTmemoryByte(memoryManager, effectiveAddress);
            case 0b011:
                //return ins$NEGmemoryByte(memoryManager, effectiveAddress);
                TODO();
            case 0b100:
                return ins$MULmemoryByte(memoryManager, segment, effectiveAddress);
            case 0b101:
                //return ins$IMULmemoryByte(memoryManager, effectiveAddress);
            case 0b110:
                //return ins$DIVmemoryByte(memoryManager, effectiveAddress);
            case 0b111:
                //return ins$IDIVmemoryByte(memoryManager, effectiveAddress);
                TODO();
            default:
                ILLEGAL_INSTRUCTION();
                return;
            }
        }
        case 0xF7: // TEST/unused/NOT/NEG/MUL/IMUL/DIV/IDIV: (16-bit from immediate to register/memory)/(16-bit register/memory)
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
            {
                switch (regBits)
                {
                case 0b000:
                {
                    LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
                    //return ins$TESTimmediateToRegister(rmBits, immediate);
                    TODO();
                }
                case 0b010:
                    return ins$NOTregisterWord(rmBits);
                case 0b011:
                    //return ins$NEGregisterWord(rmBits);
                    TODO();
                case 0b100:
                    return ins$MULregisterWord(rmBits);
                case 0b101:
                    //return ins$IMULergisterWord(rmBits);
                    TODO();
                case 0b110:
                    return ins$DIVregisterWord(rmBits);
                case 0b111:
                    //return ins$IDIVregisterWord(rmBits);
                    TODO();
                default:
                    ILLEGAL_INSTRUCTION();
                    return;
                }
            }

            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            switch (regBits)
            {
            case 0b000:
            {
                LOAD_NEXT_INSTRUCTION_WORD(memoryManager, immediate);
                return ins$TESTimmediateToMemory(memoryManager, segment, effectiveAddress, immediate);
            }
            case 0b010:
                return ins$NOTmemoryWord(memoryManager, segment, effectiveAddress);
            case 0b011:
                //return ins$NEGmemoryWord(memoryManager, segment, effectiveAddress);
                TODO();
            case 0b100:
                return ins$MULmemoryWord(memoryManager, segment, effectiveAddress);
            case 0b101:
                //return ins$IMULmemoryWord(memoryManager, segment, effectiveAddress);
            case 0b110:
                //return ins$DIVmemoryWord(memoryManager, segment, effectiveAddress);
            case 0b111:
                //return ins$IDIVmemoryWord(memoryManager, segment, effectiveAddress);
                TODO();
            default:
                ILLEGAL_INSTRUCTION();
                return;
            }
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
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
            {
                switch (regBits)
                {
                case 0b000:
                    return ins$INCregister(IS_BYTE, rmBits);
                case 0b001:
                    return ins$DECregister(IS_BYTE, rmBits);
                default:
                    ILLEGAL_INSTRUCTION();
                    return;
                }
            }
            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_BYTE, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            switch (regBits)
            {
            case 0b000:
                return ins$INCmemoryByte(memoryManager, segment, effectiveAddress);
            case 0b001:
                return ins$DECmemoryByte(memoryManager, segment, effectiveAddress);
            default:
                ILLEGAL_INSTRUCTION();
                return;
            }
        }
        case 0xFF: // INC/DEC/CALL/CALL/JMP/JMP/PUSH/unused: 16-bit (memory)/(intrasegment register/memory)/(intrasegment memory)/(intrasegment register/memory)/(intersegment memory)/(memory)
        {
            LOAD_NEXT_INSTRUCTION_BYTE(memoryManager, byte);
            PARSE_MOD_REG_RM_BITS(byte, modBits, regBits, rmBits);

            if (IS_IN_REGISTER_MODE(modBits))
            {
                switch (regBits)
                {
                case 0b010:
                case 0b100:
                    TODO();
                    return;
                default:
                    VERIFY_NOT_REACHED();
                    return;
                }
            }
            LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(memoryManager, modBits, rmBits, displacementLowByte, displacementHighByte);
            CALCULATE_EFFECTIVE_ADDRESS(effectiveAddress, rmBits, modBits, IS_WORD, displacementLowByte, displacementHighByte, DATA_SEGMENT, segment);

            switch (regBits)
            {
            case 0b000:
                return ins$INCmemoryWord(memoryManager, segment, effectiveAddress);
            case 0b001:
                TODO();
                return;
            case 0b010: // CALL: Intrasegment
                return ins$CALLnearFromMemory(memoryManager, segment, effectiveAddress);
            case 0b011: // CALL: Intersegment
                TODO();
                return;
            case 0b100: // JMP: Intrasegment
                return ins$JMPnearFromMemory(memoryManager, segment, effectiveAddress);
            case 0b101:
                TODO();
                return;
            case 0b110:
                TODO();
                return;
            default:
                VERIFY_NOT_REACHED();
                return;
            }
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
        INSTRUCTION_TRACE("ins$HLT: Halting");
        TODO();
    }

    void Processor::ins$CLC()
    {
        INSTRUCTION_TRACE("ins$CLC: Clear carry flag");
        CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
    }

    void Processor::ins$CMC()
    {
        INSTRUCTION_TRACE("ins$CMC: Toggle Carry Flag");
        if (IS_BIT_SET(m_flags, CARRY_FLAG))
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }
        else
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        }
    }

    void Processor::ins$STC()
    {
        INSTRUCTION_TRACE("ins$STC: Set carry flag");
        SET_FLAG_BIT(m_flags, CARRY_FLAG);
    }

    void Processor::ins$CLD()
    {
        INSTRUCTION_TRACE("ins$CLD: Clear direction flag");
        CLEAR_FLAG_BIT(m_flags, DIRECTION_FLAG);
    }

    void Processor::ins$STD()
    {
        INSTRUCTION_TRACE("ins$STD: Set direction flag");
        SET_FLAG_BIT(m_flags, DIRECTION_FLAG);

    }

    void Processor::ins$CLI()
    {
        INSTRUCTION_TRACE("ins$CLI: Disable interrupts");
        CLEAR_FLAG_BIT(m_flags, INTERRUPT_ENABLE_FLAG);
    }

    void Processor::ins$STI()
    {
        INSTRUCTION_TRACE("ins$STI: Enabling interrupts");
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

    void Processor::ins$AAD(uint8_t immediate)
    {
        INSTRUCTION_TRACE("ins$AAD: ASCII adjust AX before division");
        // Intel pulled a sneaky and pretended that immediate could only be 0x0A (10) so NEC V20 only works in that mode and ignored immediate
        AL(AL() + (immediate * AH()));
        AH(0);
        setFlagsAfterArithmeticOperation(AL());
    }

    void Processor::ins$ADCimmediateToRegister(uint8_t destREG, uint8_t immediate)
    {
        INSTRUCTION_TRACE("ins$ADC: 8-bit immediate to register");
        uint8_t registerValue = getRegisterValueFromREG8(destREG);
        uint8_t carryFlag = IS_BIT_SET(m_flags, CARRY_FLAG);

        // Note: this may be UB :(
        uint8_t result = immediate + registerValue + carryFlag;

        // Carry (unsigned overflow)
        if (registerValue > USHRT_MAX - immediate - carryFlag)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            // Fix UB
            result = (USHRT_MAX - immediate) + registerValue + carryFlag;
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (registerValue > SHRT_MAX - immediate - carryFlag)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        updateRegisterFromREG8(destREG, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADCregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t registerValue)
    {
        INSTRUCTION_TRACE("ins$ADC: 16-bit register to memory");
        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);
        uint16_t carryFlag = IS_BIT_SET(m_flags, CARRY_FLAG);

        // Note: this may be UB :(
        uint16_t result = memoryValue + registerValue + carryFlag;

        // Carry (unsigned overflow)
        if (registerValue > USHRT_MAX - memoryValue - carryFlag)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            // Fix UB
            result = (USHRT_MAX - memoryValue) + registerValue + carryFlag;
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (registerValue > SHRT_MAX - memoryValue - carryFlag)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        memoryManager.writeWord(segment, effectiveAddress, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADCregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG)
    {
        INSTRUCTION_TRACE("ins$ADC: 16-bit register to register");
        uint16_t registerValue = getRegisterFromREG16(destREG);
        uint16_t sourceValue = getRegisterFromREG16(sourceREG);
        uint16_t carryFlag = IS_BIT_SET(m_flags, CARRY_FLAG);

        // Note: this may be UB :(
        uint16_t result = registerValue + sourceValue + carryFlag;

        // Carry (unsigned overflow)
        if (sourceValue > USHRT_MAX - registerValue - carryFlag)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            // Fix UB
            result = (USHRT_MAX - registerValue) + sourceValue + carryFlag;
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (sourceValue > SHRT_MAX - registerValue - carryFlag)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        updateRegisterFromREG16(destREG, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADDimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t immediate)
    {
        INSTRUCTION_TRACE("ins$ADD: 8-bit imediate to memory");
        uint8_t memoryValue = memoryManager.readByte(segment, effectiveAddress);
        
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

        memoryManager.writeByte(segment, effectiveAddress, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADDimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t immediate)
    {
        INSTRUCTION_TRACE("ins$ADD: 16-bit immediate to memory");
        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);

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

        memoryManager.writeWord(segment, effectiveAddress, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADDimmediateToRegister(uint8_t destREG, uint8_t value)
    {
        INSTRUCTION_TRACE("ins$ADD: 8-bit immediate to register");
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
        INSTRUCTION_TRACE("ins$ADD: 16-bit immediate to register");
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

    void Processor::ins$ADDmemoryToRegisterWord(MemoryManager& memoryManager, uint8_t regBits, uint16_t segment, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$ADD: 16-bit memory to register {0}", getRegisterNameFromREG16(regBits));
        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);
        uint16_t registerValue = getRegisterFromREG16(regBits);

        // Note: this may be UB :(
        uint16_t result = registerValue + memoryValue;

        // Carry (unsigned overflow)
        if (memoryValue > USHRT_MAX - registerValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            // Fix UB
            result = (USHRT_MAX - registerValue) + memoryValue;
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (memoryValue > SHRT_MAX - registerValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        updateRegisterFromREG16(regBits, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADDregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t registerValue)
    {
        INSTRUCTION_TRACE("ins$ADD: 8-bit register to memory");
        uint8_t memoryValue = memoryManager.readByte(segment, effectiveAddress);

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

        memoryManager.writeByte(segment, effectiveAddress, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADDregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t registerValue)
    {
        INSTRUCTION_TRACE("ins$ADD: 16-bit register to memory");
        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);

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

        memoryManager.writeWord(segment, effectiveAddress, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$ADDregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG)
    {
        INSTRUCTION_TRACE("ins$ADD: 8-bit register to register");
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
        INSTRUCTION_TRACE("ins$ADD: 16-bit register to register");
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

    void Processor::ins$ANDimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t immediate)
    {
        INSTRUCTION_TRACE("ins$AND: 8-bit immediate to memory");
        uint8_t memoryValue = memoryManager.readByte(segment, effectiveAddress);
        uint8_t result = memoryValue & immediate;
        memoryManager.writeByte(segment, effectiveAddress, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$ANDimmediateToRegister(uint8_t destREG, uint8_t value)
    {
        INSTRUCTION_TRACE("ins$AND: 8-bit immediate to register {0}", getRegisterNameFromREG8(destREG));
        uint8_t registerValue = getRegisterValueFromREG8(destREG);
        uint8_t result = registerValue & value;
        updateRegisterFromREG8(destREG, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$ANDimmediateToRegister(uint8_t destREG, uint16_t value)
    {
        INSTRUCTION_TRACE("ins$AND: 16-bit immediate to register {0}", getRegisterNameFromREG16(destREG));
        uint16_t registerValue = getRegisterFromREG16(destREG);
        uint16_t result = registerValue & value;
        updateRegisterFromREG16(destREG, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$CALLnear(MemoryManager& memoryManager, int16_t offset)
    {
        INSTRUCTION_TRACE("ins$CALL: near to {0:X}:{1:X}", m_codeSegment, offset + IP());
        // Start by pushing IP onto stack
        // Decrement the Stack Pointer (by size of register) before doing anything
        SP() -= 2;
        memoryManager.writeWord(SS(), SP(), IP());
        IP() += offset;
    }

    void Processor::ins$CALLnearFromMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        SP() -= 2;
        memoryManager.writeWord(SS(), SP(), IP());
        if (m_segmentPrefix != EMPTY_SEGMENT_OVERRIDE)
            segment = getSegmentRegisterValueAndResetOverride();

        IP() = memoryManager.readWord(segment, effectiveAddress);
        INSTRUCTION_TRACE("ins$CALL: near to {0:X}:{1:X}", segment, IP());
    }

    void Processor::ins$CBW()
    {
        INSTRUCTION_TRACE("ins$CBW: Sign-extend AL into AX");
        uint16_t extended = signExtendByteToWord(AL());
        AX() = extended;
    }

    void Processor::ins$CMPimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t immediate)
    {
        INSTRUCTION_TRACE("ins$CMP: 8-bit immediate to memory");
        if (m_segmentPrefix != EMPTY_SEGMENT_OVERRIDE)
            segment = getSegmentRegisterValueAndResetOverride();

        uint8_t memoryValue = memoryManager.readByte(segment, effectiveAddress);

        // Note: this may be UB :(
        uint8_t result = memoryValue - immediate;

        // Carry (unsigned overflow)
        if (immediate > memoryValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
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

        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$CMPimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t immediate)
    {
        INSTRUCTION_TRACE("ins$CMP: 16-bit immediate to memory");
        if (m_segmentPrefix != EMPTY_SEGMENT_OVERRIDE)
            segment = getSegmentRegisterValueAndResetOverride();

        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);

        // Note: this may be UB :(
        uint16_t result = memoryValue - immediate;

        // Carry (unsigned overflow)
        if (immediate > memoryValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
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

        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$CMPimmediateToRegister(uint8_t destREG, uint8_t immediate)
    {
        INSTRUCTION_TRACE("ins$CMP: 8-bit immediate to register {0}", getRegisterNameFromREG8(destREG));
        uint8_t registerValue = getRegisterValueFromREG8(destREG);

        // Note: this may be UB :(
        uint8_t result = registerValue - immediate;

        // Carry (unsigned overflow)
        if (immediate > registerValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (immediate > SCHAR_MAX - registerValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$CMPimmediateToRegister(uint8_t destREG, uint16_t immediate)
    {
        INSTRUCTION_TRACE("ins$CMP: 16-bit immediate to register {0}", getRegisterNameFromREG16(destREG));
        uint16_t registerValue = getRegisterFromREG16(destREG);

        // Note: this may be UB :(
        uint16_t result = registerValue - immediate;

        // Carry (unsigned overflow)
        if (immediate > registerValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (immediate > SHRT_MAX - registerValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$CMPmemoryToRegisterByte(MemoryManager& memoryManager, uint8_t regBits, uint16_t segment, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$CMP: 8-bit register to memory");
        uint8_t memoryValue = memoryManager.readByte(segment, effectiveAddress);
        uint8_t registerValue = getRegisterValueFromREG8(regBits);

        // Note: this may be UB :(
        uint8_t result = registerValue - memoryValue;

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

        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$CMPmemoryToRegisterWord(MemoryManager& memoryManager, uint8_t regBits, uint16_t segment, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$CMP: 16-bit memory to register");
        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);
        uint16_t registerValue = getRegisterFromREG16(regBits);

        // Note: this may be UB :(
        uint16_t result = registerValue - memoryValue;

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

        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$CMPregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG)
    {
        INSTRUCTION_TRACE("ins$CMP: 8-bit register {0} to register {1}", getRegisterNameFromREG8(sourceREG), getRegisterNameFromREG8(destREG));
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

        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$CMPregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG)
    {
        INSTRUCTION_TRACE("ins$CMP: 16-bit register {0} to register {1}", getRegisterNameFromREG16(sourceREG), getRegisterNameFromREG16(destREG));
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

        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$CMPregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t registerValue)
    {
        INSTRUCTION_TRACE("ins$CMP: 8-bit register to memory");
        uint8_t memoryValue = memoryManager.readByte(segment, effectiveAddress);

        // Note: this may be UB :(
        uint8_t result = memoryValue - registerValue;

        // Carry (unsigned overflow)
        if (memoryValue > registerValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (memoryValue > SCHAR_MAX - registerValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$CMPregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t registerValue)
    {
        INSTRUCTION_TRACE("ins$CMP: 16-bit register to memory");
        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);

        // Note: this may be UB :(
        uint16_t result = memoryValue - registerValue;

        // Carry (unsigned overflow)
        if (memoryValue > registerValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (memoryValue > SHRT_MAX - registerValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$DECregister(uint8_t isWordBit, uint8_t REG)
    {
        if (isWordBit)
        {
            INSTRUCTION_TRACE("ins$DEC: DEC {0}", getRegisterNameFromREG16(REG));
            uint16_t currentValue = getRegisterFromREG16(REG);
            uint16_t newValue = currentValue - 1;
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
            INSTRUCTION_TRACE("ins$DEC: DEC {0}", getRegisterNameFromREG8(REG));
            uint8_t currentValue = getRegisterValueFromREG8(REG);
            uint8_t newValue = currentValue - 1;
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

    void Processor::ins$DECmemoryByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$DECmemoryWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$DIVregisterWord(uint8_t REG)
    {
        INSTRUCTION_TRACE("ins$DIV: {0}", getRegisterNameFromREG16(REG));
        // Get divisor value
        uint16_t divisor = getRegisterFromREG16(REG);
        if (divisor == 0)
        {
            // Cause an interupt and stop
            TODO();
        }
        // Setup the dividend
        uint32_t dividend = AX();
        dividend = dividend | uint32_t(DX() << 16);

        uint32_t result = dividend / divisor;

        // If the result is too large to fit in 16-bits
        if (result > 0xFFFF)
        {
            // Interrupt and stop?
            TODO();
        }
        AX() = result;
        
        // Remainder
        uint16_t remainder = dividend % divisor;
        DX() = remainder;
    }

    void Processor::ins$INCregister(uint8_t isWordBit, uint8_t REG)
    {
        if (isWordBit)
        {
            INSTRUCTION_TRACE("ins$INC: INC {0}", getRegisterNameFromREG16(REG));
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
            INSTRUCTION_TRACE("ins$INC: INC {0}", getRegisterNameFromREG8(REG));
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

    void Processor::ins$INCmemoryByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$INC: 8-bit memory {0:X}:{1:X}", segment, effectiveAddress);
        uint8_t currentValue = memoryManager.readByte(segment, effectiveAddress);
        uint8_t newValue = currentValue + 1;
        memoryManager.writeByte(segment, effectiveAddress, newValue);

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

    void Processor::ins$INCmemoryWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$INC: 16-bit memory {0:X}:{1:X}", segment, effectiveAddress);
        uint16_t currentValue = memoryManager.readByte(segment, effectiveAddress);
        uint16_t newValue = currentValue + 1;
        memoryManager.writeWord(segment, effectiveAddress, newValue);

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

    void Processor::ins$INT(MemoryManager& memoryManager, uint16_t immediate)
    {
        INSTRUCTION_TRACE("ins$INT: Interrupt {0:X}", immediate);
        // Push flags
        SP() -= 2;
        memoryManager.writeWord(SS(), SP(), m_flags);
        // TODO: Handle TF
        // Clear IF and TF
        CLEAR_FLAG_BIT(m_flags, INTERRUPT_ENABLE_FLAG);
        CLEAR_FLAG_BIT(m_flags, TRAP_FLAG);
        // Push CS
        SP() -= 2;
        memoryManager.writeWord(SS(), SP(), CS());
        // Push IP
        SP() -= 2;
        memoryManager.writeWord(SS(), SP(), IP());

        // Get new CS:IP
        IP() = memoryManager.readWord(0, immediate * 4);
        CS() = memoryManager.readWord(0, immediate * 4 + 2);
    }

    void Processor::ins$IRET(MemoryManager& memoryManager)
    {
        INSTRUCTION_TRACE("ins$IRET: Returning from an interrupt service routine");
        // Pop IP 
        IP() = memoryManager.readWord(SS(), SP());
        SP() += 2;
        // Pop CS 
        CS() = memoryManager.readWord(SS(), SP());
        SP() += 2;
        // Pop flags
        m_flags = memoryManager.readWord(SS(), SP());
        SP() += 2;
    }

    void Processor::ins$JMPinterSegment(uint16_t newCodeSegment, uint16_t newInstructionPointer)
    {
        INSTRUCTION_TRACE("ins$JMP: Jumping to {0:x}:{1:x}", newCodeSegment, newInstructionPointer);

        // Debug: Print the BIOS ROM address
        if (newCodeSegment == 0xF000)
            INSTRUCTION_TRACE(".. which is at BIOS 0x{0:X} in HEX EDITOR or 0x{1:X} in the actual ROM", MemoryManager::addresstoPhysical(newCodeSegment, newInstructionPointer) - 0xF8000, MemoryManager::addresstoPhysical(newCodeSegment, newInstructionPointer) - 0xF0000);

        m_codeSegment = newCodeSegment;
        m_instructionPointer = newInstructionPointer;
    }

    void Processor::ins$JMPnearFromMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        if (m_segmentPrefix != EMPTY_SEGMENT_OVERRIDE)
            segment = getSegmentRegisterValueAndResetOverride();

        IP() = memoryManager.readWord(segment, effectiveAddress);
        INSTRUCTION_TRACE("ins$JMP: Jumping near to {0:X}:{1:X}", segment, IP());
    }

    void Processor::ins$JMPshort(int8_t increment)
    {
        INSTRUCTION_TRACE("ins$JMP: Jumping to short");
        m_instructionPointer += increment;
    }

    void Processor::ins$JMPshortWord(int16_t increment)
    {
        INSTRUCTION_TRACE("ins$JMP: Jumping to short");
        m_instructionPointer += increment;
    }

    void Processor::ins$LEA(uint8_t destREG, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$LEA: Storing '0x{0:X}' into {1}", effectiveAddress, getRegisterNameFromREG16(destREG));
        updateRegisterFromREG16(destREG, effectiveAddress);
    }

    void Processor::ins$LEStoRegister(MemoryManager& memoryManager, uint8_t destREG, uint16_t segment, uint16_t effectiveAddress)
    {
        uint16_t newRegisterValue = memoryManager.readWord(segment, effectiveAddress);
        uint16_t newSegmentValue = memoryManager.readWord(segment, effectiveAddress + 2);

        switch (m_segmentPrefix)
        {
        case REGISTER_DS:
            DS() = newSegmentValue;
            RESET_SEGMENT_PREFIX();
            break;

        case REGISTER_CS:
            CS() = newSegmentValue;
            RESET_SEGMENT_PREFIX();
            break;

        case REGISTER_SS:
            SS() = newSegmentValue;
            RESET_SEGMENT_PREFIX();
            break;
        case REGISTER_ES:
        default:
            ES() = newSegmentValue;
            RESET_SEGMENT_PREFIX();
            break;
        }
        updateRegisterFromREG16(destREG, newRegisterValue);
    }

    void Processor::ins$LODSbyte(MemoryManager& memoryManager)
    {
        std::string segRegName = getSegmentRegisterName(REGISTER_DS);
        switch (m_segmentPrefix)
        {
        case REGISTER_ES:
            segRegName = getSegmentRegisterName(m_segmentPrefix);
            AL(memoryManager.readByte(EXTRA_SEGMENT, SI()));
            RESET_SEGMENT_PREFIX();
            break;

        case REGISTER_CS:
            segRegName = getSegmentRegisterName(m_segmentPrefix);
            AL(memoryManager.readByte(CODE_SEGMENT, SI()));
            RESET_SEGMENT_PREFIX();
            break;

        case REGISTER_SS:
            segRegName = getSegmentRegisterName(m_segmentPrefix);
            AL(memoryManager.readByte(STACK_SEGMENT, SI()));
            RESET_SEGMENT_PREFIX();
            break;

        case REGISTER_DS:
        default:
            AL(memoryManager.readByte(DATA_SEGMENT, SI()));
            RESET_SEGMENT_PREFIX();
            break;
        }

        // Increment or decrement depending on DF
        if (IS_BIT_SET(m_flags, DIRECTION_FLAG))
            SI() -= 1;
        else
            SI() += 1;

        INSTRUCTION_TRACE("ins$LODS: Load {0}:SI word into AL", segRegName);
    }

    void Processor::ins$LODSword(MemoryManager& memoryManager)
    {
        std::string segRegName = getSegmentRegisterName(REGISTER_DS);
        switch (m_segmentPrefix)
        {
        case REGISTER_ES:
            segRegName = getSegmentRegisterName(m_segmentPrefix);
            AX() = memoryManager.readWord(ES(), SI());
            RESET_SEGMENT_PREFIX();
            break;

        case REGISTER_CS:
            segRegName = getSegmentRegisterName(m_segmentPrefix);
            AX() = memoryManager.readWord(CS(), SI());
            RESET_SEGMENT_PREFIX();
            break;

        case REGISTER_SS:
            segRegName = getSegmentRegisterName(m_segmentPrefix);
            AX() = memoryManager.readWord(SS(), SI());
            RESET_SEGMENT_PREFIX();
            break;

        case REGISTER_DS:
        default:
            AX() = memoryManager.readWord(DS(), SI());
            break;
        }

        // Increment or decrement depending on DF
        if (IS_BIT_SET(m_flags, DIRECTION_FLAG))
            SI() -= 2;
        else
            SI() += 2;

        INSTRUCTION_TRACE("ins$LODS: Load {0}:SI word into AX", segRegName);
    }

    void Processor::ins$LOOP(int8_t offset)
    {
        INSTRUCTION_TRACE("ins$LOOP: Loop with CX as counter");
        // Decrement at the start
        CX()--;
        // Get out of loop if CX == 0
        if (CX() == 0)
            return;

        // Otherwise we keep going
        IP() += offset;
    }

    void Processor::ins$MOV(MemoryManager& mm, Ref<Operand> destination, Ref<Operand> source)
    {
        if (destination->size() == OperandSize::Byte)
            destination->updateByte(this, mm, source->valueByte(this, mm));
        else
            destination->updateWord(this, mm, source->valueWord(this, mm));
    }

    void Processor::ins$MOVimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t immediate)
    {
        INSTRUCTION_TRACE("ins$MOV: 8-bit immediate to memory");
        memoryManager.writeByte(segment, effectiveAddress, immediate);
    }

    void Processor::ins$MOVimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t immediate)
    {
        INSTRUCTION_TRACE("ins$MOV: 16-bit immediate to memory");
        memoryManager.writeWord(segment, effectiveAddress, immediate);
    }

    void Processor::ins$MOVimmediateToRegisterByte(uint8_t reg, uint8_t immediate)
    {
        INSTRUCTION_TRACE("ins$MOV: 8-bit immediate to register");
        updateRegisterFromREG8(reg, immediate);
    }

    void Processor::ins$MOVimmediateToRegisterWord(uint8_t reg, uint16_t immediate)
    {
        INSTRUCTION_TRACE("ins$MOV: 16-bit immediate to register");
        updateRegisterFromREG16(reg, immediate);
    }

    void Processor::ins$MOVmemoryToRegisterByte(MemoryManager& memoryManager, uint8_t destREG, uint16_t segment, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$MOV: 8-bit memory to register {0}", getRegisterNameFromREG8(destREG));
        if (m_segmentPrefix != EMPTY_SEGMENT_OVERRIDE)
            segment = getSegmentRegisterValueAndResetOverride();
        uint8_t value = memoryManager.readByte(segment, effectiveAddress);
        updateRegisterFromREG8(destREG, value);
    }

    void Processor::ins$MOVmemoryToRegisterWord(MemoryManager& memoryManager, uint8_t destREG, uint16_t segment, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$MOV: 16-bit memory to register {0}", getRegisterNameFromREG16(destREG));
        if (m_segmentPrefix != EMPTY_SEGMENT_OVERRIDE)
            segment = getSegmentRegisterValueAndResetOverride();
        uint16_t value = memoryManager.readWord(segment, effectiveAddress);
        updateRegisterFromREG16(destREG, value);
    }

    void Processor::ins$MOVmemoryToSegmentRegisterWord(MemoryManager& memoryManager, uint8_t srBits, uint16_t segment, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$MOV: 16-bit memory to segment register");
        uint16_t value = memoryManager.readWord(segment, effectiveAddress);

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

    void Processor::ins$MOVregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t registerValue)
    {
        INSTRUCTION_TRACE("ins$MOV: 8-bit register to memory");
        memoryManager.writeByte(segment, effectiveAddress, registerValue);
    }

    void Processor::ins$MOVregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t registerValue)
    {
        INSTRUCTION_TRACE("ins$MOV: 16-bit register to memory");
        memoryManager.writeWord(segment, effectiveAddress, registerValue);
    }

    void Processor::ins$MOVregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG)
    {
        INSTRUCTION_TRACE("ins$MOV: 8-bit register to register");
        uint8_t sourceValue = getRegisterValueFromREG8(sourceREG);
        updateRegisterFromREG8(destREG, sourceValue);
    }

    void Processor::ins$MOVregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG)
    {
        INSTRUCTION_TRACE("ins$MOV: 16-bit register to register");
        uint16_t sourceValue = getRegisterFromREG16(sourceREG);
        updateRegisterFromREG16(destREG, sourceValue);
    }

    void Processor::ins$MOVregisterToSegmentRegisterWord(uint8_t srBits, uint16_t value)
    {
        INSTRUCTION_TRACE("ins$MOV: 16-bit register to segment register");
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

    void Processor::ins$MOVsegmentRegisterToMemoryWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t SEGREG)
    {
        INSTRUCTION_TRACE("ins$MOV: 16-bit segment register to memory");
        uint16_t segRegValue;
        switch (SEGREG)
        {
        case 0b00:
            segRegValue = m_extraSegment;
            break;

        case 0b01:
            segRegValue = m_codeSegment;
            break;

        case 0b10:
            segRegValue = m_stackSegment;
            break;

        case 0b11:
            segRegValue = m_dataSegment ;
            break;

        default:
            DC_CORE_ERROR("Malformed segment register bits : 0b{0:b}", SEGREG);
            VERIFY_NOT_REACHED();
            return;
        }

        memoryManager.writeWord(segment, effectiveAddress, segRegValue);
    }

    void Processor::ins$MOVsegmentRegisterToRegisterWord(uint8_t REG, uint8_t SEGREG)
    {
        INSTRUCTION_TRACE("ins$MOV: 16-bit segment register to register");
        uint16_t segRegValue;
        switch (SEGREG)
        {
        case 0b00:
            segRegValue = m_extraSegment;
            break;

        case 0b01:
            segRegValue = m_codeSegment;
            break;

        case 0b10:
            segRegValue = m_stackSegment;
            break;

        case 0b11:
            segRegValue = m_dataSegment;
            break;

        default:
            DC_CORE_ERROR("Malformed segment register bits : 0b{0:b}", SEGREG);
            VERIFY_NOT_REACHED();
            return;
        }

        updateRegisterFromREG16(REG, segRegValue);
    }

    void Processor::ins$MOVSword(MemoryManager& memoryManager)
    {
        uint16_t source = memoryManager.readWord(m_dataSegment, m_sourceIndex);
        memoryManager.writeWord(m_extraSegment, m_destinationIndex, source);

        // Increment if not set, decrement if set
        if (IS_BIT_SET(m_flags, DIRECTION_FLAG))
        {
            m_sourceIndex -= 2;
            m_destinationIndex -= 2;
        }
        else
        {
            m_sourceIndex += 2;
            m_destinationIndex += 2;
        }
    }

    void Processor::ins$MULmemoryByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$MUL: 8-bit memory");
        uint8_t memoryValue = memoryManager.readByte(segment, effectiveAddress);
        AX() = memoryValue * AL();
        if (AH() > 0)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        }
    }

    void Processor::ins$MULmemoryWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$MUL: 16-bit memory");
        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);
        uint32_t  result = memoryValue * AX();
        DX() = result >> 16; // Higher part
        AX() = result & 0xFFFF; // Lower part
        if (DX() > 0)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        }
    }

    void Processor::ins$MULregisterByte(uint8_t REG)
    {
        INSTRUCTION_TRACE("ins$MUL: 8-bit {0}", getRegisterNameFromREG8(REG));
        uint8_t registerValue = getRegisterValueFromREG8(REG);
        AX() = registerValue * AL();
        if (AH() > 0)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        }
    }

    void Processor::ins$MULregisterWord(uint8_t REG)
    {
        INSTRUCTION_TRACE("ins$MUL: 16-bit {0}", getRegisterNameFromREG16(REG));
        uint16_t registerValue = getRegisterFromREG16(REG);
        uint32_t  result = registerValue * AX();
        DX() = result >> 16; // Higher part
        AX() = result & 0xFFFF; // Lower part
        if (DX() > 0)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        }
    }

    void Processor::ins$NOTmemoryWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$NOT: 16-bit memory");
        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);
        memoryValue = ~memoryValue;
        memoryManager.writeWord(segment, effectiveAddress, memoryValue);
    }

    void Processor::ins$NOTregisterWord(uint8_t REG)
    {
        INSTRUCTION_TRACE("ins$NOT: 16-bit register");
        uint16_t registerValue = getRegisterFromREG16(REG);
        registerValue = ~registerValue;
        updateRegisterFromREG16(REG, registerValue);
    }

    void Processor::ins$ORimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t immediate)
    {
        INSTRUCTION_TRACE("ins$OR: 8-bit immediate to memory");
        uint8_t memoryValue = memoryManager.readByte(segment, effectiveAddress);
        uint8_t result = memoryValue | immediate;
        memoryManager.writeByte(segment, effectiveAddress, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$ORimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t immediate)
    {
        INSTRUCTION_TRACE("ins$OR: 16-bit immediate to memory");
        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);
        uint16_t result = memoryValue | immediate;
        memoryManager.writeWord(segment, effectiveAddress, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$ORimmediateToRegister(uint8_t destREG, uint8_t immediate)
    {
        INSTRUCTION_TRACE("ins$OR: 8-bit immediate to register");
        auto operand = getRegisterValueFromREG8(destREG);
        uint8_t result = operand | immediate;
        updateRegisterFromREG8(destREG, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$ORimmediateToRegister(uint8_t destREG, uint16_t immediate)
    {
        INSTRUCTION_TRACE("ins$OR: 16-bit immediate to register");
        auto operand = getRegisterFromREG16(destREG);
        uint16_t result = operand | immediate;
        updateRegisterFromREG16(destREG, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$ORregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t registerValue)
    {
        INSTRUCTION_TRACE("ins$OR: 8-bit register to memory");
        uint8_t memoryValue = memoryManager.readByte(segment, effectiveAddress);
        uint8_t result = memoryValue | registerValue;
        memoryManager.writeByte(segment, effectiveAddress, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$ORregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t registerValue)
    {
        INSTRUCTION_TRACE("ins$OR: 16-bit register to memory");
        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);
        uint16_t result = memoryValue | registerValue;
        memoryManager.writeWord(segment, effectiveAddress, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$ORregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG)
    {
        INSTRUCTION_TRACE("ins$OR: 8-bit register to register");
        uint8_t operand = getRegisterValueFromREG8(destREG);
        uint8_t operand2 = getRegisterValueFromREG8(sourceREG);
        uint8_t result = operand | operand2;
        updateRegisterFromREG8(destREG, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$ORregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG)
    {
        INSTRUCTION_TRACE("ins$OR: 16-bit register to register");
        uint16_t operand = getRegisterFromREG16(destREG);
        uint16_t operand2 = getRegisterFromREG16(sourceREG);
        uint16_t result = operand | operand2;
        updateRegisterFromREG16(destREG, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$POPF(MemoryManager& memoryManager)
    {
        INSTRUCTION_TRACE("ins$POPF: Pop flags");
        m_flags = memoryManager.readWord(SS(), SP());

        // Increment the Stack Pointer (by size of register)
        SP() += 2;
    }

    void Processor::ins$POPsegmentRegister(MemoryManager& memoryManager, uint8_t srBits)
    {
        INSTRUCTION_TRACE("ins$POP: segment register");
        switch (srBits)
        {
        case REGISTER_ES:
            ES() = memoryManager.readWord(SS(), SP());
            break;
        case REGISTER_CS:
            CS() = memoryManager.readWord(SS(), SP());
            break;
        case REGISTER_SS:
            SS() = memoryManager.readWord(SS(), SP());
            break;
        case REGISTER_DS:
            DS() = memoryManager.readWord(SS(), SP());
            break;
        default:
            ILLEGAL_INSTRUCTION();
            break;
        }

        // Increment the Stack Pointer (by size of register)
        SP() += 2;
    }

    void Processor::ins$POPregisterWord(MemoryManager& memoryManager, uint8_t REG)
    {
        INSTRUCTION_TRACE("ins$POP: register {0}", getRegisterNameFromREG16(REG));
        switch (REG)
        {
        case REGISTER_AX:
            AX() = memoryManager.readWord(SS(), SP());
            break;

        case REGISTER_CX:
            CX() = memoryManager.readWord(SS(), SP());
            break;

        case REGISTER_DX:
            DX() = memoryManager.readWord(SS(), SP());
            break;

        case REGISTER_BX:
            BX() = memoryManager.readWord(SS(), SP());
            break;

        case REGISTER_SP:
            SP() = memoryManager.readWord(SS(), SP());
            break;

        case REGISTER_BP:
            BP() = memoryManager.readWord(SS(), SP());
            break;

        case REGISTER_SI:
            SI() = memoryManager.readWord(SS(), SP());
            break;

        case REGISTER_DI:
            DI() = memoryManager.readWord(SS(), SP());
            break;

        default:
            ILLEGAL_INSTRUCTION();
            break;
        }

        // Increment the Stack Pointer (by size of register)
        SP() += 2;
    }

    void Processor::ins$PUSHF(MemoryManager& memoryManager)
    {
        INSTRUCTION_TRACE("ins$PUSHF: Push flags");
        // Decrement the Stack Pointer (by size of register) before doing anything
        SP() -= 2;
        memoryManager.writeWord(SS(), SP(), m_flags);
    }

    void Processor::ins$PUSHregisterByte(MemoryManager& memoryManager, uint8_t REG)
    {
        INSTRUCTION_TRACE("ins$PUSH: register {0}", getRegisterNameFromREG8(REG));
        // Decrement the Stack Pointer (by size of register) before doing anything
        SP() -= 1;
        switch (REG)
        {
        case REGISTER_AL:
            memoryManager.writeByte(SS(), SP(), AL());
            return;

        case REGISTER_CL:
            memoryManager.writeByte(SS(), SP(), CL());
            return;

        case REGISTER_DL:
            memoryManager.writeByte(SS(), SP(), DL());
            return;

        case REGISTER_BL:
            memoryManager.writeByte(SS(), SP(), BL());
            return;

        case REGISTER_AH:
            memoryManager.writeByte(SS(), SP(), AH());
            return;

        case REGISTER_CH:
            memoryManager.writeByte(SS(), SP(), CH());
            return;

        case REGISTER_DH:
            memoryManager.writeByte(SS(), SP(), DH());
            return;

        case REGISTER_BH:
            memoryManager.writeByte(SS(), SP(), BH());
            return;

        default:
            ILLEGAL_INSTRUCTION();
            break;
        }
    }

    void Processor::ins$PUSHregisterWord(MemoryManager& memoryManager, uint8_t REG)
    {
        INSTRUCTION_TRACE("ins$PUSH: register word");
        // Decrement the Stack Pointer (by size of register) before doing anything
        SP() -= 2;
        switch (REG)
        {
        case REGISTER_AX:
            memoryManager.writeWord(SS(), SP(), AX());
            return;

        case REGISTER_CX:
            memoryManager.writeWord(SS(), SP(), CX());
            return;

        case REGISTER_DX:
            memoryManager.writeWord(SS(), SP(), DX());
            return;

        case REGISTER_BX:
            memoryManager.writeWord(SS(), SP(), BX());
            return;

        case REGISTER_SP:
            memoryManager.writeWord(SS(), SP(), SP());
            return;

        case REGISTER_BP:
            memoryManager.writeWord(SS(), SP(), BP());
            return;

        case REGISTER_SI:
            memoryManager.writeWord(SS(), SP(), SI());
            return;

        case REGISTER_DI:
            memoryManager.writeWord(SS(), SP(), DI());
            return;

        default:
            ILLEGAL_INSTRUCTION();
            break;
        }
    }

    void Processor::ins$PUSHsegmentRegister(MemoryManager& memoryManager, uint8_t srBits)
    {
        INSTRUCTION_TRACE("ins$PUSH: segment register");
        // Decrement the Stack Pointer (by size of register) before doing anything
        SP() -= 2;
        switch (srBits)
        {
        case REGISTER_ES:
            memoryManager.writeWord(SS(), SP(), ES());
            break;
        case REGISTER_CS:
            memoryManager.writeWord(SS(), SP(), CS());
            break;
        case REGISTER_SS:
            memoryManager.writeWord(SS(), SP(), SS());
            break;
        case REGISTER_DS:
            memoryManager.writeWord(SS(), SP(), DS());
            break;
        default:
            ILLEGAL_INSTRUCTION();
            break;
        }
    }

    void Processor::ins$RCLmemoryOnceByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$RCLmemoryOnceWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
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

    void Processor::ins$RCRmemoryOnceByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$RCRmemoryOnceWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$RCRregisterByCLWord(uint8_t REG)
    {
        INSTRUCTION_TRACE("ins$RCR: {0},{1}", getRegisterNameFromREG16(REG), CL());
        uint16_t registerValue = getRegisterFromREG16(REG);
        auto counter = CL();
        while (counter != 0)
        {
            uint8_t bitZeroBefore = IS_BIT_SET(registerValue, 0);
            registerValue >>= 1;
            // Set MSB
            if (IS_BIT_SET(m_flags, CARRY_FLAG))
                SET_FLAG_BIT(registerValue, 15);
            else
                CLEAR_FLAG_BIT(registerValue, 15);
            // Set Carry flag
            if (bitZeroBefore)
                SET_FLAG_BIT(m_flags, CARRY_FLAG);
            else
                CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
            updateRegisterFromREG16(REG, registerValue);
            counter--;
        }
    }

    void Processor::ins$RCRregisterOnceByte(uint8_t REG)
    {
        TODO();
    }

    void Processor::ins$RCRregisterOnceWord(uint8_t REG)
    {
        TODO();
    }

    void Processor::ins$REP_MOVSword(MemoryManager& memoryManager)
    {
        INSTRUCTION_TRACE("ins$REP_MOVS: Repeat move string");
        while (CX() != 0)
        {
            uint16_t source = memoryManager.readWord(m_dataSegment, m_sourceIndex);
            memoryManager.writeWord(m_extraSegment, m_destinationIndex, source);
            if (IS_BIT_SET(m_flags, DIRECTION_FLAG))
            {
                m_sourceIndex -= 2;
                m_destinationIndex -= 2;
            }
            else
            {
                m_sourceIndex += 2;
                m_destinationIndex += 2;
            }
            CX()--;
        }
    }

    void Processor::ins$REP_STOSword(MemoryManager& memoryManager)
    {
        INSTRUCTION_TRACE("ins$REP_STOS: Repeat fill with string");
        while (CX() != 0)
        {
            memoryManager.writeWord(m_extraSegment, m_destinationIndex, AX());
            if (IS_BIT_SET(m_flags, DIRECTION_FLAG))
                m_destinationIndex -= 2;
            else
                m_destinationIndex += 2;
            CX()--;
        }
    }

    void Processor::ins$RETfarAddImmediateToSP(MemoryManager& memoryManager, uint16_t immediate)
    {
        INSTRUCTION_TRACE("ins$RET: Return to NEAR");
        // Pop into IP
        IP() = memoryManager.readWord(SS(), SP());
        SP() += 2;
        // Pop into CS
        CS() = memoryManager.readWord(SS(), SP());
        SP() += 2;
        // POP immediate bytes
        SP() += immediate;
    }

    void Processor::ins$RETnear(MemoryManager& memoryManager)
    {
        INSTRUCTION_TRACE("ins$RET: Return to NEAR");
        // Pop into IP
        IP() = memoryManager.readWord(SS(), SP());
        SP() += 2;
    }

    void Processor::ins$ROLmemoryOnceByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$ROLmemoryOnceWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$ROLregisterOnceByte(uint8_t REG)
    {
        INSTRUCTION_TRACE("ins$ROL: {0},1", getRegisterNameFromREG8(REG));
        uint8_t registerValue = getRegisterValueFromREG8(REG);

        uint8_t lastBit = IS_BIT_SET(registerValue, 7);
        registerValue <<= 1;
        // Set LSB
        if (lastBit)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            SET_FLAG_BIT(registerValue, 0);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
            CLEAR_FLAG_BIT(registerValue, 0);
        }
        updateRegisterFromREG16(REG, registerValue);
    }

    void Processor::ins$ROLregisterOnceWord(uint8_t REG)
    {
        TODO();
    }

    void Processor::ins$RORmemoryOnceByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$RORmemoryOnceWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$RORregisterOnceByte(uint8_t REG)
    {
        INSTRUCTION_TRACE("ins$ROR: {0},1", getRegisterNameFromREG8(REG));
        uint8_t registerValue = getRegisterValueFromREG8(REG);

        uint8_t firstBit = IS_BIT_SET(registerValue, 0);
        registerValue >>= 1;
        // Set MSB
        if (firstBit)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            SET_FLAG_BIT(registerValue, 7);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
            CLEAR_FLAG_BIT(registerValue, 7);
        }
        updateRegisterFromREG8(REG, registerValue);
    }

    void Processor::ins$RORregisterOnceWord(uint8_t REG)
    {
        INSTRUCTION_TRACE("ins$ROR: {0},1", getRegisterNameFromREG16(REG));
        uint16_t registerValue = getRegisterFromREG16(REG);

        uint16_t firstBit = IS_BIT_SET(registerValue, 0);
        registerValue >>= 1;
        // Set MSB
        if (firstBit)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
            SET_FLAG_BIT(registerValue, 15);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
            CLEAR_FLAG_BIT(registerValue, 15);
        }
        updateRegisterFromREG16(REG, registerValue);
    }

    void Processor::ins$SALmemoryOnceByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$SALmemoryOnceWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$SALregisterByCLByte(uint8_t rmBits)
    {
        INSTRUCTION_TRACE("ins$SAL: {0},{1}", getRegisterNameFromREG8(rmBits), CL());
        uint8_t registerValue = getRegisterValueFromREG8(rmBits);
        auto counter = CL();
        while (counter != 0)
        {
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
            if (bitZeroBefore != IS_BIT_SET(registerValue, 7))
                SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
            updateRegisterFromREG8(rmBits, registerValue);
            counter--;
        }
    }

    void Processor::ins$SALregisterByCLWord(uint8_t rmBits)
    {
        INSTRUCTION_TRACE("ins$SAL: {0},{1}", getRegisterNameFromREG16(rmBits), CL());
        uint16_t registerValue = getRegisterFromREG16(rmBits);
        auto counter = CL();
        while (counter != 0)
        {
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
            if (bitZeroBefore != IS_BIT_SET(registerValue, 15))
                SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
            updateRegisterFromREG16(rmBits, registerValue);
            counter--;
        }
    }

    void Processor::ins$SALregisterOnceByte(uint8_t rmBits)
    {
        INSTRUCTION_TRACE("ins$SAL: {0},1", getRegisterNameFromREG8(rmBits));
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
        INSTRUCTION_TRACE("ins$SAL: {0},1", getRegisterNameFromREG16(rmBits));
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

    void Processor::ins$SARmemoryOnceByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$SARmemoryOnceWord(MemoryManager & memoryManager, uint16_t segment, uint16_t effectiveAddress)
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

    void Processor::ins$SHRmemoryOnceByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$SHRmemoryOnceWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress)
    {
        TODO();
    }

    void Processor::ins$SHRregisterByCLByte(uint8_t rmBits)
    {
        INSTRUCTION_TRACE("ins$SHR: {0},{1}", getRegisterNameFromREG8(rmBits), CL());
        uint8_t registerValue = getRegisterValueFromREG8(rmBits);
        uint8_t MSBbefore = IS_BIT_SET(registerValue, 7);
        auto counter = CL();
        while (counter != 0)
        {
            uint8_t bitZeroBefore = IS_BIT_SET(registerValue, 7);
            bool setCarry;
            if (registerValue > SCHAR_MAX)
                setCarry = true;
            else
                setCarry = false;

            registerValue >>= 1;
            setFlagsAfterLogicalOperation(registerValue);
            // Set carry flag
            if (setCarry)
                SET_FLAG_BIT(m_flags, CARRY_FLAG);
            else
                CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
            // Set overflow flag
            if (MSBbefore)
                SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
            else
                CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);
            updateRegisterFromREG8(rmBits, registerValue);
            counter--;
        }
    }

    void Processor::ins$SHRregisterByCLWord(uint8_t rmBits)
    {
        INSTRUCTION_TRACE("ins$SHR: {0},{1}", getRegisterNameFromREG16(rmBits), CL());
        uint16_t registerValue = getRegisterFromREG16(rmBits);
        uint8_t MSBbefore = IS_BIT_SET(registerValue, 15);
        auto counter = CL();
        while (counter != 0)
        {
            uint8_t bitZeroBefore = IS_BIT_SET(registerValue, 15);
            bool setCarry;
            if (registerValue > SHRT_MAX)
                setCarry = true;
            else
                setCarry = false;

            registerValue >>= 1;
            setFlagsAfterLogicalOperation(registerValue);
            // Set carry flag
            if (setCarry)
                SET_FLAG_BIT(m_flags, CARRY_FLAG);
            else
                CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
            // Set overflow flag
            if (MSBbefore)
                SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
            else
                CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);
            updateRegisterFromREG16(rmBits, registerValue);
            counter--;
        }
    }

    void Processor::ins$SHRregisterOnceByte(uint8_t rmBits)
    {
        INSTRUCTION_TRACE("ins$SHR: {0},1", getRegisterNameFromREG8(rmBits));
        uint8_t registerValue = getRegisterValueFromREG8(rmBits);
        uint8_t MSBbefore = IS_BIT_SET(registerValue, 7);
        bool setCarry;
        if (IS_BIT_SET(registerValue, 0))
            setCarry = true;
        else
            setCarry = false;

        registerValue >>= 1;
        setFlagsAfterLogicalOperation(registerValue);
        // Set carry flag
        if (setCarry)
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        // Set overflow flag
        if (MSBbefore)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        updateRegisterFromREG8(rmBits, registerValue);
    }

    void Processor::ins$SHRregisterOnceWord(uint8_t rmBits)
    {
        INSTRUCTION_TRACE("ins$SHR: {0},1", getRegisterNameFromREG16(rmBits));
        uint16_t registerValue = getRegisterFromREG16(rmBits);
        uint8_t MSBbefore = IS_BIT_SET(registerValue, 15);
        bool setCarry;
        if (IS_BIT_SET(registerValue, 0))
            setCarry = true;
        else
            setCarry = false;

        registerValue >>= 1;
        setFlagsAfterLogicalOperation(registerValue);
        // Set carry flag
        if (setCarry)
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        // Set overflow flag
        if (MSBbefore)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        updateRegisterFromREG16(rmBits, registerValue);
    }

    void Processor::ins$STOSbyte(MemoryManager& memoryManager)
    {
        INSTRUCTION_TRACE("ins$STOS: Store AL into ES:DI");
        memoryManager.writeByte(m_extraSegment, m_destinationIndex, AL());
        // Increment if not set, decrement if set
        if (IS_BIT_SET(m_flags, DIRECTION_FLAG))
            m_destinationIndex -= 1;
        else
            m_destinationIndex += 1;
    }

    void Processor::ins$STOSword(MemoryManager& memoryManager)
    {
        INSTRUCTION_TRACE("ins$STOS: Store AX into ES:DI");
        memoryManager.writeWord(m_extraSegment, m_destinationIndex, AX());
        // Increment if not set, decrement if set
        if (IS_BIT_SET(m_flags, DIRECTION_FLAG))
            m_destinationIndex -= 2;
        else
            m_destinationIndex += 2;
    }

    void Processor::ins$SUBimmediateToRegister(uint8_t destREG, uint8_t value)
    {
        INSTRUCTION_TRACE("ins$SUB: 8-bit immediate to register");
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
        INSTRUCTION_TRACE("ins$SUB: 16-bit immediate to register");
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

    void Processor::ins$SUBmemoryToRegisterWord(MemoryManager& memoryManager, uint8_t destREG, uint16_t segment, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$SUB: 16-bit immediate to register");
        uint16_t registerValue = getRegisterFromREG16(destREG);
        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);

        // Note: this may be UB :(
        uint16_t result = registerValue - memoryValue;

        // Carry (unsigned overflow)
        if (memoryValue > registerValue)
        {
            SET_FLAG_BIT(m_flags, CARRY_FLAG);
        }
        else
        {
            CLEAR_FLAG_BIT(m_flags, CARRY_FLAG);
        }

        // Overflow
        if (memoryValue > SHRT_MAX - registerValue)
            SET_FLAG_BIT(m_flags, OVERFLOW_FLAG);
        else
            CLEAR_FLAG_BIT(m_flags, OVERFLOW_FLAG);

        updateRegisterFromREG16(destREG, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$SUBregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t registerValue)
    {
        INSTRUCTION_TRACE("ins$SUB: 8-bit register to memory");
        uint8_t memoryValue = memoryManager.readByte(segment, effectiveAddress);

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

        memoryManager.writeByte(segment, effectiveAddress, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$SUBregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t registerValue)
    {
        INSTRUCTION_TRACE("ins$SUB: 16-bit register to memory");
        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);

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

        memoryManager.writeWord(segment, effectiveAddress, result);
        setFlagsAfterArithmeticOperation(result);
    }

    void Processor::ins$SUBregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG)
    {
        INSTRUCTION_TRACE("ins$SUB: 8-bit register to register");
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
        INSTRUCTION_TRACE("ins$SUB: 16-bit register to register");
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

    void Processor::ins$TESTimmediateToRegister(uint8_t destREG, uint8_t value)
    {
        INSTRUCTION_TRACE("ins$TEST: 8-bit immediate to register");
        uint8_t registerValue = getRegisterValueFromREG8(destREG);
        uint8_t result = registerValue & value;
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$TESTimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t immediate)
    {
        INSTRUCTION_TRACE("ins$TEST: 8-bit immediate to memory");
        if (m_segmentPrefix != EMPTY_SEGMENT_OVERRIDE)
            segment = getSegmentRegisterValueAndResetOverride();
        uint8_t memoryValue = memoryManager.readByte(segment, effectiveAddress);
        uint8_t result = memoryValue & immediate;
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$TESTimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t immediate)
    {
        INSTRUCTION_TRACE("ins$TEST: 16-bit immediate to memory");
        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);
        uint16_t result = memoryValue & immediate;
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$TESTregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG)
    {
        INSTRUCTION_TRACE("ins$TEST: 8-bit register to register");
        uint16_t registerValue = getRegisterFromREG16(destREG);
        uint16_t operand2 = getRegisterFromREG16(sourceREG);
        uint16_t result = registerValue & operand2;
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$XCHGmemoryToRegisterByte(MemoryManager& memoryManager, uint8_t destREG, uint16_t segment, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$XCHG: Exchange 8-bit memory with {0}", getRegisterNameFromREG8(destREG));
        uint8_t memoryValue = memoryManager.readByte(segment, effectiveAddress);
        uint8_t registerValue = getRegisterValueFromREG8(destREG);
        memoryManager.writeByte(segment, effectiveAddress, registerValue);
        updateRegisterFromREG8(destREG, memoryValue);
    }

    void Processor::ins$XCHGmemoryToRegisterWord(MemoryManager& memoryManager, uint8_t destREG, uint16_t segment, uint16_t effectiveAddress)
    {
        INSTRUCTION_TRACE("ins$XCHG: Exchange 16-bit memory with {0}", getRegisterNameFromREG16(destREG));
        uint16_t memoryValue = memoryManager.readWord(segment, effectiveAddress);
        uint16_t registerValue = getRegisterFromREG16(destREG);
        memoryManager.writeWord(segment, effectiveAddress, registerValue);
        updateRegisterFromREG16(destREG, memoryValue);
    }

    void Processor::ins$XCHGregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG)
    {
        INSTRUCTION_TRACE("ins$XCHG: Exchange 8-bit {0} and {1}", getRegisterNameFromREG8(destREG), getRegisterNameFromREG8(sourceREG));
        uint8_t registerOneValue = getRegisterValueFromREG8(destREG);
        uint8_t registerTwoValue = getRegisterValueFromREG8(sourceREG);
        updateRegisterFromREG8(destREG, registerTwoValue);
        updateRegisterFromREG8(sourceREG, registerOneValue);
    }

    void Processor::ins$XCHGregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG)
    {
        INSTRUCTION_TRACE("ins$XCHG: Exchange 16-bit {0} and {1}", getRegisterNameFromREG16(destREG), getRegisterNameFromREG16(sourceREG));
        uint16_t registerOneValue = getRegisterFromREG16(destREG);
        uint16_t registerTwoValue = getRegisterFromREG16(sourceREG);
        updateRegisterFromREG16(destREG, registerTwoValue);
        updateRegisterFromREG16(sourceREG, registerOneValue);
    }

    void Processor::ins$XORregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t registerValue)
    {
        INSTRUCTION_TRACE("ins$XOR: XOR-ing 8-bit register to memory");
        auto original = memoryManager.readByte(segment, effectiveAddress);
        uint8_t result = original ^ registerValue;
        memoryManager.writeByte(segment, effectiveAddress, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$XORregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t registerValue)
    {
        INSTRUCTION_TRACE("ins$XOR: XOR-ing 16-bit register to memory");
        auto original = memoryManager.readWord(segment, effectiveAddress);
        uint16_t result = original ^ registerValue;
        memoryManager.writeWord(segment, effectiveAddress, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$XORregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG)
    {
        INSTRUCTION_TRACE("ins$XOR: 8-bit register to register");
        auto operand = getRegisterValueFromREG8(sourceREG);
        auto operand2 = getRegisterValueFromREG8(destREG);
        uint8_t result = operand ^ operand2;
        updateRegisterFromREG8(destREG, result);
        setFlagsAfterLogicalOperation(result);
    }

    void Processor::ins$XORregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG)
    {
        INSTRUCTION_TRACE("ins$XOR: 16-bit register to register");
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

    uint16_t Processor::getSegmentRegisterValueAndResetOverride()
    {
        uint16_t value = 0;
        switch (m_segmentPrefix)
        {
        case REGISTER_ES:
            value = ES();
            break;

        case REGISTER_CS:
            value = CS();
            break;

        case REGISTER_SS:
            value = SS();
            break;

        case REGISTER_DS:
            value = DS();
            break;
        default:
            DC_CORE_ERROR("Segment override not set");
            VERIFY_NOT_REACHED();
            break;
        }
        RESET_SEGMENT_PREFIX();
        return value;
    }

    uint16_t Processor::getEffectiveAddressFromBits(uint8_t rmBits, uint8_t modBits, uint8_t isWord, uint8_t displacementLow, uint8_t displacementHigh, uint16_t defaultSegment, uint16_t& segment)
    {
        segment = defaultSegment;
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
                segment = SS();
                return m_basePointer + m_sourceIndex;

            case 0b011:
                segment = SS();
                return m_basePointer + m_destinationIndex;

            case 0b100:
                return m_sourceIndex;

            case 0b101:
                return m_destinationIndex;

            case 0b110: // We use the "displacement" directly as it acts like a direct address at this point
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
        {
            uint16_t fullDisplacement = signExtendByteToWord(displacementLow);
            switch (rmBits)
            {
            case 0b000:
                return m_BX + m_sourceIndex + fullDisplacement;

            case 0b001:
                return m_BX + m_destinationIndex + fullDisplacement;

            case 0b010:
                segment = SS();
                return m_basePointer + m_sourceIndex + fullDisplacement;

            case 0b011:
                segment = SS();
                return m_basePointer + m_destinationIndex + fullDisplacement;

            case 0b100:
                return m_sourceIndex + fullDisplacement;

            case 0b101:
                return m_destinationIndex + fullDisplacement;

            case 0b110:
                segment = SS();
                return m_basePointer + fullDisplacement;

            case 0b111:
                return m_BX + fullDisplacement;

            default:
                VERIFY_NOT_REACHED();
                return 0;
            }
        }
        case 0b10:
        {
            uint16_t fullDisplacement;
            SET8BITREGISTERLOW(fullDisplacement, displacementLow);
            SET8BITREGISTERHIGH(fullDisplacement, displacementHigh);

            switch (rmBits)
            {
            case 0b000:
                return m_BX + m_sourceIndex + fullDisplacement;

            case 0b001:
                return m_BX + m_destinationIndex + fullDisplacement;

            case 0b010:
                segment = SS();
                return m_basePointer + m_sourceIndex + fullDisplacement;

            case 0b011:
                segment = SS();
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

    void Processor::loadDisplacementsFromInstructionStream(MemoryManager& memoryManager, uint8_t modBits, uint8_t rmBits, uint8_t& displacementLowByte, uint8_t& displacementHighByte)
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

    const char* Processor::getSegmentRegisterName(uint8_t REG)
    {
        switch (REG)
        {
        case REGISTER_ES:
            return "ES";
        case REGISTER_CS:
            return "CS";
        case REGISTER_SS:
            return "SS";
        case REGISTER_DS:
            return "DS";
        default:
            DC_CORE_ERROR("Malformed SegReg bits : 0b{0:b}", REG);
            VERIFY_NOT_REACHED();
            return "ERROR";
        }
    }
}
