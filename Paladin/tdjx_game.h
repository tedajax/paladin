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

        virtual void on_key_down(int key) {}
        virtual void on_key_up(int key) {}
        virtual void on_mouse_down(int x, int y, int button) {}
        virtual void on_mouse_up(int x, int y, int button) {}
        virtual void on_mouse_move(int x, int y, int dx, int dy) {}

        GameTime time;
    };

    template <typename t_context>
    struct Game : public BaseGame
    {
        t_context context;
    };
}