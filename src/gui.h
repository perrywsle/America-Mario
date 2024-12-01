#ifndef UI_H
#define UI_H

#include <SDL2/SDL.h>
#include "init.h"

void renderMainMenu(int screen_width, int screen_height, GameData* g);
void renderPauseMenu(int screen_width, int screen_height, GameData* g);
void renderSummaryWindow(int screen_width, int screen_height, GameData* g);
void processInput(SDL_Event* e, bool* leftPressed, bool* rightPressed, bool* spacePressed);

#endif