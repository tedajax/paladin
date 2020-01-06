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

        template <typename t_type>
        inline bool is_pow2(t_type value)
        {
            return (value != 0) ? (value & (value - 1)) == 0 : false;
        }
        
        template <typename t_type>
        inline t_type next_or_equal_pow2(t_type value)
        {
            return (is_pow2(value)) ? value : next_pow2(value);
        }
    }
}