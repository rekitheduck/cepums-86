#include "cepumspch.h"
#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace Cepums {

    std::shared_ptr<spdlog::logger> Log::s_logger;

    void Log::init()
    {
        spdlog::set_pattern("%^[%T] %v%$");
        s_logger = spdlog::stdout_color_mt("Cepums-86");
        s_logger->set_level(spdlog::level::trace);
    }
}
