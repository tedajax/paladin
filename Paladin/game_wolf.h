#pragma once

#include "types.h"
#include "tdjx_game.h"

struct SDL_Window;

struct WolfContext
{
    struct Player
    {
        float32 x, y;
        float32 rot;
    };

    Player player = { 2, 4, 0 };
};

struct WolfGame : public tdjx::Game<WolfContext>
{
    void init(void* userData) override final;
    void update() override final;
    void render() override final;
};