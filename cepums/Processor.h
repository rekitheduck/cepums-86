#pragma once

#include "IOManager.h"
#include "MemoryManager.h"

#define HIGHER(word) (uint8_t)(word >> 8)
#define LOWER(word) word & 0xFF
#define HIGHER_HALFBYTE(byte) (byte & 0xF0) >> 4
#define LOWER_HALFBYTE(byte) byte & 0xF

#define REGBITS(pos, byte) byte >>= pos; byte &= 0x7
#define MODBITS(pos, byte) byte >>= pos; byte &= 0x3
#define RMBITS(pos, byte) byte >>= pos; byte &= 0x7
#define SRBITS(pos, byte) byte >>= pos; byte &=0x3;

#define SET8BITREGISTERHIGH(reg, data) reg &= 0x00FF; uint16_t temp = data << 8; reg |= temp & 0xFF00
#define SET8BITREGISTERLOW(reg, data) reg &= 0xFF00; reg |= data & 0x00FF

#define LOAD_NEXT_INSTRUCTION_BYTE(mm, byte) uint8_t byte = mm.readByte(m_codeSegment, m_instructionPointer); m_instructionPointer++
#define LOAD_NEXT_INSTRUCTION_WORD(mm, word) uint16_t word = mm.readWord(m_codeSegment, m_instructionPointer); m_instructionPointer += 2
#define LOAD_INCREMENT_BYTE(mm, byte) int8_t byte = mm.readByte(m_codeSegment, m_instructionPointer); m_instructionPointer++;
#define LOAD_INCREMENT_WORD(mm, word) int16_t word = mm.readWord(m_codeSegment, m_instructionPointer); m_instructionPointer += 2
#define PARSE_MOD_REG_RM_BITS(byte, mod, reg, rm) uint8_t rm = byte; RMBITS(0, rm); uint8_t reg = byte; REGBITS(3, reg); uint8_t mod = byte; MODBITS(6, mod)
#define LOAD_DISPLACEMENTS_FROM_INSTRUCTION_STREAM(mm, modBits, rmBits, displLow, displHigh) uint8_t displLow = 0; uint8_t displHigh = 0; loadDisplacementsFromInstructionStream(mm, modBits, rmBits, displLow, displHigh)
#define CALCULATE_EFFECTIVE_ADDRESS(ea, rmBits, modBits, isWord, displLow, displHigh, defaultSegmentValue, segment) uint16_t segment; uint16_t ea = getEffectiveAddressFromBits(rmBits, modBits, isWord, displLow, displHigh, defaultSegmentValue, segment)
#define RESET_SEGMENT_PREFIX() m_segmentPrefix = EMPTY_SEGMENT_OVERRIDE; m_segmentPrefixCounter = 0

#define IS_IN_REGISTER_MODE(mod) mod == 0b11
#define IS_IN_MEMORY_BODE_NO_DISPLACEMENT(mod) mod == 0b00

#define CLEAR_FLAG_BIT(flags, flag_bit) flags &= ~(BIT(flag_bit))
#define SET_FLAG_BIT(flags, flag_bit) flags |= BIT(flag_bit)

#define IS_BYTE 0
#define IS_WORD 1

// Processor flags
#define CARRY_FLAG 0
#define PARITY_FLAG 2
#define AUXCARRY_FLAG 4
#define ZERO_FLAG 6
#define SIGN_FLAG 7
#define TRAP_FLAG 8
#define INTERRUPT_ENABLE_FLAG 9
#define DIRECTION_FLAG 10
#define OVERFLOW_FLAG 11

// Registers
#define REGISTER_AL 0b000
#define REGISTER_CL 0b001
#define REGISTER_DL 0b010
#define REGISTER_BL 0b011
#define REGISTER_AH 0b100
#define REGISTER_CH 0b101
#define REGISTER_DH 0b110
#define REGISTER_BH 0b111

#define REGISTER_AX 0b000
#define REGISTER_CX 0b001
#define REGISTER_DX 0b010
#define REGISTER_BX 0b011
#define REGISTER_SP 0b100
#define REGISTER_BP 0b101
#define REGISTER_SI 0b110
#define REGISTER_DI 0b111

