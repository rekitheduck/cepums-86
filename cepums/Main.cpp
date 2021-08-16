#include "cepumspch.h"

#include "Log.h"
#include "Processor.h"

int main()
{
    // Initialize the basics
    Cepums::Log::init();
    DC_CORE_INFO("Cepums-86 starting up ...");

    // Make a processor
    Cepums::Processor processor;

    return 0;
}