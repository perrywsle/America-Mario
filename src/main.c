#include "init.h"
#include "gui.h"
#include "render.h"
#include "shooter.h"

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

    float terrainSizes[] = {50.0f, 100.0f, 200.0f};
    initHillNoise(hn, terrainSizes, sizeof(terrainSizes) / sizeof(terrainSizes[0]));

    SDL_Event e;
    g.levelFiles = NULL;
    g.levelCount = loadLevelFiles("levels", &g.levelFiles);

    if (g.levelCount < 0) {
        printf("Failed to load levels!\n");
        return 1;
    }

    g.showLevelSelection = true;
    g.selectedLevelIndex = 0;

    PauseButton pauseButton_instance = {
        .x = 1820,
        .y = 50,
        .width = 100.0f,
        .height = 100.0f
    };
    g.pauseButton = &pauseButton_instance;

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
                    if (mouseX >= g.pauseButton->x && mouseX <= g.pauseButton->x + g.pauseButton->width && mouseY >= g.pauseButton->y && mouseY <= g.pauseButton->y + g.pauseButton->height) {
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
            loadMainMenu(&g, screen_width, screen_height);
        }
        if (!g.isPaused && !g.showLevelSelection) {
            updateGame(&g, hn, screen_width, screen_height, leftPressed, rightPressed, spacePressed);
        }
        if (g.isPaused && !g.showSummaryWindow) {
            loadPause(&g, screen_width, screen_height);
        }
        if (g.showSummaryWindow) {
            loadSummary(&g, screen_width, screen_height);
        }
        if (g.quit) break;

        igRender();
        ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
        SDL_GL_SwapWindow(g.window);
        SDL_Delay(1);
    }

    clear(&g);
    freeHillNoise(hn);
    freeLevelFiles(g.levelFiles, g.levelCount);
    TTF_CloseFont(g.font);
    TTF_Quit();
    SDL_Quit();

    return 0;
}