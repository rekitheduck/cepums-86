#pragma once

#include <memory>

#ifdef _WIN32
#ifdef CEPUMS_DEBUG
#define DC_CORE_ASSERT(x, ...) { if(!(x)) { DC_CORE_CRITICAL("Assertation failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define DC_CORE_ASSERT(x, ...)
#endif
#define TODO() { DC_CORE_CRITICAL("TODO hit in {0}:{1}", __FILE__, __LINE__); __debugbreak(); }
#else
// TODO: figure out how to do debugging on gdb
#define DC_CORE_ASSERT(x, ...) { if(!(x)) { DC_CORE_CRITICAL("Assertation failed: {0} in {1} at {2}", __VA_ARGS__, __FILE__, __LINE__); abort(); } }
#define TODO() { DC_CORE_CRITICAL("TODO hit in {0}:{1}", __FILE__, __LINE__); abort(); }
#endif

#define ILLEGAL_INSTRUCTION() { DC_CORE_CRITICAL("ILLEGAL INSTRUCTION REACHED in {0}:{1}", __FILE__, __LINE__); __debugbreak(); }
#define UNKNOWN_INSTRUCTION() { DC_CORE_CRITICAL("Unknown instruction parsed in {0}:{1}", __FILE__, __LINE__); __debugbreak(); }

#define BIT(x) (1 << x)

namespace Cepums {

    // Unique pointers aka Scoped pointers
    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T, typename ... Args>
    constexpr Scope<T> createScope(Args&& ... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    // Shared pointers aka References
    template<typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T, typename ... Args>
    constexpr Ref<T> createRef(Args&& ... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}
