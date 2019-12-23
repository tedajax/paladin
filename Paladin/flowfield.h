#pragma once

#include "algebra.h"
#include "perlin.h"
#include <vector>

class flow_field {
public:
    flow_field(f32 worldWidth, f32 worldHeight, f32 cellSize, f32 offsetX, f32 offsetY);
    void perlin_angles(const perlin_gen& perlin, f32 scale, f32 z = 0.f);
    void set(int cellX, int cellY, vec2 vec);
    vec2 get(int cx, int cy) const;
    vec2 get(vec2 pos) const;
    vec2 cell_center(int cx, int cy);
    static vec2 perlin_get(const perlin_gen& perlin, f32 x, f32 y, f32 z = 0.f);

    int width() const;
    int height() const;
    f32 cell_size() const;

    vec2 cell_to_world(int cx, int cy) const;
    bool world_to_cell(vec2 pos, int& cx, int& cy) const;

private:
    inline int index(int x, int y) const;

    f32 worldWidth;
    f32 worldHeight;
    f32 offsetX;
    f32 offsetY;
    f32 cellSize;
    int cellWidth;
    int cellHeight;
    std::vector<vec2> vectors;
};