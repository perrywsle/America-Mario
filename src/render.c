#include "render.h"

#define PI 3.14159265358979323846
#define SQUARE_WIDTH 2

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

void renderBackground(GameData g, SDL_Renderer* renderer, int screen_width, int screen_height) {
    SDL_Rect bgRect = {(int)(g.cameraX), 0, screen_width, screen_height};
    SDL_RenderCopy(renderer, g.backgroundTexture, &bgRect, NULL);
}

void renderTerrains(GameData g, SDL_Renderer* renderer, HillNoise* hn, float startX, SDL_Color color, float heightScale, int screen_height) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a); // Set terrain color

    for (float x = startX; x < WORLD_WIDTH; x += 1) {
        float yNoise = evaluateHillNoise(hn, 3*x); // Noise-based terrain generation
        float y = screen_height - (yNoise * heightScale); // Scale and adjust height
        
        SDL_Rect filledArea;
        filledArea.x = (int)(x * SQUARE_WIDTH - g.cameraX);
        filledArea.y = (int)(y);
        filledArea.w = SQUARE_WIDTH;
        filledArea.h = (int)(screen_height - y);

        SDL_RenderFillRect(renderer, &filledArea);
    }
}

void renderText(GameData g, SDL_Renderer* renderer, TTF_Font* font, int screen_width) {
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
        
        SDL_Rect turnRect;
        turnRect.x = screen_width / 2 - turnSurface->w / 2;
        turnRect.y = 10;
        turnRect.w = turnSurface->w;
        turnRect.h = turnSurface->h;

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

    int yPositions[] = {40, 70, 100};

    for (int j = 0; j < 3; j++) {
        if (surfaces[j]) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surfaces[j]);
            SDL_Rect renderQuad = {10, yPositions[j], surfaces[j]->w, surfaces[j]->h};
            SDL_RenderCopy(renderer, texture, NULL, &renderQuad);
            SDL_FreeSurface(surfaces[j]);
            SDL_DestroyTexture(texture);
        }
    }
}

void renderHearts(GameData g, SDL_Renderer* renderer) {
    int currentPlayer = g.isPlayer1Turn ? 0 : 1;

    SDL_Color heartColor = {255, 0, 0, 255}; 
    SDL_SetRenderDrawColor(renderer, heartColor.r, heartColor.g, heartColor.b, heartColor.a);
    int offSet = 60;

    for (int i = 0; i < g.shooters[currentPlayer].health; i++) {
        SDL_Rect heartRect;
        heartRect.x = 10 + offSet*i;
        heartRect.y = 130;
        heartRect.w = 50;
        heartRect.h = 50;

        SDL_RenderFillRect(renderer, &heartRect);
    }
}

void drawShooter(GameData g, SDL_Renderer* renderer) {
    Shooter* currentShooter = &g.shooters[g.isPlayer1Turn ? 0 : 1];
    SDL_Texture* currentTexture = currentShooter->texture;

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

    SDL_Rect dstRect;
    dstRect.x = (int)(currentShooter->x - g.cameraX);
    dstRect.y = (int)(currentShooter->y);
    dstRect.w = currentShooter->width;
    dstRect.h = currentShooter->height;

    SDL_RenderCopy(renderer, currentTexture, &srcRect, &dstRect);
}

