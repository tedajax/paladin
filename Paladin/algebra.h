#pragma once

#include "types.h"
#include <limits>
#include <glm/vec3.hpp>
#include <cmath>

#include "tdjx_math.h"

using namespace tdjx;

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


