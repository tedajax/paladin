#include <vector>
#include <random>
#include <functional>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <cinttypes>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <Box2D/Box2D.h>

#include <effolkronium/random.hpp>

#include "algebra.h"
#include "renderer.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "tdjx_gfx.h"
#include "tdjx_game.h"

#include "util.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <glm/glm.hpp>

#include "game_wolf.h"
#include "game_lab.h"

using Random = effolkronium::random_static;

int main(int argc, char* argv[])
{
    Random::seed(1);

    SDL_Init(SDL_INIT_VIDEO);

    //const int kWindowWidth = 1440;
    //const int kWindowHeight = 1080;
    const int kWindowWidth = 1280;
    const int kWindowHeight = 640;
    
    uint32 windowFlags = SDL_WINDOW_OPENGL;
    //windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    //windowFlags |= SDL_WINDOW_ALLOW_HIGHDPI;

    SDL_Window* window = SDL_CreateWindow(
        "Paladin",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        kWindowWidth, kWindowHeight,
        windowFlags);

    int ww, wh;
    int wdw, wdh;
    SDL_GetWindowSize(window, &ww, &wh);
        
    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, tdjx::render::glContext());
    ImGui_ImplOpenGL3_Init(tdjx::render::glslVersion());

    int fps = 0;
    uint64 ticks = SDL_GetPerformanceCounter();
    uint64 lastTicks = ticks;
    uint64 frame_ns = 0;

    int framesInSecond = 0;
    float32 dt = 0.f;
    float32 time = 0.f;

    bool isRunning = true;
    bool shouldReloadShaders = false;
    bool showImgui = true;

    auto app_quit = [&isRunning]() { isRunning = false; };
    auto app_reload_shaders = [&shouldReloadShaders]() { shouldReloadShaders = true; };
    auto toggle_imgui = [&showImgui]() { showImgui = !showImgui; };

    const std::unordered_map<SDL_Scancode, std::function<void(void)>> kDebugCommands = {
        { SDL_SCANCODE_ESCAPE, app_quit },
        { SDL_SCANCODE_F2, app_reload_shaders },
        { SDL_SCANCODE_F3, tdjx::render::prev_mode },
        { SDL_SCANCODE_F4, tdjx::render::next_mode },
        { SDL_SCANCODE_F8, toggle_imgui },
        { SDL_SCANCODE_P, []() { __debugbreak(); } },
    };

    const float32 size = 0.5f;
    glm::vec3 vertices[8] = {
        { size, size, size },
        { size, size, -size },
        { size, -size, size },
        { size, -size, -size },
        { -size, size, size },
        { -size, size, -size },
        { -size, -size, size },
        { -size, -size, -size }
    };

    const int kTriangles = 8;
    int indices[kTriangles * 3] = {
        0, 3, 2,
        0, 1, 3,
        4, 7, 6,
        4, 5, 7,
        0, 6, 2,
        0, 4, 6,
        1, 7, 3,
        1, 5, 7
    };

    std::unique_ptr<BaseGame> game(new LabGame(window));

    SDL_GL_GetDrawableSize(window, &wdw, &wdh);

    printf("w:  %d\nh:  %d\ndw: %d\ndh: %d\n", ww, wh, wdw, wdh);

    while (isRunning)
    {
        if (shouldReloadShaders)
        {
            shouldReloadShaders = false;
            tdjx::render::reload_shaders();
        }

        // Process Events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type)
            {
            case SDL_KEYDOWN:
            {
                auto search = kDebugCommands.find(event.key.keysym.scancode);
                if (search != kDebugCommands.end())
                {
                    search->second();
                }
                else
                {
                    game->on_key_down(event.key.keysym.scancode);
                }
                break;
            }
            case SDL_KEYUP:
                game->on_key_up(event.key.keysym.scancode);
                break;
            case SDL_MOUSEBUTTONDOWN:
                game->on_mouse_down(event.button.x, event.button.y, event.button.button);
                break;
            case SDL_MOUSEBUTTONUP:
                game->on_mouse_up(event.button.x, event.button.y, event.button.button);
                break;
            case SDL_MOUSEMOTION:
                game->on_mouse_move(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
                break;
            case SDL_QUIT:
                app_quit();
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event)
                {
                case SDL_WINDOWEVENT_RESIZED:
                    tdjx::render::on_resize();
                    break;
                default: break;
                }
                break;
            }
        }

        // TIME
        uint64 frequency = SDL_GetPerformanceFrequency();
        uint64 delta_ns = 0;
        {
            const uint64 k_nano = 1'000'000'000;

            lastTicks = ticks;
            ticks = SDL_GetPerformanceCounter();

            uint64 diff = ticks - lastTicks;

            delta_ns = diff * k_nano / frequency;

            dt = static_cast<float32>(static_cast<float64>(delta_ns) / static_cast<float64>(k_nano));
            time += dt;

            ++framesInSecond;
            frame_ns += delta_ns;
            if (frame_ns >= k_nano)
            {
                frame_ns -= k_nano;
                fps = framesInSecond;
                framesInSecond = 0;
            }
        }

        game->time.elapsed = time;
        game->time.delta = dt;
        game->update();
        game->render();

        // UI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        if (showImgui)
        {
            ImGui::Begin("Debug");

            ImGui::Text("FPS: %d", fps);
            ImGui::Text("Delta MS: %0.2fms", static_cast<float64>(delta_ns) / 1000000);

            if (ImGui::Button("Reload Shaders"))
            {
                app_reload_shaders();
            }

            if (ImGui::Button(tdjx::render::get_mode_name()))
            {
                tdjx::render::next_mode();
            }

            {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                ImGui::Text("Mouse Pos: %d, %d", mx, my);
                
                int windowWidth, windowHeight;
                SDL_GetWindowSize(window, &windowWidth, &windowHeight);

                float32 sx = static_cast<float32>(mx) / windowWidth;
                float32 sy = static_cast<float32>(my) / windowHeight;
                
                int gfxWidth, gfxHeight;
                tdjx::gfx::query_screen_dimensions(gfxWidth, gfxHeight);

                int screenX = static_cast<int>(sx * gfxWidth);
                int screenY = static_cast<int>(sy * gfxHeight);
                ImGui::Text("Mouse Pos: %d, %d", screenX, screenY);

                //tdjx::gfx::point(screenX, screenY, 8);
            }


            ImGui::End();
        }

        tdjx::gfx::flip();
        // RENDER
        {
            ImGui::Render();
            SDL_GL_MakeCurrent(window, tdjx::render::glContext());

            tdjx::render::draw();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            SDL_GL_SwapWindow(window);
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