void drawPlatforms(GameData g, SDL_Renderer* renderer) {
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

void drawCollectibles(GameData g, SDL_Renderer* renderer) {
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

void drawEnemies1(GameData g, SDL_Renderer* renderer) {
    for (int i = 0; i < g.numEnemies1; i++) {
        Enemy* currentEnemy = &g.enemies1[i];
        SDL_Texture* currentTexture = currentEnemy->texture;
        
        // animation update
        currentEnemy->animationTimer += g.deltaTime;
        if (currentEnemy->animationTimer >= currentEnemy->frameDelay) {
            currentEnemy->currentFrame = (currentEnemy->currentFrame + 1) % currentEnemy->totalFrames;
            currentEnemy->animationTimer = 0;
        }

        if (currentEnemy->active) {
            SDL_Rect srcRect;
            srcRect.x = currentEnemy->currentFrame * currentEnemy->frameWidth;
            srcRect.y = 0;  
            srcRect.w = currentEnemy->frameWidth;
            srcRect.h = currentEnemy->frameHeight;

            SDL_Rect dstRect;
            dstRect.x = (int)(currentEnemy->x - g.cameraX); 
            dstRect.y = (int)(currentEnemy->y);
            dstRect.w = currentEnemy->width;  
            dstRect.h = currentEnemy->height;

            SDL_RenderCopy(renderer, currentTexture, &srcRect, &dstRect);
        }
    }
}

void drawEnemies2(GameData g, SDL_Renderer* renderer) {
    for (int i = 0; i < g.numEnemies2; i++) {
        Enemy* currentEnemy = &g.enemies2[i];
        SDL_Texture* currentTexture = currentEnemy->texture;

        // Animation update
        currentEnemy->animationTimer += g.deltaTime;
        if (currentEnemy->animationTimer >= currentEnemy->frameDelay) {
            currentEnemy->currentFrame = (currentEnemy->currentFrame + 1) % currentEnemy->totalFrames;
            currentEnemy->animationTimer = 0;
        }
        if (currentEnemy->active) {
            SDL_Rect srcRect;
            srcRect.x = currentEnemy->currentFrame * currentEnemy->frameWidth;
            srcRect.y = 0; 
            srcRect.w = currentEnemy->frameWidth;
            srcRect.h = currentEnemy->frameHeight;

            SDL_Rect dstRect;
            dstRect.x = (int)(currentEnemy->x - g.cameraX); 
            dstRect.y = (int)(currentEnemy->y);
            dstRect.w = currentEnemy->width;  
            dstRect.h = currentEnemy->height;

            SDL_RenderCopy(renderer, currentTexture, &srcRect, &dstRect);
        }
    }
}

void drawBullets(GameData g, SDL_Renderer* renderer) {
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
            SDL_Rect srcRect;
            srcRect.x = currentFrame * frameWidth;
            srcRect.y = 0;
            srcRect.w = frameWidth;
            srcRect.h = 16;

            SDL_Rect dstRect;
            dstRect.x = (int)(g.bullets[i].x - g.cameraX);
            dstRect.y = (int)g.bullets[i].y;
            dstRect.w = 40;
            dstRect.h = 40;

            SDL_RenderCopy(renderer, g.bulletSpriteSheet, &srcRect, &dstRect);
        }
    }
}

void drawAmmo(GameData g, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);  // Yellow collectibles
    for (int i = 0; i < g.numAmmos; i++) {
        if (!g.ammos[i].collected) {
            SDL_Rect ammoRect;
            ammoRect.x = (int)(g.ammos[i].x - g.cameraX);
            ammoRect.y = (int)g.ammos[i].y;
            ammoRect.w = g.ammos[i].width;
            ammoRect.h = g.ammos[i].height;

            SDL_RenderFillRect(renderer, &ammoRect);
        }
    }
}

void drawFinishFlag(GameData g, SDL_Renderer* renderer, int screen_height) {
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 200);
    SDL_Rect flagRect = {
        (int)(WORLD_WIDTH - g.cameraX),
        screen_height - 600,
        50,
        600
    };
    SDL_RenderFillRect(renderer, &flagRect);
}

void drawPauseButton(GameData g, SDL_Renderer* renderer) {
    SDL_Rect pauseButtonRect = {1820, 50, 100, 100};
    SDL_RenderCopy(renderer, g.pauseTexture, NULL, &pauseButtonRect);
}

void render(GameData g,
            SDL_Renderer* renderer, 
            TTF_Font* font, 
            HillNoise* hn,
            int screen_width,
            int screen_height) {
    // Clear the screen
    SDL_RenderClear(renderer);

    // Render background
    renderBackground(g, renderer, screen_width, screen_height);

    // Render generated terrain
    renderTerrains(g, renderer, hn, 0, (SDL_Color){34, 139, 34, 255}, 500, screen_height);
    renderTerrains(g, renderer, hn, 0, (SDL_Color){144, 238, 54, 255}, 300, screen_height);

    // Draw pause button
    drawPauseButton(g, renderer);

    // Draw game entities
    drawPlatforms(g, renderer);
    drawCollectibles(g, renderer);
    drawAmmo(g, renderer);
    drawEnemies1(g, renderer);
    drawEnemies2(g, renderer);
    drawShooter(g, renderer);
    drawBullets(g, renderer);
    drawFinishFlag(g, renderer, screen_height);

    // Render UI elements
    renderText(g, renderer, font, screen_width);
    renderHearts(g, renderer);

    // Present the rendered frame
    SDL_RenderPresent(renderer);
}