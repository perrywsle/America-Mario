#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "init.h"
#include "render.h"
#include "shooter.h"

GameData g;

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (!init()) {
        printf("Failed to initialize SDL!\n");
        return 1;
    }

    // Load media
    if (!loadMedia()) {
        printf("Failed to load media!\n");
        return 1;
    }
    
    // Initialize terrain sizes
    float terrainSizes[] = {50.0f, 100.0f, 200.0f}; 
    initHillNoise(hn, terrainSizes, sizeof(terrainSizes) / sizeof(terrainSizes[0]));

    SDL_Event e;
    // setup imgui
    igCreateContext(NULL);
    ImGuiIO io = *igGetIO();
    printf("Display Size: %f x %f\n", io.DisplaySize.x, io.DisplaySize.y);

    // Initialize game data
    initializeGame(&g);

    // Key states
    bool leftPressed = false;
    bool rightPressed = false;
    bool spacePressed = false;
    int mouseX, mouseY;

    // SDL rendering loop
    while (!g.quit) {
        Uint32 currentTime = SDL_GetTicks();
        g.deltaTime = (currentTime - g.lastTime) / 1000.0f;
        g.lastTime = currentTime;

        // Handle SDL events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                g.quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    // Get the mouse position
                    SDL_GetMouseState(&mouseX, &mouseY);
                    shootBullet(g.bullets, g.shooterX, g.shooterY, mouseX + g.cameraX, mouseY, &g.ammo);  // Adjust for camera
                }
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                    case SDLK_a:
                        leftPressed = true;
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        rightPressed = true;
                        break;
                    case SDLK_SPACE:
                    case SDLK_w:
                        spacePressed = true;
                        break;
                }
            } else if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                    case SDLK_a:
                        leftPressed = false;
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        rightPressed = false;
                        break;
                    case SDLK_SPACE:
                    case SDLK_w:
                        spacePressed = false;
                        break;
                }
            }
        }

        // Update game objects if the game is not paused
        if (!g.isPaused) {
            updateShooterPosition(&g.shooterX, &g.shooterY, &g.velocityY, &g.onGround, g.deltaTime, leftPressed, rightPressed, spacePressed, g.platforms);
            updateCollectibles(g.shooterX, g.shooterY, g.collectibles, &g.score);
            updateAmmos(g.shooterX, g.shooterY, g.ammos, &g.ammo);
            updateEnemies(g.enemies1, g.enemies2, g.shooterX, g.shooterY, g.deltaTime, g.platforms, g.cameraX);
            handleEnemyCollisions(&g.shooterX, &g.shooterY, &g.velocityY, &g.onGround, &g.health, g.enemies1, g.enemies2, &g.cameraX);
            updateBullets(g.deltaTime, g.enemies1, g.enemies2, g.bullets, g.cameraX, &g.ammo);

            // Ground boundary check
            if (g.shooterY >= GROUND_LEVEL) {
                g.shooterY = GROUND_LEVEL;
                g.velocityY = 0;
                g.onGround = true;
            }

            // Left boundary check
            if (g.shooterX < LEFT_BOUNDARY) {
                g.shooterX = LEFT_BOUNDARY; 
            }

            // Camera control
            if (g.shooterX >= SCREEN_WIDTH / 2.0f) {
                g.cameraX = g.shooterX - SCREEN_WIDTH / 2.0f;
            }
        }

        // Render the game
        render(renderer, 
               backgroundTexture, 
               g.platforms, 
               g.collectibles, 
               g.ammos, 
               g.bullets, 
               g.enemies1, 
               g.enemies2, 
               g.shooterY, 
               g.cameraX, 
               g.shooterX, 
               &g.score, 
               &g.health, 
               &g.ammo, 
               font, 
               hn);

        SDL_Delay(16);  // Cap the frame rate to ~60 FPS
    }

    // Clean up
    clear();
    freeHillNoise(hn);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    IMG_Quit();

    return 0;
}
