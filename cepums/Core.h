#pragma once

#include <memory>

#ifdef _WIN32
#ifdef CEPUMS_DEBUG
#define DC_CORE_ASSERT(x, ...) { if(!(x)) { DC_CORE_CRITICAL("Assertation failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
#define DC_CORE_ASSERT(x, ...)
#endif
#define TODO() { DC_CORE_CRITICAL("TODO hit in {0}:{1}", __FILE__, __LINE__); __debugbreak(); }
#define ILLEGAL_INSTRUCTION() { DC_CORE_CRITICAL("ILLEGAL INSTRUCTION REACHED in {0}:{1}", __FILE__, __LINE__); __debugbreak(); }
#define UNKNOWN_INSTRUCTION() { DC_CORE_CRITICAL("Unknown instruction parsed in {0}:{1}", __FILE__, __LINE__); __debugbreak(); }
#define VERIFY_NOT_REACHED() DC_CORE_CRITICAL("Verify not reached hit in {0}:{1}!", __FILE__, __LINE__); __debugbreak()
#else
// TODO: figure out how to do debugging on gdb
#define DC_CORE_ASSERT(x, ...) { if(!(x)) { DC_CORE_CRITICAL("Assertation failed: {0} in {1} at {2}", __VA_ARGS__, __FILE__, __LINE__); abort(); } }
#define ILLEGAL_INSTRUCTION() { DC_CORE_CRITICAL("ILLEGAL INSTRUCTION REACHED in {0}:{1}", __FILE__, __LINE__); abort(); }
#define UNKNOWN_INSTRUCTION() { DC_CORE_CRITICAL("Unknown instruction parsed in {0}:{1}", __FILE__, __LINE__); abort(); }
#define VERIFY_NOT_REACHED() DC_CORE_CRITICAL("Verify not reached hit in {0}:{1}!", __FILE__, __LINE__); abort()
#define TODO() { DC_CORE_CRITICAL("TODO hit in {0}:{1}", __FILE__, __LINE__); abort(); }
#endif

#define INSTRUCTION_TRACE(...) if(s_debugSpam) DC_CORE_WARN(__VA_ARGS__)
//#define INSTRUCTION_TRACE(...)

#define BIT(x) (1 << x)
#define IS_BIT_SET(number, bit) ((number >> bit) & 1U)
#define IS_BIT_NOT_SET(number, bit) !(IS_BIT_SET(number, bit))
#define DO_PARITY_BYTE(byte) byte ^= byte >> 4; byte ^= byte >> 2; byte ^= byte >> 1
#define DO_PARITY_WORD(byte) byte ^= byte >> 8; byte ^= byte >> 4; byte ^= byte >> 2; byte ^= byte >> 1
#define IS_PARITY_EVEN(byteOrWord) (~byteOrWord) & 1
#define SET_BIT(byte, bit) byte |= BIT(bit)


namespace Cepums {

    // I don't know how this works
    static uint16_t signExtendByteToWord(uint8_t byte)
    {
        uint16_t temp = 1u << (8 - 1);
        return (byte ^ temp) - temp;
    }

    static char halfByteToHexChar(uint8_t halfbyte)
    {
        if (halfbyte < 0xA)
            return halfbyte + 48;
        else
            return halfbyte - 10 + 65;
    }

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
