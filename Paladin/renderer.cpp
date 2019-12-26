#include "renderer.h"

#include <GL/gl3w.h>
#include <SDL2/SDL.h>
#include <cstdio>

namespace tdjx
{
    namespace render
    {

        SDL_GLContext g_gl;
        uint g_programId;
        uint g_emptyVao;
        uint g_texture;
        uint g_textureUniform;
        SDL_Window* g_window;

        void set_texture_data(uint* data, int width, int height)
        {
            glBindTexture(GL_TEXTURE_2D, g_texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
        }

        bool init(SDL_Window* window)
        {
            g_window = window;

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

            SDL_DisplayMode current;
            SDL_GetCurrentDisplayMode(0, &current);

            g_gl = SDL_GL_CreateContext(g_window);

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

            glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

            glGenVertexArrays(1, &g_emptyVao);

            glGenTextures(1, &g_texture);
            glBindTexture(GL_TEXTURE_2D, g_texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glGenerateMipmap(GL_TEXTURE_2D);

            const char* vertexShader =
                "#version 410 core\n"
                "layout(location = 0) out vec2 uv;\n"
                "void main()\n"
                "{\n"
                "    float x = float(((uint(gl_VertexID) + 2u) / 3u) % 2u);\n"
                "    float y = float(((uint(gl_VertexID) + 1u) / 3u) % 2u);\n"
                "    gl_Position = vec4(-1.0f + x * 2.0f, 1.0f - y * 2.0f, 0.0f, 1.0f);\n"
                "    uv = vec2(x, y);\n"
                "}";

            const char* fragmentShader =
                "#version 410 core\n"
                "#define PI 3.1415926538\n"
                "layout(location = 0) in vec2 uv;\n"
                "uniform sampler2D image;\n"
                "out vec3 color;\n"
                "void main(){\n"
                " color = texture(image, uv).rgb;\n"
                "}";

            GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
            GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

            auto compile_shader = [](uint shaderId) -> bool
            {
                glCompileShader(shaderId);

                const size_t bufferLen = 2048;
                char buffer[bufferLen];

                int len = 0;

                glGetShaderInfoLog(shaderId, bufferLen, &len, buffer);

                if (len > 0)
                {
                    printf(buffer);
                    return false;
                }

                return true;
            };

            glShaderSource(vertexShaderId, 1, &vertexShader, nullptr);
            glCompileShader(vertexShaderId);

            glShaderSource(fragmentShaderId, 1, &fragmentShader, nullptr);
            glCompileShader(fragmentShaderId);

            g_programId = glCreateProgram();
            glAttachShader(g_programId, vertexShaderId);
            glAttachShader(g_programId, fragmentShaderId);
            glLinkProgram(g_programId);

            const size_t bufferLen = 2048;
            char buffer[bufferLen];

            int len = 0;

            glGetProgramInfoLog(g_programId, bufferLen, &len, buffer);

            if (len > 0)
            {
                printf(buffer);
            }

            glDetachShader(g_programId, vertexShaderId);
            glDetachShader(g_programId, fragmentShaderId);

            glDeleteShader(vertexShaderId);
            glDeleteShader(fragmentShaderId);

            glUseProgram(g_programId);

            g_textureUniform = glGetUniformLocation(g_programId, "texture");

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            return true;
        }

        void shutdown()
        {
            SDL_GL_DeleteContext(g_gl);
        }

        void draw()
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(g_programId);

            glBindTexture(GL_TEXTURE_2D, g_texture);
            glBindVertexArray(g_emptyVao);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(GL_NONE);
        }

        const char* glslVersion() { return "#version 410 core"; }
        SDL_GLContext  glContext() { return g_gl; }
    }
}


