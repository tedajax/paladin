#pragma once

#include "types.h"
#include "camera.h"

#include <SDL2/SDL.h>

#include <vector>

namespace tdjx
{
    namespace render
    {
        bool init(SDL_Window* window);
        void shutdown();
        void draw();

        void set_texture_data(uint* data, int width, int height);

        const char* glslVersion();
        SDL_GLContext  glContext();
    }
}
