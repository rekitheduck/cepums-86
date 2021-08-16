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
        auto hopefully_an_instruction = memoryManager.readWord(m_codeSegment, m_instructionPointer);

        DC_CORE_INFO("CPU will now execute 0x{0:x}", hopefully_an_instruction);

        switch (hopefully_an_instruction)
        {
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
}
