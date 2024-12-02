#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "init.h"

void render(GameData g,
            SDL_Renderer* renderer, 
            SDL_Texture* textureBackground, 
            SDL_Texture* texturePause, 
            TTF_Font* font, 
            HillNoise* hn,
            int screen_width,
            int screen_height);
void freeHillNoise(HillNoise* hn);
void initHillNoise(HillNoise* hn, float* sizes, int num_sizes);


#endif
