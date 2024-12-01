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

void renderTerrains(SDL_Renderer* renderer, HillNoise* hn, float startX, SDL_Color color, float heightScale, int screen_height) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); // Set terrain color

    for (float x = startX; x < WORLD_WIDTH; x += 1) {
        float yNoise = evaluateHillNoise(hn, 3*x); // Noise-based terrain generation
        float y = screen_height - (yNoise * heightScale); // Scale and adjust height
        
        SDL_Rect filledArea = {
            (int)(x * SQUARE_WIDTH - g.cameraX),  
            (int)(y),        
            SQUARE_WIDTH,                       
            (int)(screen_height - y)            
        };
        SDL_RenderFillRect(renderer, &filledArea);
    }
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, int screen_width) {
    int currentPlayer = g.isPlayer1Turn ? 0 : 1;

    // Create text for current turn indicator
    char turnText[50];
    snprintf(turnText, sizeof(turnText), "Player %d's Turn", currentPlayer + 1);

    // Render text only for the current player's stats
    char scoreText[30];
    snprintf(scoreText, sizeof(scoreText), "P%d Score: %d", currentPlayer + 1, g.shooters[currentPlayer].score);
    char ammoText[30];
    snprintf(ammoText, sizeof(ammoText), "P%d Ammo: %d", currentPlayer + 1, g.shooters[currentPlayer].ammo);
    char timeText[30];
    snprintf(timeText, sizeof(timeText), "P%d Time: %.2lf", currentPlayer + 1, g.shooters[currentPlayer].time);

    SDL_Color textColor = {255, 255, 0, 255}; // Yellow for active player

    // Render turn indicator
    SDL_Surface* turnSurface = TTF_RenderText_Solid(font, turnText, (SDL_Color){0, 0, 0, 255});
    if (turnSurface) {
        SDL_Texture* turnTexture = SDL_CreateTextureFromSurface(renderer, turnSurface);
        SDL_Rect turnRect = {screen_width / 2 - turnSurface->w / 2, 10, turnSurface->w, turnSurface->h};
        SDL_RenderCopy(renderer, turnTexture, NULL, &turnRect);
        SDL_FreeSurface(turnSurface);
        SDL_DestroyTexture(turnTexture);
    }

    // Render player stats
    SDL_Surface* surfaces[] = {
        TTF_RenderText_Solid(font, scoreText, textColor),
        TTF_RenderText_Solid(font, ammoText, textColor),
        TTF_RenderText_Solid(font, timeText, textColor)
    };

    int xOffset = currentPlayer * 0;
    int yPositions[] = {40, 70, 100};

    for (int j = 0; j < 3; j++) {
        if (surfaces[j]) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surfaces[j]);
            SDL_Rect renderQuad = {10 + xOffset, yPositions[j], surfaces[j]->w, surfaces[j]->h};
            SDL_RenderCopy(renderer, texture, NULL, &renderQuad);
            SDL_FreeSurface(surfaces[j]);
            SDL_DestroyTexture(texture);
        }
    }
}

void renderHearts(SDL_Renderer* renderer) {
    int currentPlayer = g.isPlayer1Turn ? 0 : 1;

    SDL_Color heartColor = {255, 0, 0, 255}; 
    SDL_SetRenderDrawColor(renderer, heartColor.r, heartColor.g, heartColor.b, heartColor.a);

    int xOffset = currentPlayer * 0;
    for (int i = 0; i < g.shooters[currentPlayer].health; i++) {
        SDL_Rect heartRect = {10 + xOffset + i * 60, 130, 50, 50};
        SDL_RenderFillRect(renderer, &heartRect);
    }
}

void drawShooter(SDL_Renderer* renderer) {
    Shooter* currentShooter = &g.shooters[g.isPlayer1Turn ? 0 : 1];
    SDL_Texture* currentTexture = g.isPlayer1Turn ? shooter1SpriteSheetIdle : shooter2SpriteSheetIdle;

    currentShooter->animationTimer += g.deltaTime;
    if (currentShooter->animationTimer >= currentShooter->frameDelay) {
        currentShooter->currentFrame = (currentShooter->currentFrame + 1) % currentShooter->totalFrames;
        currentShooter->animationTimer = 0;
    }

    SDL_Rect srcRect;
    srcRect.x = currentShooter->currentFrame * currentShooter->frameWidth;
    srcRect.y = 0;
    srcRect.w = currentShooter->frameWidth;
    srcRect.h = currentShooter->frameHeight;

    SDL_Rect dstRect = {
        (int)(currentShooter->x - g.cameraX),
        (int)(currentShooter->y),
        100,
        100
    };
    // destroy texture?

    SDL_RenderCopy(renderer, currentTexture, &srcRect, &dstRect);
}

