#include "perlin.h"

#include "algebra.h"

using math::lerp;
using math::grad;

perlin_gen::perlin_gen(uint32 seed) {
    std::iota(&d[0], &d[256], 0);
    std::default_random_engine engine(seed);
    std::shuffle(&d[0], &d[256], engine);
    std::copy(&d[0], &d[256], &d[256]);
}

float32 perlin_gen::noise(float32 x, float32 y, float32 z /* = 0.f */) const {
    int X = (math::floor_int(x) & 255);
    int Y = (math::floor_int(y) & 255);
    int Z = (math::floor_int(z) & 255);

    x -= math::floor(x);
    y -= math::floor(y);
    z -= math::floor(z);

    float32 u = math::fade(x), v = math::fade(y), w = math::fade(z);

    int A = d[X] + Y;
    int AA = d[A] + Z;
    int AB = d[A + 1] + Z;
    int B = d[X + 1] + Y;
    int BA = d[B] + Z;
    int BB = d[B + 1] + Z;

    float32 ret = lerp(
        lerp(
            lerp(
                grad(d[AA], x, y, z),
                grad(d[BA], x - 1, y, z),
                u),
            lerp(
                grad(d[AB], x, y - 1, z),
                grad(d[BB], x - 1, y - 1, z),
                u),
            v),
        lerp(
            lerp(
                grad(d[AA + 1], x, y, z - 1),
                grad(d[BA + 1], x - 1, y, z - 1),
                u),
            lerp(
                grad(d[AB + 1], x, y - 1, z - 1),
                grad(d[BB + 1], x - 1, y - 1, z - 1),
                u),
            v),
        w);

    return (ret + 1.f) / 2.f;
}