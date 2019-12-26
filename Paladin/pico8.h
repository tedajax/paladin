#pragma once

#include "types.h"

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
            m_value = static_cast<uint32>(v) << k_bitsOfPrecision;
        }

        fixed16(float32 v)
        {
            m_value = static_cast<uint32>(v * k_precisionBit);
        }

        fixed16(uint32 v) : m_value(v) {}
        fixed16(const fixed16& v) : m_value(v.m_value) {}

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

        inline fixed16& operator=(const fixed16& other)
        {
            m_value = other.m_value;
            return *this;
        }

        inline fixed16& operator+=(const fixed16& other)
        {
            m_value = m_value + other.m_value;
            return *this;
        }

        inline fixed16& operator-=(const fixed16& other)
        {
            m_value = m_value - other.m_value;
            return *this;
        }

        inline fixed16& operator*=(const fixed16& other)
        {
            m_value = static_cast<uint32>((static_cast<int64>(m_value) * static_cast<int64>(other.m_value)) >> k_bitsOfPrecision);
            return *this;
        }

        inline fixed16& operator/=(const fixed16& other)
        {
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

        inline uint32 raw() const
        {
            return m_value;
        }

        uint32 m_value;
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
        return (a < b) ? a : b;
    }

    inline fixed16 max(fixed16 a, fixed16 b)
    {
        return (a > b) ? a : b;
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

    void update(float32 dt);
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
    //void rect(int x, int y, int w, int h, uint8 c);
    //void rectfill(int x, int y, int w, int h, uint8 c);
    //void circ(int x, int y, int r, uint8 c);
    //void circfill(int x, int y, int r, uint8 c);
}
