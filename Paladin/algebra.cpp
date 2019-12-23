#include "algebra.h"
#include <Box2D/Common/b2Math.h>
#include <glm/vec3.hpp>

vec2::vec2(const b2Vec2& b2vec)
    : x(b2vec.x), y(b2vec.y) { }

vec2::operator b2Vec2() const {
    return b2Vec2(x, y);
}

vec2::operator glm::vec3() const {
    return glm::vec3(x, 0, y);
}

float32 vec2::angle_between(const vec2& a, const vec2& b) {
    auto na = vec2::normalize(a);
    auto nb = vec2::normalize(b);
    return math::acos(dot(a, b));
}

vec2 vec2::ZERO(0, 0);
vec2 vec2::ONE(1, 1);
vec2 vec2::RIGHT(1, 0);
vec2 vec2::UP(0, -1);