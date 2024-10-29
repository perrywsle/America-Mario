#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "init.h"
#include "render.h"
#include "shooter.h"

GameData g;

int main(int argc, char* argv[]) {
    // Initialize SDL and other components
    if (!init()) {
        printf("Failed to initialize!\n");
        return 1;
    }

    if (!loadMedia()) {
        printf("Failed to load media!\n");
        return 1;
    }

    float terrainSizes[] = {50.0f, 100.0f, 200.0f};
    initHillNoise(hn, terrainSizes, sizeof(terrainSizes) / sizeof(terrainSizes[0]));

    SDL_Event e;
    initializeGame(&g);

    bool leftPressed = false;
    bool rightPressed = false;
    bool spacePressed = false;
    int mouseX, mouseY;
    
    while (true) {
        Uint32 currentTime = SDL_GetTicks();
        g.deltaTime = (currentTime - g.lastTime) / 1000.0f;
        g.lastTime = currentTime;

        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (e.type == SDL_QUIT) {
                g.quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    SDL_GetMouseState(&mouseX, &mouseY);
                    if (mouseX >= 700 && mouseX <= 750 && mouseY >= 50 && mouseY <= 100) {
                        g.isPaused = !g.isPaused;
                    } else if (!g.isPaused && !igGetIO()->WantCaptureMouse) {
                        shootBullet(g.bullets, g.shooterX, g.shooterY, 
                                    mouseX + g.cameraX, mouseY, &g.ammo);
                    }
                }
            } 
            
            // Move key event handling outside the mouse event block
            if (e.type == SDL_KEYDOWN) {
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


        // If quit event was triggered, exit the loop
        if (g.quit) break;

         // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        igNewFrame();

        // Render only if not paused
        if (!g.isPaused) {
            // Game logic update
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

            // Render game
            render(renderer, backgroundTexture, pauseTexture, font, hn);
        }
        
        if(g.isPaused) {
            // Store window flags
            ImGuiWindowFlags window_flags = 
                ImGuiWindowFlags_NoResize | 
                ImGuiWindowFlags_NoMove | 
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse |
                ImGuiWindowFlags_NoTitleBar;
            
            // Set window position to center of screen
            ImVec2 center = {SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f};
            ImVec2 window_size = {200.0f, 120.0f};
            ImVec2 window_pos = {
                center.x,
                center.y - window_size.y * 0.5f
            };
            
            // Set window properties
            igSetNextWindowPos(window_pos, ImGuiCond_Always, (ImVec2){0.5f, 0.5f});
            igSetNextWindowSize(window_size, ImGuiCond_Always);
            igPushStyleVar_Float(ImGuiStyleVar_Alpha, 1.0f);
            igPushStyleColor_Vec4(ImGuiCol_WindowBg, (ImVec4){0.0f, 0.0f, 0.0f, 0.9f});
            
            // Begin window with these flags
            igBegin("Pause Menu", NULL, window_flags);
            
            // Center text
            float window_width = igGetWindowWidth();
            ImVec2 button_size = {160.0f, 30.0f};
            
            // Center each button
            float center_pos_x = (window_width - button_size.x) * 0.5f;
            igSetCursorPosX(center_pos_x);
            if (igButton("Resume", button_size)) {
                g.isPaused = false;
                SDL_Delay(100);
            }
            
            igSetCursorPosX(center_pos_x);
            if (igButton("Restart", button_size)) {
                initializeGame(&g);
                g.isPaused = false;
                SDL_Delay(100);
            }
            
            igSetCursorPosX(center_pos_x);
            if (igButton("Exit", button_size)) {
                g.quit = true;
            }
            
            igEnd();
            
            // Pop style modifications
            igPopStyleVar(1);
            igPopStyleColor(1);
        }

        // Always render ImGui and end frame, even if no windows are shown
        igRender();
        ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
        
        // Swap the window buffer
        SDL_GL_SwapWindow(window);
        
    
        SDL_Delay(16); // Frame rate control

    }
    

    clear();
    freeHillNoise(hn);
    TTF_CloseFont(font);
    TTF_Quit();
    
    SDL_Quit();

    return 0;

}