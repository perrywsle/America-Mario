#include "render.h"
#include "shooter.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PI 3.14159265358979323846
#define SQUARE_WIDTH 2

HillNoise hn_instance = {
    .sizes = NULL, 
    .offsets = NULL,
    .num_sizes = 0,
    .sigma = 1.0f
};

// Reference to the global `hn`
HillNoise* hn = &hn_instance;

// Initialize Hill Noise
void initHillNoise(HillNoise* hn, float* sizes, int num_sizes) {
    hn->sizes = sizes;
    hn->offsets = (float*)malloc(num_sizes * sizeof(float));
    hn->num_sizes = num_sizes;
    hn->sigma = 0.0f;

    for (int i = 0; i < num_sizes; i++) {
        hn->offsets[i] = ((float)rand() / RAND_MAX) * 2 * PI;
        hn->sigma += powf(sizes[i] / 2.0f, 2);
    }
    hn->sigma = sqrtf(hn->sigma);
}

// Evaluate terrain noise using sine waves
float evaluateHillNoise(HillNoise* hn, float x) {
    float noise = 0.0f;
    float alpha = 0.5f;
    float beta = 10.0f;
    float u;

    for (int i = 0; i < hn->num_sizes; i++) {
        noise += hn->sizes[i] * sinf(x / hn->sizes[i] + hn->offsets[i]);
    }

    noise /= hn->sigma;

    if (noise < 0){
        u = 1 - (0.5f * -1 * sqrtf(1.0f - expf(-2.0f / PI * noise * noise)) + 0.5f);
    }else {
        u = 1 - (0.5f * 1 * sqrtf(1.0f - expf(-2.0f / PI * noise * noise)) + 0.5f);
    }
    return alpha * powf((-logf(u)), (1.0/beta));
}

// Free memory for HillNoise
void freeHillNoise(HillNoise* hn) {
    free(hn->offsets);
}

void renderTerrains(SDL_Renderer* renderer, HillNoise* hn, float startX, float cameraX, SDL_Color color, float heightScale) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); // Set terrain color

    for (float x = startX; x < WORLD_WIDTH; x += 1) {
        float yNoise = evaluateHillNoise(hn, 3*x); // Noise-based terrain generation
        float y = SCREEN_HEIGHT - (yNoise * heightScale); // Scale and adjust height
        
        SDL_Rect filledArea = {
            (int)(x * SQUARE_WIDTH - cameraX),  
            (int)(y),        
            SQUARE_WIDTH,                       
            (int)(SCREEN_HEIGHT - y)            
        };
        SDL_RenderFillRect(renderer, &filledArea);
    }
}

void renderText(SDL_Renderer* renderer, TTF_Font* font) {
    char scoreText[20];
    snprintf(scoreText, sizeof(scoreText), "Score: %d", g.score);
    char ammoText[20];
    snprintf(ammoText, sizeof(ammoText), "Ammo: %d", g.ammo);

    SDL_Color textColor = {255, 255, 255, 255}; // White text
    SDL_Surface* textSurface1 = TTF_RenderText_Solid(font, scoreText, textColor);
    SDL_Surface* textSurface2 = TTF_RenderText_Solid(font, ammoText, textColor);
    
    if(!textSurface1 || !textSurface2) {
        printf("Unable to render text surface: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture* textTexture1 = SDL_CreateTextureFromSurface(renderer, textSurface1);
    SDL_Texture* textTexture2 = SDL_CreateTextureFromSurface(renderer, textSurface2);
    SDL_FreeSurface(textSurface1);
    SDL_FreeSurface(textSurface2);
    
    if(!textTexture1 || !textTexture2) {
        printf("Unable to create texture from rendered text: %s\n", SDL_GetError());
        return;
    }

    SDL_Rect renderQuad1 = {10, 10, textSurface1->w, textSurface1->h}; 
    SDL_Rect renderQuad2 = {200, 10, textSurface1->w, textSurface1->h}; 
    SDL_RenderCopy(renderer, textTexture1, NULL, &renderQuad1);
    SDL_RenderCopy(renderer, textTexture2, NULL, &renderQuad2);
    SDL_DestroyTexture(textTexture1);
    SDL_DestroyTexture(textTexture2);
}

void renderHearts(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); //red
    for (int i = 0; i < g.health; i++) {
        SDL_Rect heartRect = {10 + i * 40, 50, 30, 30};
        SDL_RenderFillRect(renderer, &heartRect);
    }
}

void drawShooter(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red shooter
    SDL_Rect shooterRect = {(int)(g.shooterX - g.cameraX), (int)(g.shooterY), 50, 50};
    SDL_RenderFillRect(renderer, &shooterRect);
}

void drawPlatforms(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);  // Blue platforms
    for (int i = 0; i < NUM_PLATFORM; i++) {
        SDL_Rect platformRect = {
            (int)(g.platforms[i].x - g.cameraX), 
            (int)(g.platforms[i].y), 
            (int)(g.platforms[i].width), 
            (int)(g.platforms[i].height)
        };
        SDL_RenderFillRect(renderer, &platformRect);
    }
}

