#pragma once

#include "types.h"

#include <functional>
#include <optional>

struct SDL_Window;

namespace tdjx
{
    struct GameTime
    {
        float32 elapsed;
        float32 delta;
    };

    struct BaseGame
    {
        virtual void update() = 0;
        virtual void render() = 0;

        GameTime time;
    };

    template <typename t_context>
    struct Game : public BaseGame
    {
        t_context context;
    };
}