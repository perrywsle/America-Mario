#include "init.h"
#include <SDL2/SDL_image.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* backgroundTexture = NULL;
TTF_Font* font = NULL;

bool init() {
    bool success = true;
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        success = false;
    }

    window = SDL_CreateWindow("Shooter Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        success = false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        success = false;
    }

    if (TTF_Init() == -1) {
        printf("Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        success = false;
    }

    font = TTF_OpenFont("arial.ttf", 24);  
    if (font == NULL) {
        printf("Failed to load font: %s\n", TTF_GetError());
        success = false;
    }

    return success;
}

bool loadMedia() {
    bool success = true;

    backgroundTexture = IMG_LoadTexture(renderer, "background.png");
    if (backgroundTexture == NULL) {
        printf("Failed to load background texture! SDL_image Error: %s\n", IMG_GetError());
        success = false;
    }

    return success;
}

void clear() {
    if (backgroundTexture != NULL) {
        SDL_DestroyTexture(backgroundTexture);
        backgroundTexture = NULL;
    }

    if (renderer != NULL) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }

    if (window != NULL) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
}

void initializeGame(GameData* state) {
    state->shooterX = 0.0f;
    state->shooterY = GROUND_LEVEL;
    state->velocityY = 0.0f;
    state->onGround = true;
    state->cameraX = 0.0f;
    state->health = 3;
    state->ammo = 3;
    state->score = 0;

    state->deltaTime = 0.0f; 
    state->lastTime = SDL_GetTicks(); 
    state->isPaused = false; 
    state->quit = false; 

    Platform initialPlatforms[NUM_PLATFORM] = {
        {100, 500, 600, 30},
        {800, 400, 600, 30},
        {1500, 300, 600, 30}
    };

    // Initialize enemies
    Enemy initialEnemies1[NUM_ENEMIES1] = {
        {500, 400, 30, 30, true, 0, 100.0f},
        {1200, 300, 30, 30, true, 0, 100.0f},
        {1900, 200, 30, 30, true, 0, 100.0f}
    };

    Enemy initialEnemies2[NUM_ENEMIES2] = {
        {250, 470, 30, 30, true, 0, 100.0f},  
        {1200, 370, 30, 30, true, 0, 100.0f},
        {1900, 270, 30, 30, true, 0, 100.0f}
    };

    // Initialize collectibles
    Collectible initialCollectibles[NUM_COLLECTIBLES] = {
        {200, 400, 20, 20, false},
        {900, 350, 20, 20, false},
        {1700, 250, 20, 20, false}
    };

    Collectible initialAmmos[NUM_AMMOS] = {
        {200, 400, 20, 20, false},
        {900, 350, 20, 20, false},
        {1700, 250, 20, 20, false}
    };

    // Initialize bullets
    for (int i = 0; i < (state->ammo); i++) {
        state->bullets[i].active = false;
    }

    memcpy(state->platforms, initialPlatforms,sizeof(initialPlatforms));
    memcpy(state->enemies1, initialEnemies1,sizeof(initialEnemies1));
    memcpy(state->enemies2, initialEnemies2,sizeof(initialEnemies2));
    memcpy(state->collectibles, initialCollectibles,sizeof(initialCollectibles));
    memcpy(state->ammos , initialAmmos,sizeof(initialAmmos));
}
