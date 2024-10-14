#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "shooter.h"

void render(SDL_Renderer* renderer, 
            SDL_Texture* texture, 
            Platform platforms[], 
            Collectible collectibles[], 
            Collectible ammos[],
            Bullet bullets[],
            Enemy enemies1[],
            Enemy enemies2[],
            float shooterY, 
            float cameraX, 
            float shooterX, 
            int* score,
            int* health,
            int* ammo, 
            TTF_Font* font, 
            HillNoise* hn);
void initHillNoise(HillNoise* hn, float* sizes, int num_sizes);
void freeHillNoise(HillNoise* hn);

#endif
