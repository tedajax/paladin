#pragma once

#pragma once

#include "types.h"
#include "tdjx_game.h"
#include "tdjx_math.h"
#include "tdjx_gfx.h"

struct SDL_Window;

struct LabContext
{
};

struct LabGame : public tdjx::Game<LabContext>
{
    LabGame(SDL_Window* window);
    ~LabGame();

    void update() override final;
    void render() override final;

    void on_mouse_down(int x, int y, int button) override;
    void on_mouse_up(int x, int y, int button) override;
    void on_mouse_move(int x, int y, int dx, int dy) override;
};