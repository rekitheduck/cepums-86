#pragma once

#include <string>

#include "Operand.h"

namespace Cepums {

    class Immediate8 : public Operand
    {
    public:
        Immediate8(uint8_t immediate)
        {
            m_immediate = immediate;
            std::stringstream ss;
            ss << std::hex << static_cast<unsigned int>(immediate) << "h";
            m_name = ss.str();
        }

        virtual uint8_t valueByte(Processor*, MemoryManager&) override { return m_immediate; }
        virtual void updateByte(Processor*, MemoryManager&, uint8_t newValue) override { VERIFY_NOT_REACHED(); }

        virtual const char* name() override { return m_name.c_str(); }
        virtual OperandType type() override { return OperandType::Immediate; }
        virtual OperandSize size() override { return OperandSize::Byte; }
    private:
        uint8_t m_immediate;
        std::string m_name;
    };

    class Immediate16 : public Operand
    {
    public:
        Immediate16(uint16_t immediate) 
        {
            m_immediate = immediate;
            std::stringstream ss;
            ss << std::hex << static_cast<unsigned int>(immediate) << "h";
            m_name = ss.str();
        }

        virtual uint16_t valueWord(Processor*, MemoryManager&) override { return m_immediate; }
        virtual void updateWord(Processor*, MemoryManager&, uint16_t newValue) override { VERIFY_NOT_REACHED(); }

        virtual const char* name() override { return m_name.c_str(); }
        virtual OperandType type() override { return OperandType::Immediate; }
        virtual OperandSize size() override { return OperandSize::Word; }
    private:
        uint16_t m_immediate;
        std::string m_name;
    };
}
