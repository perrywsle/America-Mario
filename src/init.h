#ifndef INIT_H
#define INIT_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <cjson/cJSON.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define WORLD_WIDTH 3000
#define SHOOTER_SPEED 200
#define JUMP_SPEED 600
#define GRAVITY 680
#define GROUND_LEVEL (1080 - 100)
#define LEFT_BOUNDARY 0
#define MAX_BULLETS 10
#define MAX_HEALTH 3

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

// Structure to hold save file information
typedef struct {
    char filename[256];
    char displayName[256];  
} SaveFileInfo;

typedef struct {
    float x, y;
    float width, height;
} Platform;

typedef struct {
    float x, y;
    float width, height;
} PauseButton;

typedef struct {
    float x, y;
    int width, height;
    float velocityY;
    bool onGround;
    int health;
    int ammo;
    int score;
    double time;
    char textureLocation[256];
    SDL_Texture* texture;
    int currentFrame;
    int frameWidth;          
    int frameHeight;         
    int totalFrames; 
    float animationTimer;
    float frameDelay;
    bool dead;
} Shooter;

typedef struct {
    float x, y;              
    int width, height;       
    bool active;   
    int platformIndex;       
    int currentFrame;        
    float speed;      
    char textureLocation[256];       
    SDL_Texture* texture;    
    int frameWidth;          
    int frameHeight;         
    int totalFrames; 
    float animationTimer;
    float frameDelay;        
} Enemy;

typedef struct {
    float x, y;
    int width, height;
    bool collected;
} Collectible;

typedef struct {
    float x, y;  
    float speed; 
    bool active; 
    float dirX, dirY;
    float lifespan;
} Bullet;

typedef struct {
    float* sizes;
    float* offsets;
    int num_sizes;
    float sigma;
} HillNoise;

typedef struct {
    Shooter* shooters;
    Platform* platforms;
    int numPlatforms;
    Enemy* enemies1;
    int numEnemies1;
    Enemy* enemies2;
    int numEnemies2;
    Collectible* collectibles;
    int numCollectibles;
    Collectible* ammos;
    int numAmmos;
    Bullet bullets[100];
    float cameraX;
    int ammo;

    PauseButton* pauseButton;

    float deltaTime;
    Uint32 lastTime;
    bool isPaused;
    bool showSummaryWindow;
    bool showLevelSelection;
    bool quit;
    bool isPlayer1Turn;
    int selectedLevelIndex;
    char** levelFiles;
    int levelCount;

    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Texture* backgroundTexture;
    SDL_Texture* pauseTexture;
    SDL_Texture* bulletSpriteSheet;
} GameData;

bool init(GameData* g);
bool loadMedia(GameData* g);
void clear(GameData* g);
void initializeGame(GameData* state, const char* levelFile, int screen_width, int screen_height);
void cleanupGameState(GameData* state);

#endif
