#include "cepumspch.h"
#include "Register.h"

#include "Processor.h"

namespace Cepums {

    uint8_t Register8::valueByte(Processor* processor, MemoryManager&)
    {
        DC_CORE_ASSERT(processor, "processor ptr");
        return processor->getRegisterValueFromREG8(m_registerBits);
    }

    void Register8::updateByte(Processor* processor, MemoryManager&, uint8_t newValue)
    {
        DC_CORE_ASSERT(processor, "processor ptr");
        processor->updateRegisterFromREG8(m_registerBits, newValue);
    }

    const char* Register8::nameFromREG8(uint8_t REG)
    {
        switch (REG)
        {
        case 0x0:
            return "AL";
        case 0x1:
            return "CL";
        case 0x2:
            return "DL";
        case 0x3:
            return "BL";
        case 0x4:
            return "AH";
        case 0x5:
            return "CH";
        case 0x6:
            return "DH";
        case 0x7:
            return "BH";
        default:
            DC_CORE_ERROR("Malformed REG bits : 0b{0:b}", REG);
            VERIFY_NOT_REACHED();
            return "ERROR";
        }
    }

    const char* Register8::name()
    {
        return Register8::nameFromREG8(m_registerBits);
    }

    uint16_t Register16::valueWord(Processor* processor, MemoryManager&)
    {
        DC_CORE_ASSERT(processor, "processor ptr");
        return processor->getRegisterFromREG16(m_registerBits);
    }

    void Register16::updateWord(Processor* processor, MemoryManager&, uint16_t newValue)
    {
        DC_CORE_ASSERT(processor, "processor ptr");
        processor->updateRegisterFromREG16(m_registerBits, newValue);
    }

    const char* Register16::nameFromREG16(uint8_t REG)
    {
        switch (REG)
        {
        case 0x0:
            return "AX";
        case 0x1:
            return "CX";
        case 0x2:
            return "DX";
        case 0x3:
            return "BX";
        case 0x4:
            return "SP";
        case 0x5:
            return "BP";
        case 0x6:
            return "SI";
        case 0x7:
            return "DI";
        default:
            DC_CORE_ERROR("Malformed REG bits : 0b{0:b}", REG);
            VERIFY_NOT_REACHED();
            return "ERROR";
        }
    }

    const char* Register16::name()
    {
        return Register16::nameFromREG16(m_registerBits);
    }
}
