#pragma once

#pragma once

#include "types.h"
#include "tdjx_game.h"

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
};