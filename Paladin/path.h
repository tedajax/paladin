#pragma once

#include "algebra.h"
#include <vector>
#include <limits>

enum class path_dir {
    kCW,
    kCCW
};

class path {
public:
    path(path_dir dir, std::initializer_list<vec2> pts);
    path(path_dir dir, const vec2* pts, size_t count);

    // find nearest point on path to point p
    vec2 nearest(const vec2& p, vec2& direction);
    // how far away is p from the path
    float32 distance(const vec2& p);

    inline const std::vector<vec2>& path_points() const { return points; }

private:
    std::vector<vec2> points;
    path_dir dir;
};