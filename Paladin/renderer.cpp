#include "renderer.h"

#include <GL/gl3w.h>
#include <SDL2/SDL.h>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace tdjx
{
    namespace render
    {
        enum class Textures
        {
            kIntensity,
            kPalette,
            kCount
        };

        const int kTextureCount = static_cast<int>(Textures::kCount);
        
        static struct
        {
            uint textures[kTextureCount];
            SDL_GLContext gl;
            int width;
            int height;
            uint emptyVao;
            Material material;
            SDL_Window* window;
            Mode renderMode;
            Filtering filtering;
        } r;
        
        uint get_texture(Textures tex)
        {
            return r.textures[static_cast<int>(tex)];
        }

        void set_texture_data(uint* data, int width, int height)
        {
            /*bind_texture(Textures::kBasic);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);*/
        }

        void set_palette(const uint8* data, int size)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, get_texture(Textures::kPalette));
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE0);
        }

        void set_intensity(const uint8* data)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, get_texture(Textures::kIntensity));
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, r.width, r.height, GL_RED, GL_UNSIGNED_BYTE, data);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        bool init(SDL_Window* window, int bufferWidth, int bufferHeight)
        {
            r.window = window;
            r.width = bufferWidth;
            r.height = bufferHeight;

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

            SDL_DisplayMode current;
            SDL_GetCurrentDisplayMode(0, &current);

            r.gl = SDL_GL_CreateContext(r.window);

            // vsync
            SDL_GL_SetSwapInterval(1);

            if (gl3wInit() != GL3W_OK)
            {
                fprintf(stderr, "gl3w failed to init.\n");
                return false;
            }

            if (!gl3wIsSupported(4, 1))
            {
                fprintf(stderr, "OpenGL 4.1 is not supported.\n");
                return false;
            }

            glClearColor(0.9f, 0.0f, 0.9f, 1.0f);

            glGenVertexArrays(1, &r.emptyVao);
            
            glGenTextures(kTextureCount, r.textures);
            for (int i = 0; i < kTextureCount; ++i)
            {
                glBindTexture(GL_TEXTURE_2D, r.textures[i]);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glGenerateMipmap(GL_TEXTURE_2D);
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, get_texture(Textures::kIntensity));
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, r.width, r.height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);
            
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, get_texture(Textures::kPalette));
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 1, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);

            glActiveTexture(GL_TEXTURE0);

            r.material = material::create("assets/shaders/screen_quad.vert",
                "assets/shaders/screen_quad_indexed.frag",
                { "intensity", "palette", "mode", "bufferWindowRatio" });

            on_resize();

            set_filtering(Filtering::kNearest);

            return true;
        }

        void shutdown()
        {
            SDL_GL_DeleteContext(r.gl);
        }

        void on_resize()
        {
            glUseProgram(r.material.programId);

            int windowWidth, windowHeight;
            SDL_GetWindowSize(r.window, &windowWidth, &windowHeight);

            float32 bufferAspect = static_cast<float32>(r.width) / r.height;
            float32 windowAspect = static_cast<float32>(windowWidth) / windowHeight;

            float32 offset = bufferAspect / windowAspect;

            glUniform1f(r.material.uniforms["bufferWindowRatio"], offset);

            glUseProgram(0);
        }

        const char* kModeNames[static_cast<int>(Mode::kCount)] = {
            "Normal",
            "Intensity",
            "Palette"
        };

        const char* get_mode_name()
        {
            return kModeNames[static_cast<int>(r.renderMode)];
        }

        const char* get_filtering_name()
        {
            switch (r.filtering)
            {
            case Filtering::kNearest: return "Nearest";
            case Filtering::KLinear: return "Linear";
            default: return "Unknown";
            }
        }

        void set_mode(Mode mode)
        {
            r.renderMode = mode;
            glUseProgram(r.material.programId);
            glUniform1i(r.material.uniforms["mode"], static_cast<int>(r.renderMode));
            glUseProgram(0);
        }

        void next_mode()
        {
            int modeId = (static_cast<int>(r.renderMode) + 1) % static_cast<int>(Mode::kCount);
            set_mode(static_cast<Mode>(modeId));
        }

        void prev_mode()
        {
            int modeId = static_cast<int>(r.renderMode) - 1;
            if (modeId < 0)
            {
                modeId += static_cast<int>(Mode::kCount);
            }
            set_mode(static_cast<Mode>(modeId));
        }

        void set_filtering(Filtering filtering)
        {
            r.filtering = filtering;

            glBindTexture(GL_TEXTURE_2D, get_texture(Textures::kIntensity));
            
            int f;
            switch (filtering)
            {
            default:
            case Filtering::kNearest: f = GL_NEAREST;
            case Filtering::KLinear: f = GL_LINEAR;
            }


            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, get_texture(Textures::kIntensity));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, f);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, f);
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, r.width, r.height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        void prev_filtering()
        {
            int filteringId = static_cast<int>(r.filtering) - 1;
            if (filteringId < 0)
            {
                filteringId = static_cast<int>(Filtering::kCount) - 1;
            }
            set_filtering(static_cast<Filtering>(filteringId));
        }

        void next_filtering()
        {
            int filteringId = static_cast<int>(r.filtering) + 1;
            if (filteringId >= static_cast<int>(Filtering::kCount))
            {
                filteringId = 0;
            }
            set_filtering(static_cast<Filtering>(filteringId));
        }

        void draw()
        {
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(r.material.programId);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, get_texture(Textures::kIntensity));
            glUniform1i(r.material.uniforms["intensity"], 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, get_texture(Textures::kPalette));
            glUniform1i(r.material.uniforms["palette"], 1);

            glActiveTexture(GL_TEXTURE0);

            glBindVertexArray(r.emptyVao);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(GL_NONE);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);

            glUseProgram(0);
        }

        void reload_shaders()
        {
            const char* vert = r.material.vertexFilename;
            const char* frag = r.material.fragmentFilename;

            std::vector<std::string> uniforms;
            uniforms.reserve(r.material.uniforms.size());
            for (auto kvp : r.material.uniforms)
            {
                uniforms.push_back(kvp.first);
            }

            material::destroy(r.material);

            r.material = material::create(vert, frag, uniforms);
        }

        const char* glslVersion() { return "#version 410 core"; }
        SDL_GLContext  glContext() { return r.gl; }

        namespace material
        {
            Material create(const char* vertFilename, const char* fragFilename, std::vector<std::string> uniforms)
            {
                auto loadShader = [](const char* filename, uint shaderId)
                {
                    // Load file contents and set shader source
                    {
                        std::ifstream file(filename);
                        std::stringstream buffer;
                        buffer << file.rdbuf();
                        std::string strBuffer = buffer.str();

                        const char* cbuffer = strBuffer.c_str();

                        glShaderSource(shaderId, 1, &cbuffer, nullptr);
                    }

                    glCompileShader(shaderId);
                };

                uint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
                uint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

                loadShader(vertFilename, vertexShaderId);
                loadShader(fragFilename, fragmentShaderId);

                uint programId = glCreateProgram();
                glAttachShader(programId, vertexShaderId);
                glAttachShader(programId, fragmentShaderId);
                glLinkProgram(programId);

                {
                    const size_t bufferLen = 2048;
                    char buffer[bufferLen];

                    int len = 0;

                    glGetProgramInfoLog(programId, bufferLen, &len, buffer);

                    if (len > 0)
                    {
                        printf(" -- SHADER ERROR -- \n");
                        printf(buffer);
                    }
                }

                glDetachShader(programId, vertexShaderId);
                glDetachShader(programId, fragmentShaderId);

                glDeleteShader(vertexShaderId);
                glDeleteShader(fragmentShaderId);

                Material result;
                result.programId = programId;
                result.vertexFilename = vertFilename;
                result.fragmentFilename = fragFilename;

                for (auto uniformName : uniforms)
                {
                    int location = glGetUniformLocation(programId, uniformName.c_str());
                    if (location < 0)
                    {
                        printf("Unable to find uniform location for \'%s\'\n", uniformName.c_str());
                    }
                    result.uniforms.insert_or_assign(uniformName, location);
                }

                return result;
            }

            void destroy(Material& self)
            {
                glDeleteProgram(self.programId);
                self.programId = 0;
            }
        }
    }
}


