#pragma once

#include "types.h"
#include <limits>
#include <glm/vec3.hpp>
#include <cmath>

struct b2Vec2;

class vec2 {
public:
    vec2() : x(0), y(0) { }
    vec2(const vec2& other) : x(other.x), y(other.y) {}
    vec2(const b2Vec2& b2vec);
    vec2(vec2&& other) noexcept : x(other.x), y(other.y) {}
    vec2(float32 x, float32 y) : x(x), y(y) { }
    vec2(int x, int y) : x((float32)x), y((float32)y) { }

    operator b2Vec2() const;
    operator glm::vec3() const;

    inline vec2 operator=(const vec2& other) {
        x = other.x;
        y = other.y;
        return *this;
    }

    inline vec2 operator=(vec2&& other) noexcept {
        x = other.x;
        y = other.y;
        return *this;
    }

    inline vec2& operator+=(const vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    inline vec2& operator-=(const vec2& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    inline vec2& operator*=(float32 scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    inline vec2& operator/=(float32 scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    inline vec2& operator/=(int scalar) {
        x /= (float32)scalar;
        y /= (float32)scalar;
        return *this;
    }

    inline float32 len() const{
        return std::sqrt(x *x + y * y);
    }

    inline float32 len2() const {
        return x * x + y * y;
    }

    inline float32 dot(const vec2& other) const {
        return x * other.x + y * other.y;
    }

    void decompose(vec2& direction, float32& magnitude) {
        magnitude = len();
        if (magnitude > 0) {
            direction = vec2(x / magnitude, y / magnitude);
        }
        else {
            direction = vec2::ZERO;
        }
    }

    vec2 normalize() {
        float32 l = len();
        if (l > 0) {
            *this /= l;
        }
        return *this;
    }

    static vec2 normalize(const vec2& v) {
        float32 l = v.len();
        if (l > 0) {
            vec2 ret(v.x / l, v.y / l);
            return ret;
        }
        else {
            return ZERO;
        }
    }

    static vec2 perpendicular(const vec2& v) {
        return vec2(-v.y, v.x);
    }

    static float32 dot(const vec2& a, const vec2& b) {
        return a.x * b.x + a.y * b.y;
    }

    static float32 dist(const vec2& a, const vec2& b) {
        return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
    }

    static vec2 left(const vec2& a, const vec2& b) {
        return (a.x <= b.x) ? a : b;
    }

    static vec2 right(const vec2& a, const vec2& b) {
        return (a.x >= b.x) ? a : b;
    }

    static vec2 top(const vec2& a, const vec2& b) {
        return (a.y <= b.y) ? a : b;
    }

    static vec2 bottom(const vec2& a, const vec2& b) {
        return (a.y >= b.y) ? a : b;
    }

    static vec2 clamp_to_segment(const vec2& v, const vec2& a, const vec2& b) {
        // special rules for vertical line
        if (a.x - b.x == 0) {
            vec2 t = top(a, b), b = bottom(a, b);
            if (v.y < t.y) {
                return t;
            }
            else if (v.y > b.y) {
                return b;
            }
        }
        else {
            vec2 l = left(a, b), r = right(a, b);
            if (v.x < l.x) {
                return l;
            }
            else if (v.x > r.x) {
                return r;
            }
        }

        return v;
    }

    // if v is on segment between a-b then return v, otherwise return other
    static vec2 on_segment_or_other(const vec2& v, const vec2& a, const vec2& b, const vec2& other) {
        // special rules for vertical line
        if (a.x - b.x == 0) {
            vec2 t = top(a, b), b = bottom(a, b);
            if (v.y < t.y) {
                return other;
            }
            else if (v.y > b.y) {
                return other;
            }
        }
        else {
            vec2 l = left(a, b), r = right(a, b);
            if (v.x < l.x) {
                return other;
            }
            else if (v.x > r.x) {
                return other;
            }
        }

        return v;
    }

    static float32 angle_between(const vec2& a, const vec2& b);

    void limit(float32 mag) {
        float32 l = len();
        if (l > mag) {
            float32 s = (mag / l);
            x *= s;
            y *= s;
        }
    }

    static vec2 ZERO;
    static vec2 ONE;
    static vec2 RIGHT;
    static vec2 UP;

    float32 x, y;
};

inline bool operator==(const vec2& lhs, const vec2& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

inline bool operator!=(const vec2& lhs, const vec2& rhs) {
    return !(lhs == rhs);
}

inline vec2 operator+(const vec2& lhs, const vec2& rhs) {
    vec2 ret(lhs.x + rhs.x, lhs.y + rhs.y);
    return ret;
}

inline vec2 operator-(const vec2& lhs, const vec2& rhs) {
    vec2 ret(lhs.x - rhs.x, lhs.y - rhs.y);
    return ret;
}

inline vec2 operator*(const vec2& lhs, float32 scalar) {
    vec2 ret(lhs.x * scalar, lhs.y * scalar);
    return ret;
}

inline vec2 operator*(float32 scalar, const vec2& rhs) {
    return rhs * scalar;
}

inline vec2 operator/(const vec2& lhs, float32 scalar) {
    vec2 ret(lhs.x / scalar, lhs.y / scalar);
    return ret;
}

inline vec2 operator-(const vec2& v) {
    vec2 ret(-v.x, -v.y);
    return ret;
}

namespace math {
    const float32 PI = 3.1415927410125732421875;
    const float32 TWO_PI = PI * 2;
    const float32 PI_OVER_2 = PI / 2;

    const float32 RAD_TO_DEG = 180.f / PI;
    const float32 DEG_TO_RAD = PI / 180.f;

    const float32 EPSILON = std::numeric_limits<float32>::epsilon();

    inline static float32 abs(float32 v) {
        return std::fabsf(v);
    }

    inline float32 cos(float32 degrees) {
        return std::cosf(degrees * DEG_TO_RAD);
    }

    inline float32 sin(float32 degrees) {
        return std::sinf(degrees * DEG_TO_RAD);
    }

    inline float32 tan(float32 degrees) {
        return std::tanf(degrees * DEG_TO_RAD);
    }

    inline float32 acos(float32 v) {
        return std::acosf(v) * RAD_TO_DEG;
    }

    inline float32 asin(float32 v) {
        return std::asinf(v) * RAD_TO_DEG;
    }

    inline float32 atan(float32 v) {
        return std::atan(v) * RAD_TO_DEG;
    }

    inline float32 atan2(float32 dy, float32 dx) {
        return std::atan2f(dy, dx) * RAD_TO_DEG;
    }

    inline float32 min(float32 a, float32 b) {
        return (a < b) ? a : b;
    }

    inline float32 max(float32 a, float32 b) {
        return (a > b) ? a : b;
    }

    inline float32 sign(float32 v) {
        return (v >= 0) ? 1.f : -1.f;
    }

    inline float32 pow(float32 f, float32 p) {
        return std::powf(f, p);
    }

    inline float32 exp(float32 p) {
        return std::expf(p);
    }

    inline float32 log(float32 f) {
        return std::logf(f);
    }

    inline float32 log(float32 f, float32 b) {
        return std::logf(f) / std::logf(b);
    }

    inline float32 log10(float32 f) {
        return std::log10f(f);
    }

    inline float32 log2(float32 f) {
        return std::log2f(f);
    }

    inline float32 floor(float32 v) {
        return std::floorf(v);
    }

    inline float32 ceil(float32 v) {
        return std::ceilf(v);
    }

    inline float32 round(float32 v) {
        return std::roundf(v);
    }

    inline float32 sqrt(float32 v) {
        return std::sqrtf(v);
    }

    inline int floor_int(float32 v) {
        return (int)floor(v);
    }

    inline int ceil_int(float32 v) {
        return (int)ceil(v);
    }

    inline int round_int(float32 v) {
        return (int)round(v);
    }
    
    inline float32 clamp(float32 v, float32 min, float32 max) {
        if (v < min) {
            return min;
        }
        else if (v > max) {
            return max;
        }
        else {
            return v;
        }
    }

    inline float32 clamp01(float32 v) {
        return clamp(v, 0, 1);
    }

    inline static bool approx(float32 a, float32 b) {
        return abs(b - a) * max(0.000001f * max(abs(a), abs(b)), EPSILON * 8);
    }

    inline static bool approx_zero(float32 v) {
        return approx(v, 0);
    }
    
    inline float32 repeat(float32 v, float32 max) {
        return clamp(v - floor(v / max) * max, 0.f, max);
    }

    inline float32 ping_pong(float32 v, float32 len) {
        v = repeat(v, len * 2.f);
        return len - abs(v - len);
    }

    inline vec2 vec2_from_angle(float32 degrees) {
        return vec2(cos(degrees), sin(degrees));
    }

    inline float32 angle_from_vec2(const vec2& normVec) {
        return repeat(atan2(normVec.y, normVec.x), 360.f);
    }

    inline float32 lerp(float32 a, float32 b, float32 t) {
        return a + (b - a) * t;
    }
    inline float32 lerp_angle(float32 a, float32 b, float32 t) {
        float32 delta = repeat(b - a, 360.f);
        if (delta > 180.f) {
            delta -= 360.f;
        }
        return a + delta * t;
    }

    // determines scalar value between [0-1] of v between a and b
    inline float32 inv_lerp(float32 a, float32 b, float32 v) {
        if (a != b) {
            return clamp01((v - a) / (b - a));
        }
        return 0.f;
    }

    inline float32 delta_angle(float32 a, float32 b) {
        float32 delta = repeat(b - a, 360.f);
        if (delta > 180.f) {
            delta -= 360.f;
        }
        return delta;
    }

    inline float32 move_to(float32 from, float32 to, float32 delta) {
        if (abs(to - from) <= delta) {
            return to;
        }
        return from + sign(to - from) * delta;
    }

    inline float32 rotate_to(float32 from, float32 to, float32 delta) {
        float32 da = delta_angle(from, to);
        if (-delta < da && da < delta) {
            return to;
        }
        return move_to(from, from + da, delta);
    }

    inline float32 smooth_step(float32 from, float32 to, float32 t) {
        t = clamp01(t);
        t = -2.f * t * t * t + 3.f * t * t;
        return to * t + from * (1.f - t);
    }

    inline float32 fade(float32 t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    inline float32 grad(int hash, float32 x, float32 y, float32 z) {
        int h = hash & 15;
        float32 u = (h < 8) ? x : y;
        float32 v = (h < 4) ? y : h == 12 || h == 14 ? x : z;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }

    inline float32 smooth_damp(float32 from, float32 to, float32& velocity, float32 time, float32 maxSpeed = std::numeric_limits<float32>::infinity(), float32 dt = 1.f / 60.f) {
        // from Unity math reference implementation
        // https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Mathf.cs
        // originally from Game Programming Gems 4 Chapter 1.10
        
        time = max(0.0001f, time);
        float32 omega = 2.f / time;

        float32 x = omega * dt;
        float32 exp = 1.f / (1.f + x + 0.48f * x * x + 0.235f * x * x * x);
        float32 delta = to - from;
        float32 origTo = to;

        float32 maxDelta = maxSpeed * time;
        delta = clamp(delta, -maxDelta, maxDelta);
        to = from - delta;

        float32 temp = (velocity + omega * delta) * dt;
        velocity = (velocity - omega * temp) * exp;
        float32 ret = to + (delta + temp) * exp;

        // prevent overshoots
        if (origTo - from > 0.0f == ret > origTo) {
            ret = origTo;
            velocity = (ret - origTo) / dt;
        }

        return ret;
    }

    inline float32 smooth_damp_angle(float32 from, float32 to, float32& velocity, float32 time, float32 maxSpeed = std::numeric_limits<float32>::infinity(), float32 dt = 1.f / 60.f) {
        float32 target = from + delta_angle(from, to);
        return smooth_damp(from, target, velocity, time, maxSpeed, dt);
    }
}

struct aabb {
    vec2 botLeft;
    vec2 topRight;

    inline float32 left() const { return botLeft.x; }
    inline float32 right() const { return topRight.x; }
    inline float32 top() const { return topRight.y; }
    inline float32 bottom() const { return botLeft.y; }
    inline vec2 center() const { return (botLeft + topRight) / 2; }
    inline vec2 dimensions() const { return vec2(math::abs(topRight.x - botLeft.x), math::abs(topRight.y - botLeft.y)); }

    inline bool contains(vec2 pt) {
        return pt.x >= left() && pt.x <= right() && pt.y >= bottom() && pt.y <= top();
    }

    inline void move(vec2 amount) {
        botLeft += amount;
        topRight += amount;
    }

    inline static float32 distance(const aabb& a, const aabb& b) {
        float32 dy1 = math::max(a.bottom() - b.top(), 0.f);
        float32 dy2 = math::max(b.bottom() - a.top(), 0.f);
        float32 dy = math::max(dy1, dy2);

        float32 dx1 = math::max(a.left() - b.right(), 0.f);
        float32 dx2 = math::max(b.left() - a.right(), 0.f);
        float32 dx = math::max(dx1, dx2);

        return math::sqrt(dx * dx + dy * dy);
    }

    inline static aabb create_from_center(vec2 center, vec2 dimensions) {
        auto half = dimensions / 2;
        return aabb{ center - half, center + half };
    }
};


