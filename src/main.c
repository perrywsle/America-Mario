#include <stdio.h>
#include "init.h"
#include "gui.h"
#include "render.h"
#include "shooter.h"

void updateGame(GameData* g, HillNoise* hn, int screen_width, int screen_height, char** levelFiles, int selectedLevelIndex, bool leftPressed, bool rightPressed, bool spacePressed){
    int currentPlayerIndex = g->isPlayer1Turn ? 0:1;
    if (g->isPlayer1Turn) {
                updatePlayer(g, &g->shooters[0], leftPressed, rightPressed, spacePressed, screen_width, screen_height); 
                g->shooters[0].time += g->deltaTime;
                g->player1Score = g->shooters[0].score;
                g->player1Health = g->shooters[0].health;
                g->player1Time = g->shooters[0].time;
                printf("Player 1 score: %d", g->shooters[0].score);
            } else {
                updatePlayer(g, &g->shooters[1], leftPressed, rightPressed, spacePressed, screen_width, screen_height);
                g->shooters[1].time += g->deltaTime;
            }

            if (!g->shooters[0].dead && g->isPlayer1Turn)
            {
                if (checkFinish(g)) {
                    if (g->isPlayer1Turn) {
                        cleanupGameState(g);
                        initializeGame(g, levelFiles[selectedLevelIndex], screen_width, screen_height); 
                        g->isPlayer1Turn = false;
                        g->isPaused = true;
                        g->shooters[0].score = g->player1Score;
                        g->shooters[0].health = g->player1Health;
                        g->shooters[0].time = g->player1Time;
                    } else {
                        g->showSummaryWindow = true;
                        g->isPaused = true;
                    }
                }
            } else if (g->shooters[0].dead && g->isPlayer1Turn)
            {
                cleanupGameState(g);
                initializeGame(g, levelFiles[selectedLevelIndex], screen_width, screen_height); 
                g->isPlayer1Turn = false;
                g->isPaused = true;
                g->shooters[0].score = g->player1Score;
                g->shooters[0].health = g->player1Health;
                g->shooters[0].time = g->player1Time;
                g->shooters[0].dead = true;
            } else if (!g->shooters[1].dead && g->isPlayer1Turn==false)
            {
                if (checkFinish(g)) {
                    if (g->isPlayer1Turn) {
                        cleanupGameState(g);
                        initializeGame(g, levelFiles[selectedLevelIndex], screen_width, screen_height); 
                        g->isPlayer1Turn = false;
                        g->isPaused = true;
                        g->shooters[0].score = g->player1Score;
                        g->shooters[0].health = g->player1Health;
                        g->shooters[0].time = g->player1Time;
                    } else {
                        g->showSummaryWindow = true;
                        g->isPaused = true;
                    }
                }
            } else if (g->shooters[1].dead && g->isPlayer1Turn==false)
            {
                g->showSummaryWindow = true;
                g->isPaused = true;
            }
            

            if (g->shooters[currentPlayerIndex].y >= GROUND_LEVEL) {
                g->shooters[currentPlayerIndex].y = GROUND_LEVEL;
                g->shooters[currentPlayerIndex].velocityY = 0;
                g->shooters[currentPlayerIndex].onGround = true;
            }

            if (g->shooters[currentPlayerIndex].x < LEFT_BOUNDARY) {
                g->shooters[currentPlayerIndex].x = LEFT_BOUNDARY; 
            }

            render(*g, g->renderer, g->backgroundTexture, g->pauseTexture, g->font, hn, screen_width, screen_height);
}

int main(int argc, char* argv[]) {
    GameData g;
    HillNoise hn_instance = {
        .sizes = NULL, 
        .offsets = NULL,
        .num_sizes = 0,
        .sigma = 1.0f
    };
    HillNoise* hn = &hn_instance;
    // Initialize SDL and other components
    if (!init(&g)) {
        printf("Failed to initialize!\n");
        return 1;
    }

    int screen_width, screen_height;
    SDL_GetWindowSize(g.window, &screen_width, &screen_height);

    if (!loadMedia(&g)) {
        printf("Failed to load media!\n");
        return 1;
    }

    float terrainSizes[] = {50.0f, 100.0f, 200.0f};
    initHillNoise(hn, terrainSizes, sizeof(terrainSizes) / sizeof(terrainSizes[0]));

    SDL_Event e;
    char** levelFiles = NULL;
    int levelCount = loadLevelFiles("levels", &levelFiles);

    if (levelCount < 0) {
        printf("Failed to load levels!\n");
        return 1;
    }

    int selectedLevelIndex = 0; 

    g.showLevelSelection = true;

    bool leftPressed = false;
    bool rightPressed = false;
    bool spacePressed = false;
    int mouseX, mouseY;

    while (true) {
        uint64_t currentTime = SDL_GetTicks();
        g.deltaTime = (currentTime - g.lastTime) / 1000.0f;
        g.lastTime = currentTime;

        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);

            if (e.type == SDL_QUIT) {
                g.quit = true;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    SDL_GetMouseState(&mouseX, &mouseY);
                    if (mouseX >= 1820 && mouseX <= 1920 && mouseY >= 50 && mouseY <= 150) {
                        g.isPaused = !g.isPaused;
                    } else if (!g.isPaused && !igGetIO()->WantCaptureMouse) {
                        shootBullet(&g, mouseX, mouseY);
                    }
                }
            }
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

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(g.window);
        igNewFrame();

        if (g.showLevelSelection) {
            loadMainMenu(&g, screen_width, screen_height, levelFiles, levelCount, &selectedLevelIndex);
        }
        if (!g.isPaused && !g.showLevelSelection) {
            updateGame(&g, hn, screen_width, screen_height, levelFiles, selectedLevelIndex, leftPressed, rightPressed, spacePressed);
        }
        if (g.isPaused && !g.showSummaryWindow) {
            loadPause(&g, screen_width, screen_height, levelFiles, selectedLevelIndex);
        }
        if (g.showSummaryWindow) {
            loadSummary(&g, screen_width, screen_height, levelFiles, &selectedLevelIndex);
        }
        if (g.quit) break;

        igRender();
        ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
        SDL_GL_SwapWindow(g.window);
        SDL_Delay(1);
    }

    clear(&g);
    freeHillNoise(hn);
    for (int i = 0; i < levelCount; i++) {
        free(levelFiles[i]);
    }
    free(levelFiles);
    TTF_CloseFont(g.font);
    TTF_Quit();
    SDL_Quit();

    return 0;
}