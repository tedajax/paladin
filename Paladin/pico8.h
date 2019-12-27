#pragma once

#include "types.h"

#ifdef _DEBUG
#define FIXED16_DEBUG
#endif

namespace pico8
{
    struct fixed16
    {
        static const int k_bitsOfPrecision = 16;
        static const int k_wholeBitCount = 32 - k_bitsOfPrecision;
        static const uint32 k_precisionBit = (1 << k_bitsOfPrecision);
        static const uint32 k_precisionMask = k_precisionBit - 1;
        static const uint32 k_wholeMask = ~k_precisionMask;
        static const uint32 k_negativeBit = 31;
        static const uint32 k_negativeMask = (1 << k_negativeBit);

        fixed16(int v)
        {
            set_raw(static_cast<int32>(v) << k_bitsOfPrecision);
        }

        fixed16(uint8 v)
        {
            set_raw(static_cast<int32>(v) << k_bitsOfPrecision);
        }

        fixed16(float32 v)
        {
            set_raw(static_cast<int32>(v * k_precisionBit));
        }

        fixed16(const fixed16& v) { set_raw(v.m_value); }

        operator float32() const
        {
            return static_cast<int32>(m_value) / static_cast<float32>(1 << k_bitsOfPrecision);
        }

        operator int() const
        {
            return static_cast<int>(static_cast<int16>(*this));
        }

        operator int16() const
        {
            return static_cast<int16>(m_value >> k_bitsOfPrecision);
        }

        operator uint8() const
        {
            return static_cast<uint8>((m_value >> k_bitsOfPrecision) & 0xFF);
        }

        inline fixed16& operator=(const fixed16& other)
        {
            set_raw(other.m_value);
            return *this;
        }

        inline fixed16& operator+=(const fixed16& other)
        {
            set_raw(m_value + other.m_value);
            return *this;
        }

        inline fixed16& operator-=(const fixed16& other)
        {
            set_raw(m_value - other.m_value);
            return *this;
        }

        inline fixed16& operator*=(const fixed16& other)
        {
            set_raw(static_cast<uint32>((static_cast<int64>(m_value) * static_cast<int64>(other.m_value)) >> k_bitsOfPrecision));
            return *this;
        }

        inline fixed16& operator*=(const float32& other)
        {
            *this *= static_cast<fixed16>(other);
            return *this;
        }

        inline fixed16& operator/=(const fixed16& other)
        {
            set_raw((m_value << k_bitsOfPrecision) / other.m_value);
            return *this;
        }

        inline float32 wholef() const
        {
            return std::floor(static_cast<float32>(*this));
        }

        inline float32 fracf() const
        {
            float32 f = static_cast<float32>(*this);
            return f - std::floor(f);
        }

        inline int32 raw() const
        {
            return m_value;
        }

        inline void set_raw(int32 value)
        {
            m_value = value;
#ifdef FIXED16_DEBUG
            m_floatValue = static_cast<float32>(*this);
#endif
        }

#ifdef FIXED16_DEBUG
        float32 m_floatValue;
#endif
        int32 m_value;
    };

    namespace literals
    {
        fixed16 operator"" _fx16(unsigned long long v);
        fixed16 operator"" _fx16(long double v);
    }

    using namespace literals;

    inline fixed16 operator+(const fixed16& lhs, const fixed16& rhs)
    {
        fixed16 ret = lhs;
        ret += rhs;
        return ret;
    }

    inline fixed16 operator-(const fixed16& lhs, const fixed16& rhs)
    {
        fixed16 ret = lhs;
        ret -= rhs;
        return ret;
    }

    inline fixed16 operator*(const fixed16& lhs, const fixed16& rhs)
    {
        fixed16 ret = lhs;
        ret *= rhs;
        return ret;
    }

    inline fixed16 operator/(const fixed16& lhs, const fixed16& rhs)
    {
        fixed16 ret = lhs;
        ret /= rhs;
        return ret;
    }

