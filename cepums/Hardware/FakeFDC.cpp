#include "cepumspch.h"
#include "FakeFDC.h"

namespace Cepums {

	void FakeFDC::execute(MemoryManager& mm)
	{
		switch (m_command)
		{
		case 0:
			return;
		case 2:
			break;
		case 3:
		case 4:
			TODO();
			break;
		default:
			VERIFY_NOT_REACHED();
			break;
		}

		DC_CORE_TRACE("[FakeFDC] Reading {0} sectors from {1:X}:{2:X} to {3:X}::{4:X}", m_sectorCount, m_startCylinder, m_startSector, m_upperAddress, m_lowerAddress);
		DC_CORE_ASSERT(m_startCylinder < 80, "There are 80 cylinders");
		DC_CORE_ASSERT(m_startSector < 18, "There are 18 sectors per cylinder")

		if (m_disk1.empty()) {
			DC_CORE_TRACE("[FakeFDC] Loading disk 1 ...");
			m_disk1.resize(1440 * 1024); // 1.44 MB

			std::ifstream diskBinary;

			auto disk1 = "Disk1.img";

			diskBinary.open(disk1, std::ios::binary);
			if (diskBinary.is_open())
			{
				diskBinary.read((char*)m_disk1.data(), 1440 * 1024);
				diskBinary.close();
			}
			else {
				DC_CORE_ERROR("[FakeFDC] Disk1.img not present !!!");
				VERIFY_NOT_REACHED();
			}
		}

		uint16_t lowerAddressCounting = m_lowerAddress;

		for (auto i = 0; i < m_sectorCount; i++)
		{
			auto headOffset = m_head * 80 * 18 * 512;
			auto cylinderOffset = m_startCylinder * 18 * 512;
			auto sectorOffset = (m_startSector + i) * 512;
			for (size_t j = 0; j < 512; j++)
			{
				auto value = m_disk1[headOffset + cylinderOffset + sectorOffset + j];
				mm.writeByte(m_upperAddress, lowerAddressCounting, value);
				lowerAddressCounting++;
			}
		}
		m_command = 0;
	}
}
