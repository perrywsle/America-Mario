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

void renderText(SDL_Renderer* renderer ,TTF_Font* font ,int* score, int* ammo) {
    char scoreText[20];
    snprintf(scoreText,sizeof(scoreText),"Score: %d",*score);
    char ammoText[20];
    snprintf(ammoText,sizeof(ammoText),"Ammo: %d",*ammo);

    SDL_Color textColor={255 ,255 ,255 ,255}; // White text
    SDL_Surface* textSurface1 = TTF_RenderText_Solid(font ,scoreText ,textColor);
    SDL_Surface* textSurface2 = TTF_RenderText_Solid(font ,ammoText ,textColor);
    
    if(!textSurface1){
        printf("Unable to render score text surface: %s\n",TTF_GetError());
        return;
    }
    if(!textSurface2){
        printf("Unable to render ammo text surface: %s\n",TTF_GetError());
        return;
    }

    SDL_Texture* textTexture1 = SDL_CreateTextureFromSurface(renderer, textSurface1);
    SDL_FreeSurface(textSurface1);  
    SDL_Texture* textTexture2 = SDL_CreateTextureFromSurface(renderer, textSurface2);
    SDL_FreeSurface(textSurface2);  
    
    if(!textTexture1){
        printf("Unable to create texture1 from rendered text: %s\n", SDL_GetError());
        return;
    }
    if(!textTexture2){
        printf("Unable to create texture2 from rendered text: %s\n", SDL_GetError());
        return;
    }

    SDL_Rect renderQuad1 = {10, 10, textSurface1->w, textSurface1->h}; 
    SDL_Rect renderQuad2 = {200, 10, textSurface1->w, textSurface1->h}; 
    SDL_RenderCopy(renderer, textTexture1, NULL, &renderQuad1);
    SDL_RenderCopy(renderer, textTexture2, NULL, &renderQuad2);
    SDL_DestroyTexture(textTexture1);
    SDL_DestroyTexture(textTexture2);
}

void renderHearts(SDL_Renderer* renderer, int* health){
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); //red
    for (int i = 0; i < *health; i++) {
        SDL_Rect heartRect = {10 + i * 40, 50, 30, 30};
        SDL_RenderFillRect(renderer, &heartRect);
    }
}

void drawShooter(SDL_Renderer* renderer, float shooterX, float shooterY, float cameraX) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red shooter
    SDL_Rect shooterRect = {(int)(shooterX - cameraX), (int)(shooterY), 50, 50};
    SDL_RenderFillRect(renderer, &shooterRect);
}

void drawPlatforms(SDL_Renderer* renderer, Platform platforms[], float cameraX) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);  // Blue platforms
    for (int i = 0; i < NUM_PLATFORM; i++) {
        SDL_Rect platformRect = {(int)(platforms[i].x - cameraX), (int)(platforms[i].y), (int)(platforms[i].width), (int)(platforms[i].height)};
        SDL_RenderFillRect(renderer, &platformRect);
    }
}

void drawCollectibles(SDL_Renderer* renderer, Collectible collectibles[], float cameraX) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);  // Yellow collectibles
    for (int i = 0; i < NUM_COLLECTIBLES; i++) {
        if (!collectibles[i].collected) {
            SDL_Rect collectibleRect = {(int)(collectibles[i].x - cameraX), (int)collectibles[i].y, collectibles[i].width, collectibles[i].height};
            SDL_RenderFillRect(renderer, &collectibleRect);
        }
    }
}

void drawEnemies1(SDL_Renderer* renderer, Enemy enemies1[], float* cameraX) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);  // Purple 
    for (int i = 0; i < NUM_ENEMIES1; i++) {
        if (enemies1[i].active) { 
            SDL_Rect enemy1Rect = {(int)(enemies1[i].x - *cameraX), (int)(enemies1[i].y), (int)(enemies1[i].width), (int)(enemies1[i].height)};
            SDL_RenderFillRect(renderer, &enemy1Rect);
        }
    }
}

void drawEnemies2(SDL_Renderer* renderer, Enemy enemies2[], float* cameraX) {
    SDL_SetRenderDrawColor(renderer, 155, 0, 255, 255);  // Purple
    for (int i = 0; i < NUM_ENEMIES2; i++) {
        if (enemies2[i].active) { 
            SDL_Rect enemy2Rect = {(int)(enemies2[i].x - *cameraX), (int)(enemies2[i].y), (int)(enemies2[i].width), (int)(enemies2[i].height)};
            SDL_RenderFillRect(renderer, &enemy2Rect);
        }
    }
}

void drawBullets(SDL_Renderer* renderer, Bullet bullets[], int* ammo, float cameraX) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow
    for (int i = 0; i < *ammo + 1; i++) {
        if (bullets[i].active) {
            SDL_Rect bulletRect = {(int)(bullets[i].x - cameraX), (int)bullets[i].y, 10, 5}; // Bullet size
            SDL_RenderFillRect(renderer, &bulletRect);
        }
    }
}

void drawAmmo(SDL_Renderer* renderer, Collectible ammos[], int* ammo, float cameraX) {
    SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);  // Yellow collectibles
    for (int i = 0; i < NUM_AMMOS; i++) {
        if (!ammos[i].collected) {
            SDL_Rect ammoRect = {(int)(ammos[i].x - cameraX), (int)ammos[i].y, ammos[i].width, ammos[i].height};
            SDL_RenderFillRect(renderer, &ammoRect);
        }
    }
}

void renderPauseButton(SDL_Renderer* renderer) {
    // Set color based on pause state
    SDL_Rect pauseButtonRect = {700, 50, 50, 50};
    SDL_SetRenderDrawColor(renderer, g.isPaused ? 255 : 0, 0, 0, 255); // Red when paused, black otherwise
    SDL_RenderFillRect(renderer, &pauseButtonRect);

    // Render button text if needed (optional)
    // You can draw "PAUSE" text here if you have a font set up
}

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
            HillNoise* hn) {

    // Clear the screen
    SDL_RenderClear(renderer);

    // Render background
    SDL_Rect bgRect = {(int)(cameraX), 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderCopy(renderer, texture, &bgRect, NULL);

    // Render generated terrain
    renderTerrains(renderer, hn, 0, cameraX, (SDL_Color){34, 139, 34, 255}, 500); // Dark green terrain
    renderTerrains(renderer, hn, 0, cameraX, (SDL_Color){144, 238, 144, 255}, 300); // Light green terrain on top

    // Draw platforms, collectibles, and shooter
    drawPlatforms(renderer, platforms, cameraX);
    drawCollectibles(renderer, collectibles, cameraX);
    drawAmmo(renderer, ammos, ammo, cameraX);
    drawEnemies1(renderer, enemies1, &cameraX);
    drawEnemies2(renderer, enemies2, &cameraX);
    drawShooter(renderer, shooterX, shooterY, cameraX);
    drawBullets(renderer, bullets, ammo, cameraX);

    // Render the score text
    renderText(renderer, font, score, ammo);

    // Render healthbar
    renderHearts(renderer, health);

    renderPauseButton(renderer);

    // Present the rendered frame
    SDL_RenderPresent(renderer);
}
