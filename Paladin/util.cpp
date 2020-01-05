#include "util.h"

uint32_t tdjx::util::next_pow2(uint32_t value)
{
    return (value == 1) ? 1 : 1 << (32 - clz(value));
}

uint64_t tdjx::util::next_pow2(uint64_t value)
{
    return (value == 1) ? 1 : 1 << (64 - clzl(value));
}

#include <intrin.h>
namespace tdjx
{
    namespace util
    {
        uint32_t ctz(uint32_t value)
        {
            unsigned long trailing_zero = 0;

            if (_BitScanForward(&trailing_zero, value))
            {
                return trailing_zero;
            }
            else
            {
                // This is undefined, I better choose 32 than 0
                return 32;
            }
        }

        uint32_t clz(uint32_t value)
        {
            unsigned long leading_zero = 0;

            if (_BitScanReverse(&leading_zero, value))
            {
                return 31 - leading_zero;
            }
            else
            {
                // Same remarks as above
                return 32;
            }
        }

        uint64_t ctzl(uint64_t value)
        {
            unsigned long trailing_zero = 0;

            if (_BitScanForward64(&trailing_zero, value))
            {
                return trailing_zero;
            }
            else
            {
                // This is undefined, I better choose 64 than 0
                return 64;
            }
        }

        uint64_t clzl(uint64_t value)
        {
            unsigned long leading_zero = 0;

            if (_BitScanReverse64(&leading_zero, value))
            {
                return 63 - leading_zero;
            }
            else
            {
                // Same remarks as above
                return 64;
            }
        }
    }
}

