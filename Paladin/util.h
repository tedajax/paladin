#pragma once

#include <cstdint>

namespace tdjx
{
    namespace util
    {
        uint32_t next_pow2(uint32_t value);
        uint64_t next_pow2(uint64_t value);
        uint32_t ctz(uint32_t value);
        uint32_t clz(uint32_t value);
        uint64_t ctzl(uint64_t value);
        uint64_t clzl(uint64_t value);
    }
}