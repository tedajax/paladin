#pragma once

#include "types.h"

namespace tdjx
{
    struct color
    {
        uint8 r, g, b, a;

        static color k_red;
        static color k_green;
        static color k_blue;
        static color k_yellow;

        operator uint32() const
        {
            return r + (g << 8) + (b << 16) + (a << 24);
        }
    };

    
}