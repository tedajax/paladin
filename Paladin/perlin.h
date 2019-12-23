#pragma once

#include "types.h"
#include <numeric>
#include <random>
#include <algorithm>

// modified version of a perlin noise generator I found here: https://solarianprogrammer.com/2012/07/18/perlin-noise-cpp-11/
// moved the generic math functions from that implementation into my math utilities

class perlin_gen {
public:
    perlin_gen(uint32 seed);
    float32 noise(float32 x, float32 y, float32 z = 0.f) const;
private:
    uint8 d[512];
};