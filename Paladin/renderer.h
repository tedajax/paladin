#pragma once

#include "types.h"
#include "camera.h"

#include <SDL2/SDL.h>

#include <vector>
#include <unordered_map>

namespace tdjx
{
    namespace render
    {
        enum class Mode
        {
            kDefault,
            kIntensity,
            kPalette,
            kCount
        };

        bool init(SDL_Window* window, int bufferWidth, int bufferHeight);
        void shutdown();
        void on_resize();
        void draw();

        const char* get_mode_name();
        void set_mode(Mode mode);
        void prev_mode();
        void next_mode();

        void set_texture_data(uint* data, int width, int height);
        void set_palette(uint8* data, int size);
        void set_intensity(uint8* data);

        const char* glslVersion();
        SDL_GLContext  glContext();

        struct Material
        {
            uint programId;
            std::unordered_map<std::string, int> uniforms;
        };

        namespace material
        {
            Material create(const char* vertFilename, const char* fragFilename, std::vector<const char*> uniforms);
        }
    }
}
