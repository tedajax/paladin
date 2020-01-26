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

        enum class Filtering
        {
            kNearest,
            KLinear,
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
        void set_palette(const uint8* data, int size);
        void set_intensity(const uint8* data);

        void reload_shaders();

        const char* glslVersion();
        SDL_GLContext  glContext();

        struct Material
        {
            uint programId;
            const char* vertexFilename;
            const char* fragmentFilename;
            std::unordered_map<std::string, int> uniforms;
        };

        namespace material
        {
            Material create(const char* vertFilename, const char* fragFilename, std::vector<std::string> uniforms);
            void destroy(Material& self);
        }
    }
}
