#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "Core.h"

// Engine log macros
#define DC_CORE_TRACE(...)      ::Cepums::Log::getLogger()->trace(__VA_ARGS__)
#define DC_CORE_INFO(...)       ::Cepums::Log::getLogger()->info(__VA_ARGS__)
#define DC_CORE_WARN(...)       ::Cepums::Log::getLogger()->warn(__VA_ARGS__)
#define DC_CORE_ERROR(...)      ::Cepums::Log::getLogger()->error(__VA_ARGS__)
#define DC_CORE_CRITICAL(...)   ::Cepums::Log::getLogger()->critical(__VA_ARGS__)

namespace Cepums {

    class Log
    {
    public:
        static void init();
        inline static std::shared_ptr<spdlog::logger>& getLogger() { if (!s_logger) { abort(); }; return s_logger; }
    private:
        static std::shared_ptr<spdlog::logger> s_logger;
    };
}
