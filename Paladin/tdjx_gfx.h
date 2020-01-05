#pragma once

#include "types.h"

namespace tdjx
{
    namespace gfx
    {
        void init(int width, int height, int colors);
        void shutdown();

        void clear(int color);
        void point(int x, int y, int color);
        void line(int x0, int y0, int x1, int y1, int color);
        void circle(int x0, int y0, int radius, int color);
        void circle_fill(int x0, int y0, int radius, int color);
        void rectangle(int x0, int y0, int x1, int y1, int color);
        void rectangle_fill(int x0, int y0, int x1, int y1, int color);

        void* get_context();
        uint8* get_pixels();

        struct Rect
        {
            int x0, y0, x1, y1;
        };

        namespace rect
        {
            inline int width(const Rect& self);
            inline int height(const Rect& self);

            inline bool contains_point(const Rect& self, int x, int y);
            inline bool intersect(const Rect& a, const Rect& b);

            bool clip_rect(const Rect& source, Rect& dest);
            bool clip_line(const Rect& r, int& x0, int& y0, int& x1, int& y1);
        }
    }
}