// Register names for printing
#define REGISTER_AL_NAME "AL"
#define REGISTER_CL_NAME "CL"
#define REGISTER_DL_NAME "DL"
#define REGISTER_BL_NAME "BL"
#define REGISTER_AH_NAME "AH"
#define REGISTER_CH_NAME "CH"
#define REGISTER_DH_NAME "DH"
#define REGISTER_BH_NAME "BH"

#define REGISTER_AX_NAME "AX"
#define REGISTER_CX_NAME "CX"
#define REGISTER_DX_NAME "DX"
#define REGISTER_BX_NAME "BX"
#define REGISTER_SP_NAME "SP"
#define REGISTER_BP_NAME "BP"
#define REGISTER_SI_NAME "SI"
#define REGISTER_DI_NAME "DI"

// Segment Registers
#define REGISTER_ES 0b00
#define REGISTER_CS 0b01
#define REGISTER_SS 0b10
#define REGISTER_DS 0b11

#define EXTRA_SEGMENT ES()
#define CODE_SEGMENT CS()
#define STACK_SEGMENT SS()
#define DATA_SEGMENT DS()

#define EMPTY_SEGMENT_OVERRIDE 0b111

namespace Cepums {

    class Processor
    {
    public:
        Processor();

        void reset();

        void execute(MemoryManager& memoryManager, IOManager& io);

        // Large pile of instructions
        void ins$HLT();
        void ins$CLC();
        void ins$CMC();
        void ins$STC();
        void ins$CLD();
        void ins$STD();
        void ins$CLI();
        void ins$STI();
        void ins$WAIT();
        void ins$LOCK();
        void ins$SEGMENT(); // ??

