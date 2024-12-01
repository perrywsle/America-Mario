#ifndef INIT_H
#define INIT_H

#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
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
    char displayName[64];  // Will hold formatted date/time
} SaveFileInfo;

typedef struct {
    float x, y;
    float width, height;
} Platform;

typedef struct {
    float x, y;
    float velocityY;
    bool onGround;
    int health;
    int ammo;
    int score;
    double time;
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
    SDL_Texture* texture;
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

    float deltaTime;
    Uint32 lastTime;
    bool isPaused;
    bool showSummaryWindow;
    bool showLevelSelection;
    bool quit;
    bool isPlayer1Turn;

    int player1Score;
    int player1Health;
    double player1Time;
} GameData;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    SDL_Texture* backgroundTexture;
    SDL_Texture* pauseTexture;
    SDL_Texture* enemy1SpriteSheet;
    SDL_Texture* enemy2SpriteSheet;
    SDL_Texture* bulletSpriteSheet;
    SDL_Texture* shooter1SpriteSheetIdle;
    SDL_Texture* shooter2SpriteSheetIdle;
} App;

bool init();
bool loadMedia();
void clear();
void initializeGame(GameData* state, const char* levelFile, int screen_width, int screen_height);
bool saveGame(GameData* state);
void cleanupGameState(GameData* state);
SaveFileInfo* getSaveFiles(int* count);

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SDL_Texture* backgroundTexture;
extern SDL_Texture* pauseTexture;
extern SDL_Texture* enemy1SpriteSheet;
extern SDL_Texture* enemy2SpriteSheet;
extern SDL_Texture* bulletSpriteSheet;
extern SDL_Texture* shooter1SpriteSheetIdle;
extern SDL_Texture* shooter2SpriteSheetIdle;
extern TTF_Font* font;

#endif
