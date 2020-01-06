#include "util.h"

uint32_t tdjx::util::next_pow2(uint32_t value)
{
    return (value == 1) ? 1 : 1 << (32 - clz(value));
}

uint64_t tdjx::util::next_pow2(uint64_t value)
{
    return (value == 1) ? 1 : 1 << (64 - clzl(value));
}

#ifdef _WIN32
#include <intrin.h>
inline uint32_t tdjx::util::ctz(uint32_t value)
{
    unsigned long tz = 0;

    if (_BitScanForward(&tz, value))
    {
        return tz;
    }
    else
    {
        // Undefined, but this produces less ambiguous output than 0.
        return 32;
    }
}

inline uint32_t tdjx::util::clz(uint32_t value)
{
    unsigned long lz = 0;

    if (_BitScanReverse(&lz, value))
    {
        return 31 - lz;
    }
    else
    {
        // Undefined, but this produces less ambiguous output than 0.
        return 32;
    }
}

inline uint64_t tdjx::util::ctzl(uint64_t value)
{
    unsigned long tz = 0;

    if (_BitScanForward64(&tz, value))
    {
        return tz;
    }
    else
    {
        // Undefined, but this produces less ambiguous output than 0.
        return 64;
    }
}

inline uint64_t tdjx::util::clzl(uint64_t value)
{
    unsigned long lz = 0;

    if (_BitScanReverse64(&lz, value))
    {
        return 63 - lz;
    }
    else
    {
        // Undefined, but this produces less ambiguous output than 0.
        return 64;
    }
}
#endif