        void ins$ADDimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t immediate);
        void ins$ADDimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t immediate);
        void ins$ADDimmediateToRegister(uint8_t destREG, uint8_t value);
        void ins$ADDimmediateToRegister(uint8_t destREG, uint16_t value);
        void ins$ADDmemoryToRegisterWord(MemoryManager& memoryManager, uint8_t regBits, uint16_t segment, uint16_t effectiveAddress);
        void ins$ADDregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t registerValue);
        void ins$ADDregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t registerValue);
        void ins$ADDregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG);
        void ins$ADDregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG);

        void ins$ANDimmediateToRegister(uint8_t destREG, uint8_t value);

        void ins$CALLnear(MemoryManager& memoryManager, int16_t offset);
        void ins$CALLnearFromMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);

        void ins$CMPimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t immediate);
        void ins$CMPimmediateToRegister(uint8_t destREG, uint8_t immediate);
        void ins$CMPimmediateToRegister(uint8_t destREG, uint16_t immediate);
        void ins$CMPregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG);
        void ins$CMPregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t registerValue);

        void ins$DECregister(uint8_t isWordBit, uint8_t REG);
        void ins$DECmemoryByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$DECmemoryWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);

        void ins$INCregister(uint8_t isWordBit, uint8_t REG);
        void ins$INCmemoryByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$INCmemoryWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);

        void ins$INT(MemoryManager& memoryManager, uint16_t immediate);
        void ins$IRET(MemoryManager& memoryManager);

        void ins$JMPinterSegment(uint16_t newCodeSegment, uint16_t newInstructionPointer);
        void ins$JMPshort(int8_t increment);
        void ins$JMPshortWord(int16_t increment);

        void ins$LEStoRegister(MemoryManager& memoryManager, uint8_t destREG, uint16_t segment, uint16_t effectiveAddress);

        void ins$LODSbyte(MemoryManager& memoryManager);
        void ins$LODSword(MemoryManager& memoryManager);
        void ins$LOOP(int8_t offset);

        void ins$MOVimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t immediate);
        void ins$MOVimmediateToRegisterByte(uint8_t reg, uint8_t immediate);
        void ins$MOVimmediateToRegisterWord(uint8_t reg, uint16_t immediate);
        void ins$MOVmemoryToRegisterByte(MemoryManager& memoryManager, uint8_t destREG, uint16_t segment, uint16_t effectiveAddress);
        void ins$MOVmemoryToRegisterWord(MemoryManager& memoryManager, uint8_t destREG, uint16_t segment, uint16_t effectiveAddress);
        void ins$MOVmemoryToSegmentRegisterWord(MemoryManager& memoryManager, uint8_t srBits, uint16_t segment, uint16_t effectiveAddress);
        void ins$MOVregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t registerValue);
        void ins$MOVregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t registerValue);
        void ins$MOVregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG);
        void ins$MOVregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG);
        void ins$MOVregisterToSegmentRegisterWord(uint8_t srBits, uint16_t value);
        void ins$MOVsegmentRegisterToMemoryWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t SEGREG);
        void ins$MOVsegmentRegisterToRegisterWord(uint8_t REG, uint8_t SEGREG);
        void ins$MOVSword(MemoryManager& memoryManager);

        void ins$MULmemoryByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$MULmemoryWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);

        void ins$NOTmemoryWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$NOTregisterWord(uint8_t REG);

        void ins$ORimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t immediate);
        void ins$ORimmediateToRegister(uint8_t destREG, uint8_t immediate);
        void ins$ORregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t registerValue);
        void ins$ORregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG);

        void ins$POPF(MemoryManager& memoryManager);
        void ins$POPsegmentRegister(MemoryManager& memoryManager, uint8_t srBits);
        void ins$POPregisterWord(MemoryManager& memoryManager, uint8_t REG);
        void ins$PUSHF(MemoryManager& memoryManager);
        void ins$PUSHregisterByte(MemoryManager& memoryManager, uint8_t REG);
        void ins$PUSHregisterWord(MemoryManager& memoryManager, uint8_t REG);
        void ins$PUSHsegmentRegister(MemoryManager& memoryManager, uint8_t srBits);

        void ins$RCLmemoryOnceByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$RCLmemoryOnceWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$RCLregisterOnceByte(uint8_t REG);
        void ins$RCLregisterOnceWord(uint8_t REG);
        void ins$RCRmemoryOnceByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$RCRmemoryOnceWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$RCRregisterOnceByte(uint8_t REG);
        void ins$RCRregisterOnceWord(uint8_t REG);
        void ins$REP_STOSword(MemoryManager& memoryManager);
        void ins$RETnear(MemoryManager& memoryManager);
        void ins$ROLmemoryOnceByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$ROLmemoryOnceWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$ROLregisterOnceByte(uint8_t REG);
        void ins$ROLregisterOnceWord(uint8_t REG);
        void ins$RORmemoryOnceByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$RORmemoryOnceWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$RORregisterOnceByte(uint8_t REG);
        void ins$RORregisterOnceWord(uint8_t REG);

        void ins$SALmemoryOnceByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$SALmemoryOnceWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$SALregisterByCLByte(uint8_t rmBits);
        void ins$SALregisterOnceByte(uint8_t rmBits);
        void ins$SALregisterOnceWord(uint8_t rmBits);
        void ins$SARmemoryOnceByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$SARmemoryOnceWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$SARregisterOnceByte(uint8_t rmBits);
        void ins$SARregisterOnceWord(uint8_t rmBits);
        void ins$SHRmemoryOnceByte(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$SHRmemoryOnceWord(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress);
        void ins$SHRregisterOnceByte(uint8_t rmBits);
        void ins$SHRregisterOnceWord(uint8_t rmBits);
        void ins$STOSbyte(MemoryManager& memoryManager);
        void ins$STOSword(MemoryManager& memoryManager);

        void ins$SUBimmediateToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint8_t immediate);
        void ins$SUBimmediateToMemory(MemoryManager& memoryManager, uint16_t effectiveAddress, uint16_t immediate);
        void ins$SUBimmediateToRegister(uint8_t destREG, uint8_t value);
        void ins$SUBimmediateToRegister(uint8_t destREG, uint16_t value);
        void ins$SUBregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t registerValue);
        void ins$SUBregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t registerValue);
        void ins$SUBregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG);
        void ins$SUBregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG);

        void ins$TESTimmediateToRegister(uint8_t destREG, uint8_t value);
        void ins$TESTimmediateToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t immediate);

        void ins$XCHGmemoryToRegisterByte(MemoryManager& memoryManager, uint8_t destREG, uint16_t segment, uint16_t effectiveAddress);
        void ins$XCHGregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG);

        void ins$XORregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint8_t registerValue);
        void ins$XORregisterToMemory(MemoryManager& memoryManager, uint16_t segment, uint16_t effectiveAddress, uint16_t registerValue);
        void ins$XORregisterToRegisterByte(uint8_t destREG, uint8_t sourceREG);
        void ins$XORregisterToRegisterWord(uint8_t destREG, uint8_t sourceREG);

        uint16_t& DS() { return m_dataSegment; }
        uint16_t& CS() { return m_codeSegment; }
        uint16_t& SS() { return m_stackSegment; }
        uint16_t& ES() { return m_extraSegment; }

        // 16-bit General Registers (data)
        uint16_t& AX() { return m_AX; };
        uint16_t& BX() { return m_BX; };
        uint16_t& CX() { return m_CX; };
        uint16_t& DX() { return m_DX; };

        // 8-bit General Register halves
        uint8_t AH() { return HIGHER(m_AX); }
        uint8_t AL() { return LOWER(m_AX); }
        uint8_t BH() { return HIGHER(m_BX); }
        uint8_t BL() { return LOWER(m_BX); }
        uint8_t CH() { return HIGHER(m_CX); }
        uint8_t CL() { return LOWER(m_CX); }
        uint8_t DH() { return HIGHER(m_DX); }
        uint8_t DL() { return LOWER(m_DX); }

        // Setting 8-bit registers
        void AH(uint8_t ah) { SET8BITREGISTERHIGH(m_AX, ah); }
        void AL(uint8_t al) { SET8BITREGISTERLOW(m_AX, al); }
        void BH(uint8_t bh) { SET8BITREGISTERHIGH(m_BX, bh); }
        void BL(uint8_t bl) { SET8BITREGISTERLOW(m_BX, bl); }
        void CH(uint8_t ch) { SET8BITREGISTERHIGH(m_CX, ch); }
        void CL(uint8_t cl) { SET8BITREGISTERLOW(m_CX, cl); }
        void DH(uint8_t dh) { SET8BITREGISTERHIGH(m_DX, dh); }
        void DL(uint8_t dl) { SET8BITREGISTERLOW(m_DX, dl); }

        // Pointer and Index Registers
        uint16_t& SP() { return m_stackPointer; }
        uint16_t& BP() { return m_basePointer; }
        uint16_t& SI() { return m_sourceIndex; }
        uint16_t& DI() { return m_destinationIndex; }
        uint16_t& IP() { return m_instructionPointer; }

        void updateRegisterFromREG8(uint8_t REG, uint8_t data);
        void updateRegisterFromREG16(uint8_t REG, uint16_t data);
        uint8_t getRegisterValueFromREG8(uint8_t REG);
        uint16_t& getRegisterFromREG16(uint8_t REG);
        uint16_t getSegmentRegisterValueAndResetOverride();
        uint16_t getEffectiveAddressFromBits(uint8_t rmBits, uint8_t modBits, uint8_t isWord, uint8_t displacementLow, uint8_t displacementHigh, uint16_t defaultSegment, uint16_t& segment);

        void loadDisplacementsFromInstructionStream(MemoryManager& memoryManager, uint8_t modBits, uint8_t rmBits, uint8_t& displacementLowByte, uint8_t& displacementHighByte);
        void setFlagsAfterLogicalOperation(uint8_t byte);
        void setFlagsAfterLogicalOperation(uint16_t word);
        void setFlagsAfterArithmeticOperation(uint8_t byte);
        void setFlagsAfterArithmeticOperation(uint16_t word);
        const char* getRegisterNameFromREG8(uint8_t REG);
        const char* getRegisterNameFromREG16(uint8_t REG);
        const char* getSegmentRegisterName(uint8_t REG);
    private:
        int m_cyclesToWait = 0;
        int m_currentCycleCounter = 0;

        // TODO: 6-byte instruction queue

        uint8_t m_segmentPrefix = EMPTY_SEGMENT_OVERRIDE;
        uint8_t m_segmentPrefixCounter = 0;

        // Flags Register
        uint16_t m_flags = 0;

        uint16_t m_instructionPointer = 0;

        // Segment Registers
        uint16_t m_dataSegment = 0;
        uint16_t m_codeSegment = 0xFFFF;
        uint16_t m_stackSegment = 0;
        uint16_t m_extraSegment = 0;

        // General Registers (data)
        uint16_t m_AX = 0;
        uint16_t m_BX = 0;
        uint16_t m_CX = 0;
        uint16_t m_DX = 0;

        // General Registers (pointers and indices)
        uint16_t m_stackPointer = 0;
        uint16_t m_basePointer = 0;
        uint16_t m_sourceIndex = 0;
        uint16_t m_destinationIndex = 0;
    };
}
