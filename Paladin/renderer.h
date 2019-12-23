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
        void draw(float32 dt);

        void set_image_data(uint* data, int width, int height);

        const char* glslVersion();
        SDL_GLContext  glContext();
    }
}
