#pragma once

#include "types.h"

namespace pico8
{
    void init();
    void shutdown();

    void update(float32 dt);
    void flip();
    float32 time();
    void srand(uint32 seed);
    float32 rnd(float32 r);

    uint8 pget(int x, int y);

    void cls(uint8 c);
    void pset(int x, int y, uint8 c);
    void line(int x0, int y0, int x1, int y1, uint8 c);
    //void rect(int x, int y, int w, int h, uint8 c);
    //void rectfill(int x, int y, int w, int h, uint8 c);
    //void circ(int x, int y, int r, uint8 c);
    //void circfill(int x, int y, int r, uint8 c);
}
