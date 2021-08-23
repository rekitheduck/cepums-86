#include "cepumspch.h"

#include "IOManager.h"
#include "Log.h"
#include "MemoryManager.h"
#include "Processor.h"

int main()
{
    // Initialize the basics
    Cepums::Log::init();
    DC_CORE_INFO("Cepums-86 starting up ...");

    // Make a processor
    Cepums::Processor processor;

    Cepums::MemoryManager memoryManager;
    Cepums::IOManager ioManager;

#if 0
    uint16_t test_segment = 0x06EF;
    uint16_t test_offset = 0x1234;
    uint32_t expected_result = 0x08124;

    DC_CORE_WARN("Test01: Getting a physical address from a segment&offset pair");
    DC_CORE_TRACE("Segment  : 0x{0:x}", test_segment);
    DC_CORE_TRACE("Offset   : 0x{0:x}", test_offset);
    DC_CORE_TRACE("Expected : 0x{0:x}", expected_result);

    auto test_result = memoryManager.addresstoPhysical(test_segment, test_offset);
    if (test_result == expected_result)
    {
        DC_CORE_INFO("Result == Expected");
    }
    else
    {
        DC_CORE_ERROR("Result != Expected. Got 0x{0:x} instead", test_result);
    }
#endif

    bool shouldExecute = true;

    while (shouldExecute)
        processor.execute(memoryManager, ioManager);

    return 0;
}