void drawCollectibles(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);  // Yellow collectibles
    for (int i = 0; i < NUM_COLLECTIBLES; i++) {
        if (!g.collectibles[i].collected) {
            SDL_Rect collectibleRect = {
                (int)(g.collectibles[i].x - g.cameraX), 
                (int)g.collectibles[i].y, 
                g.collectibles[i].width, 
                g.collectibles[i].height
            };
            SDL_RenderFillRect(renderer, &collectibleRect);
        }
    }
}

void drawEnemies2(SDL_Renderer* renderer) {
    for (int i = 0; i < NUM_ENEMIES2; i++) {
        if (g.enemies2[i].active) {
            // Calculate the source rectangle from the sprite sheet
            SDL_Rect srcRect;
            srcRect.x = g.enemies2[i].currentFrame * g.enemies2[i].frameWidth;
            srcRect.y = 0;  // Assuming all frames are in one row
            srcRect.w = g.enemies2[i].frameWidth;
            srcRect.h = g.enemies2[i].frameHeight;

            // Calculate the destination rectangle on the screen
            SDL_Rect dstRect;
            dstRect.x = (int)(g.enemies2[i].x - g.cameraX);  // Adjust for camera position
            dstRect.y = (int)(g.enemies2[i].y);
            dstRect.w = g.enemies2[i].frameWidth;  // You can adjust size if needed
            dstRect.h = g.enemies2[i].frameHeight;

            // Render the enemy using the sprite sheet
            SDL_RenderCopy(renderer, g.enemies2[i].texture, &srcRect, &dstRect);
        }
    }
}

void drawEnemies1(SDL_Renderer* renderer) {
    for (int i = 0; i < NUM_ENEMIES1; i++) {
        if (g.enemies1[i].active) {
            // Calculate the source rectangle from the sprite sheet
            SDL_Rect srcRect;
            srcRect.x = g.enemies1[i].currentFrame * g.enemies1[i].frameWidth;
            srcRect.y = 0;  // Assuming all frames are in one row
            srcRect.w = g.enemies1[i].frameWidth;
            srcRect.h = g.enemies1[i].frameHeight;

            // Calculate the destination rectangle on the screen
            SDL_Rect dstRect;
            dstRect.x = (int)(g.enemies1[i].x - g.cameraX);  // Adjust for camera position
            dstRect.y = (int)(g.enemies1[i].y);
            dstRect.w = g.enemies1[i].frameWidth;  // You can adjust size if needed
            dstRect.h = g.enemies1[i].frameHeight;

            // Render the enemy using the sprite sheet
            SDL_RenderCopy(renderer, g.enemies1[i].texture, &srcRect, &dstRect);
        }
    }
}

void drawBullets(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow
    for (int i = 0; i < g.ammo + 1; i++) {
        if (g.bullets[i].active) {
            SDL_Rect bulletRect = {
                (int)(g.bullets[i].x - g.cameraX), 
                (int)g.bullets[i].y, 
                10, 
                5
            }; 
            SDL_RenderFillRect(renderer, &bulletRect);
        }
    }
}

void drawAmmo(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);  // Yellow collectibles
    for (int i = 0; i < NUM_AMMOS; i++) {
        if (!g.ammos[i].collected) {
            SDL_Rect ammoRect = {
                (int)(g.ammos[i].x - g.cameraX), 
                (int)g.ammos[i].y, 
                g.ammos[i].width, 
                g.ammos[i].height
            };
            SDL_RenderFillRect(renderer, &ammoRect);
        }
    }
}

void render(SDL_Renderer* renderer, 
            SDL_Texture* textureBackground, 
            SDL_Texture* texturePause, 
            TTF_Font* font, 
            HillNoise* hn) {
    // Clear the screen
    SDL_RenderClear(renderer);

    // Render background
    SDL_Rect bgRect = {(int)(g.cameraX), 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderCopy(renderer, textureBackground, &bgRect, NULL);

    // Render pause button
    SDL_Rect pauseButtonRect = {700, 50, 50, 50};
    SDL_RenderCopy(renderer, texturePause, NULL, &pauseButtonRect);

    // Render generated terrain
    renderTerrains(renderer, hn, 0, g.cameraX, (SDL_Color){34, 139, 34, 255}, 500);
    renderTerrains(renderer, hn, 0, g.cameraX, (SDL_Color){144, 238, 144, 255}, 300);

    // Draw game elements
    drawPlatforms(renderer);
    drawCollectibles(renderer);
    drawAmmo(renderer);
    drawEnemies1(renderer);
    drawEnemies2(renderer);
    drawShooter(renderer);
    drawBullets(renderer);

    // Render UI elements
    renderText(renderer, font);
    renderHearts(renderer);

    // Present the rendered frame
    SDL_RenderPresent(renderer);
}