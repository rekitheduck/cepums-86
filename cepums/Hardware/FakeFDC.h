#pragma once

#include "MemoryManager.h"

namespace Cepums {

    class FakeFDC
    {
    public:
        void execute(MemoryManager& mm);

        void setCommand(uint8_t command) { m_command = command; }
        void setSectorCount(uint8_t sectorCount) { m_sectorCount = sectorCount; }

        void setUpperAddress(uint16_t address) { m_upperAddress = address; }
        void setLowerAddress(uint16_t address) { m_lowerAddress = address; }

        void setStartCylinder(uint8_t cylinder) { m_startCylinder = cylinder; }
        void setStartSector(uint8_t sector) { m_startSector = sector - 1; }
    private:
        uint8_t m_command = 0;
        uint8_t m_sectorCount = 0;

        uint16_t m_upperAddress = 0;
        uint16_t m_lowerAddress = 0;

        uint8_t m_startCylinder = 0;
        uint8_t m_startSector = 0;

        std::vector<uint8_t> m_disk1;
    };
}
