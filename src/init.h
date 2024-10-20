#ifndef INIT_H
#define INIT_H

#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define WORLD_WIDTH 10000
#define SHOOTER_SPEED 200
#define JUMP_SPEED 400
#define GRAVITY 580
#define NUM_PLATFORM 3
#define NUM_ENEMIES1 3
#define NUM_ENEMIES2 3
#define NUM_COLLECTIBLES 3
#define NUM_AMMOS 3
#define BULLET_LIFESPAN 2.0f 
#define GROUND_LEVEL (SCREEN_HEIGHT - 50)
#define LEFT_BOUNDARY 0

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"


typedef struct {
    float x, y;
    float width, height;
} Platform;

typedef struct {
    float x, y;     
    float width, height; 
    bool active;    
    int currentFrame;
    float speed;
} Enemy;

typedef struct {
    float x;
    float y;
    int width;
    int height;
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
    float shooterX;
    float shooterY;
    float velocityY;
    bool onGround;
    int health;
    int ammo;
    int score;
    Platform platforms[NUM_PLATFORM];
    Enemy enemies1[NUM_ENEMIES1];
    Enemy enemies2[NUM_ENEMIES2];
    Collectible collectibles[NUM_COLLECTIBLES];
    Collectible ammos[NUM_AMMOS];
    Bullet bullets[10];
    float cameraX;

    float deltaTime;
    Uint32 lastTime;
    bool isPaused;
    bool quit;
    bool showDebugWindow;
} GameData;

extern GameData g;

bool init();
bool loadMedia();
void clear();
void initializeGame(GameData* state);

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern SDL_Texture* backgroundTexture;
extern TTF_Font* font;

#endif
