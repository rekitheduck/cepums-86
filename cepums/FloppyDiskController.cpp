#include "cepumspch.h"
#include "FloppyDiskController.h"

namespace Cepums {

    uint8_t FloppyDiskController::readStatusRegisterA()
    {
        TODO();
        return uint8_t();
    }

    uint8_t FloppyDiskController::readStatusRegisterB()
    {
        TODO();
        return uint8_t();
    }

    uint8_t FloppyDiskController::readMainStatusRegister()
    {
        /*
        bit 7 = 1  RQM data register is ready
                0  no access is permitted
        bit 6 = 1  transfer is from controller to system
                0  transfer is from system to controller
        bit 5 = 1  non - DMA mode
        bit 4 = 1  diskette controller is busy
        bit 3 = 1  drive 3 busy(reserved on PS / 2)
        bit 2 = 1  drive 2 busy(reserved on PS / 2)
        bit 1 = 1  drive 1 busy(= drive is in seek mode)
        bit 0 = 1  drive 0 busy(= drive is in seek mode)
        */
        // Hmmm
        DC_CORE_TRACE("FDC: read main status register");
        //return 0x40;
        return 0xFF;
    }

    uint8_t FloppyDiskController::readDigitalInputRegister()
    {
        DC_CORE_TRACE("FDC: read digital input register");

        return 10;
    }

    void FloppyDiskController::writeConfigurationControlRegister(uint8_t value)
    {
        // Bits 0 and 1 specify the speed
        DC_CORE_TRACE("FDC: writing configuration control register");

        return;
    }
}
