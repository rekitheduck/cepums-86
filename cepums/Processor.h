#pragma once

namespace Cepums {

    class Processor
    {
    public:
        Processor() {}

        uint16_t DS() { return m_dataSegment; }
        uint16_t CS() { return m_codeSegment; }
        uint16_t SS() { return m_stackSegment; }
        uint16_t ES() { return m_extraSegment; }

    private:
        uint16_t m_dataSegment;
        uint16_t m_codeSegment;
        uint16_t m_stackSegment;
        uint16_t m_extraSegment;
    };
}