void drawPlatforms(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);  // Blue platforms
    for (int i = 0; i < g.numPlatforms; i++) {
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
    for (int i = 0; i < g.numCollectibles; i++) {
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

void drawEnemies1(SDL_Renderer* renderer) {
    for (int i = 0; i < g.numEnemies1; i++) {
        if (g.enemies1[i].active) {
            SDL_Rect srcRect;
            srcRect.x = g.enemies1[i].currentFrame * g.enemies1[i].frameWidth;
            srcRect.y = 0;  
            srcRect.w = g.enemies1[i].frameWidth;
            srcRect.h = g.enemies1[i].frameHeight;

            SDL_Rect dstRect;
            dstRect.x = (int)(g.enemies1[i].x - g.cameraX); 
            dstRect.y = (int)(g.enemies1[i].y);
            dstRect.w = g.enemies1[i].width;  
            dstRect.h = g.enemies1[i].height;

            SDL_RenderCopy(renderer, g.enemies1[i].texture, &srcRect, &dstRect);
        }
    }
}

void drawEnemies2(SDL_Renderer* renderer) {
    for (int i = 0; i < g.numEnemies2; i++) {
        if (g.enemies2[i].active) {
            SDL_Rect srcRect;
            srcRect.x = g.enemies2[i].currentFrame * g.enemies2[i].frameWidth;
            srcRect.y = 0; 
            srcRect.w = g.enemies2[i].frameWidth;
            srcRect.h = g.enemies2[i].frameHeight;

            SDL_Rect dstRect;
            dstRect.x = (int)(g.enemies2[i].x - g.cameraX); 
            dstRect.y = (int)(g.enemies2[i].y);
            dstRect.w = g.enemies2[i].width;  
            dstRect.h = g.enemies2[i].height;

            SDL_RenderCopy(renderer, g.enemies2[i].texture, &srcRect, &dstRect);
        }
    }
}

void drawBullets(SDL_Renderer* renderer) {
    static int currentFrame = 0;
    static float animationTimer = 0;  
    const float frameDelay = 0.1f;    
    const int totalFrames = 4;        
    
    // Update animation timer
    animationTimer += g.deltaTime;
    if (animationTimer >= frameDelay) {
        currentFrame = (currentFrame + 1) % totalFrames;
        animationTimer = 0;
    }
    int frameWidth = 16;  

    for (int i = 0; i < g.ammo + 1; i++) {
        if (g.bullets[i].active) {
            SDL_Rect srcRect = {
                currentFrame * frameWidth, 
                0,                         
                frameWidth,                
                16                         
            }; 

            SDL_Rect dstRect = {
                (int)(g.bullets[i].x - g.cameraX), 
                (int)g.bullets[i].y, 
                40,  // Display width
                40   // Display height
            };

            SDL_RenderCopy(renderer, bulletSpriteSheet, &srcRect, &dstRect);
        }
    }
}

void drawAmmo(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);  // Yellow collectibles
    for (int i = 0; i < g.numAmmos; i++) {
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

void drawFinishFlag(SDL_Renderer* renderer, int screen_height) {
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 200);
    SDL_Rect flagRect = {
        (int)(WORLD_WIDTH - g.cameraX),
        screen_height - 600,
        50,
        600
    };
    SDL_RenderFillRect(renderer, &flagRect);
}

void render(SDL_Renderer* renderer, 
            SDL_Texture* textureBackground, 
            SDL_Texture* texturePause, 
            TTF_Font* font, 
            HillNoise* hn,
            int screen_width,
            int screen_height) {
    // Clear the screen
    SDL_RenderClear(renderer);

    // Render background
    SDL_Rect bgRect = {(int)(g.cameraX), 0, screen_width, screen_height};
    SDL_RenderCopy(renderer, textureBackground, &bgRect, NULL);

    // Render pause button
    SDL_Rect pauseButtonRect = {1820, 50, 100, 100};
    SDL_RenderCopy(renderer, texturePause, NULL, &pauseButtonRect);

    // Render generated terrain
    renderTerrains(renderer, hn, 0, (SDL_Color){34, 139, 34, 255}, 500, screen_height);
    renderTerrains(renderer, hn, 0, (SDL_Color){144, 238, 144, 255}, 300, screen_height);

    // Draw game elements
    drawPlatforms(renderer);
    drawCollectibles(renderer);
    drawAmmo(renderer);
    drawEnemies1(renderer);
    drawEnemies2(renderer);
    drawShooter(renderer);
    drawBullets(renderer);
    drawFinishFlag(renderer, screen_height);

    // Render UI elements
    renderText(renderer, font, screen_width);
    renderHearts(renderer);

    // Present the rendered frame
    SDL_RenderPresent(renderer);
}