    // unary negation
    inline fixed16 operator-(const fixed16& v)
    {
        float32 f = static_cast<float32>(v);
        return fixed16(-f);
    }

    inline bool operator==(const fixed16& lhs, const fixed16& rhs)
    {
        return lhs.m_value == rhs.m_value;
    }

    inline bool operator!=(const fixed16& lhs, const fixed16& rhs)
    {
        return lhs.m_value != rhs.m_value;
    }

    inline bool operator<(const fixed16& lhs, const fixed16& rhs)
    {
        return lhs.m_value < rhs.m_value;
    }

    inline bool operator>(const fixed16& lhs, const fixed16& rhs)
    {
        return lhs.m_value > rhs.m_value;
    }

    inline bool operator<=(const fixed16& lhs, const fixed16& rhs)
    {
        return lhs.m_value <= rhs.m_value;
    }
    inline bool operator>=(const fixed16& lhs, const fixed16& rhs)
    {
        return lhs.m_value >= rhs.m_value;
    }

    void init();
    void shutdown();

    inline fixed16 min(fixed16 a, fixed16 b)
    {
        return (a.m_value < b.m_value) ? a : b;
    }

    inline fixed16 max(fixed16 a, fixed16 b)
    {
        return (a.m_value > b.m_value) ? a : b;
    }

    inline fixed16 mid(fixed16 a, fixed16 b, fixed16 c)
    {
        fixed16 data[3] = { a, b, c };

        // insertion sort but unrolled for 3 items
        if (data[0] > data[1])
        {
            std::swap(data[1], data[0]);
        }
        for (int j = 2; j > 0 && data[j - 1] > data[j]; --j)
        {
            std::swap(data[j], data[j - 1]);
        }

        return data[1];
    }

    const float32 k_tau = static_cast<float32>(M_PI) * 2.f;

    const size_t k_memorySize = 0x8000;
    const size_t k_offsetSpriteSheet = 0x0000;
    const size_t k_offsetSharedSpriteMap = 0x1000;
    const size_t k_offsetMap = 0x2000;
    const size_t k_offsetSpriteFlags = 0x3000;
    const size_t k_offsetMusic = 0x3100;
    const size_t k_offsetSfx = 0x3200;
    const size_t k_offsetGeneral = 0x4300;
    const size_t k_offsetPersistentData = 0x5e00;
    const size_t k_offsetDrawState = 0x5f00;
    const size_t k_offsetHardwareState = 0x5f40;
    const size_t k_offsetGpioPins = 0x5f80;
    const size_t k_offsetScreenData = 0x6000;

    const size_t k_screenSize = 0x2000;

    inline fixed16 sin(fixed16 t)
    {
        return static_cast<fixed16>(std::sinf(static_cast<float32>(t) * k_tau));
    }

    inline fixed16 cos(fixed16 t)
    {
        return static_cast<fixed16>(std::cosf(static_cast<float32>(t) * k_tau));
    }

    inline fixed16 atan2(fixed16 x, fixed16 y)
    {
        return static_cast<fixed16>(std::atan2f(y, x));
    }

    void update(float32 time);
    void flip();
    fixed16 time();
    void srand(fixed16 seed);
    fixed16 rnd(fixed16 r);

    void clip();
    void clip(fixed16 x, fixed16 y, fixed16 w, fixed16 h);

    fixed16 pget(fixed16 x, fixed16 y);

    void cls(uint8 c);

    void pset(fixed16 x, fixed16 y, fixed16 c);
    void line(fixed16 x0, fixed16 y0, fixed16 x1, fixed16 y1, fixed16 c);
    void rect(fixed16 x, fixed16 y, fixed16 w, fixed16 h, fixed16 c);
    void rectfill(fixed16 x, fixed16 y, fixed16 w, fixed16 h, fixed16 c);
    void circ(fixed16 x, fixed16 y, fixed16 r, fixed16 c);
    void circfill(fixed16 x, fixed16 y, fixed16 r, fixed16 c);
}
