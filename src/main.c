#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "init.h"
#include "render.h"
#include "shooter.h"


void loadMainMenu(int screen_width, int screen_height, const char** levelFiles, int levelCount, int* selectedLevelIndex, bool* showLevelSelection){
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
                                ImGuiWindowFlags_NoScrollWithMouse;

    ImVec2 center = {screen_width * 0.5f, screen_height * 0.5f};
    ImVec2 window_size = {400.0f, 300.0f}; // Increased size for save files
    ImVec2 window_pos = {center.x, center.y - window_size.y * 0.5f};

    igSetNextWindowPos(window_pos, ImGuiCond_Always, (ImVec2){0.5f, 0.5f});
    igSetNextWindowSize(window_size, ImGuiCond_Always);

    igBegin("Main Menu", NULL, window_flags);

    float window_width = igGetWindowWidth();
    ImVec2 button_size = {160.0f, 30.0f};
    float center_pos_x = (window_width - button_size.x) * 0.5f;

    // Title
    ImVec2 title_size;
    igCalcTextSize(&title_size, "Select Level or Continue", NULL, false, -1.0f);
    igSetCursorPosX((window_width - title_size.x) * 0.5f);
    igText("Select Level or Continue");
    igSpacing();

    // New Game section
    igText("New Game:");
    igSpacing();
    for (int i = 0; i < levelCount; i++) {
        igSetCursorPosX(center_pos_x);
        char buttonLabel[32];
        snprintf(buttonLabel, sizeof(buttonLabel), "Level %d", i + 1);
        if (igButton(buttonLabel, button_size)) {
            *selectedLevelIndex = i;
            initializeGame(&g, levelFiles[*selectedLevelIndex], screen_width, screen_height);
            *showLevelSelection = false;
            break;
        }
    }

    igSpacing();
    igSeparator();
    igSpacing();

    // Load Game section
    igText("Continue Saved Game:");
    igSpacing();

    int saveCount;
    SaveFileInfo* saves = getSaveFiles(&saveCount);
    if (saves) {
        ImVec2 save_button_size = {320.0f, 30.0f};
        float save_center_pos_x = (window_width - save_button_size.x) * 0.5f;
        
        for (int i = 0; i < saveCount; i++) {
            igSetCursorPosX(save_center_pos_x);
            if (igButton(saves[i].displayName, save_button_size)) {
                initializeGame(&g, saves[i].filename, screen_width, screen_height);
                *showLevelSelection = false;
                break;
            }
        }
        free(saves);
    } else {
        igSetCursorPosX(center_pos_x);
        igTextDisabled("No saved games found");
    }

    igSpacing();
    igSeparator();
    igSpacing();

    igSetCursorPosX(center_pos_x);
    if (igButton("Exit", button_size)) {
        g.quit = true;
    }

    igEnd();
}

void updateGame(int screen_width, int screen_height, const char** levelFiles, int selectedLevelIndex, bool leftPressed, bool rightPressed, bool spacePressed){
    if (g.isPlayer1Turn) {
                updatePlayer(&g.shooters[0], leftPressed, rightPressed, spacePressed, screen_width, screen_height); 
                g.shooters[0].time += g.deltaTime;
                g.player1Score = g.shooters[0].score;
                g.player1Health = g.shooters[0].health;
                g.player1Time = g.shooters[0].time;
            } else {
                updatePlayer(&g.shooters[1], leftPressed, rightPressed, spacePressed, screen_width, screen_height);
                g.shooters[1].time += g.deltaTime;
            }

            if (!g.shooters[0].dead && g.isPlayer1Turn)
            {
                if (checkFinish(&g.shooters[g.isPlayer1Turn ? 0:1])) {
                    if (g.isPlayer1Turn) {
                        cleanupGameState(&g);
                        initializeGame(&g, levelFiles[selectedLevelIndex], screen_width, screen_height); 
                        g.isPlayer1Turn = false;
                        g.isPaused = true;
                        g.shooters[0].score = g.player1Score;
                        g.shooters[0].health = g.player1Health;
                        g.shooters[0].time = g.player1Time;
                    } else {
                        g.showSummaryWindow = true;
                        g.isPaused = true;
                    }
                }
            } else if (g.shooters[0].dead && g.isPlayer1Turn)
            {
                cleanupGameState(&g);
                initializeGame(&g, levelFiles[selectedLevelIndex], screen_width, screen_height); 
                g.isPlayer1Turn = false;
                g.isPaused = true;
                g.shooters[0].score = g.player1Score;
                g.shooters[0].health = g.player1Health;
                g.shooters[0].time = g.player1Time;
                g.shooters[0].dead = true;
            } else if (!g.shooters[1].dead && g.isPlayer1Turn==false)
            {
                if (checkFinish(&g.shooters[g.isPlayer1Turn ? 0:1])) {
                    if (g.isPlayer1Turn) {
                        cleanupGameState(&g);
                        initializeGame(&g, levelFiles[selectedLevelIndex], screen_width, screen_height); 
                        g.isPlayer1Turn = false;
                        g.isPaused = true;
                        g.shooters[0].score = g.player1Score;
                        g.shooters[0].health = g.player1Health;
                        g.shooters[0].time = g.player1Time;
                    } else {
                        g.showSummaryWindow = true;
                        g.isPaused = true;
                    }
                }
            } else if (g.shooters[1].dead && g.isPlayer1Turn==false)
            {
                g.showSummaryWindow = true;
                g.isPaused = true;
            }
            

            if (g.shooters[g.isPlayer1Turn? 0 : 1].y >= GROUND_LEVEL) {
                g.shooters[g.isPlayer1Turn? 0 : 1].y = GROUND_LEVEL;
                g.shooters[g.isPlayer1Turn? 0 : 1].velocityY = 0;
                g.shooters[g.isPlayer1Turn? 0 : 1].onGround = true;
            }

            if (g.shooters[g.isPlayer1Turn? 0 : 1].x < LEFT_BOUNDARY) {
                g.shooters[g.isPlayer1Turn? 0 : 1].x = LEFT_BOUNDARY; 
            }

            render(renderer, backgroundTexture, pauseTexture, font, hn, screen_width, screen_height);
}

void loadPause(int screen_width, int screen_height, const char** levelFiles, int selectedLevelIndex, bool* showLevelSelection){
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
                                            ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar;

    ImVec2 center = {screen_width * 0.5f, screen_height * 0.5f};
    ImVec2 window_size = {200.0f, 160.0f}; // Increased height for new button
    ImVec2 window_pos = {center.x, center.y - window_size.y * 0.5f};

    igSetNextWindowPos(window_pos, ImGuiCond_Always, (ImVec2){0.5f, 0.5f});
    igSetNextWindowSize(window_size, ImGuiCond_Always);
    igPushStyleVar_Float(ImGuiStyleVar_Alpha, 1.0f);
    igPushStyleColor_Vec4(ImGuiCol_WindowBg, (ImVec4){0.0f, 0.0f, 0.0f, 0.9f});

    igBegin("Pause Menu", NULL, window_flags);

    float window_width = igGetWindowWidth();
    ImVec2 button_size = {160.0f, 30.0f};
    float center_pos_x = (window_width - button_size.x) * 0.5f;

    igSetCursorPosX(center_pos_x);
    if (igButton("Resume", button_size)) {
        g.isPaused = false;
        SDL_Delay(100);
    }

    igSetCursorPosX(center_pos_x);
    if (igButton("Restart", button_size)) {
        initializeGame(&g, levelFiles[selectedLevelIndex], screen_width, screen_height);
        g.isPaused = false;
        SDL_Delay(100);
    }

    igSetCursorPosX(center_pos_x);
    if (igButton("Save & Exit to Menu", button_size)) {
        if (saveGame(&g)) {
            cleanupGameState(&g);
            *showLevelSelection = true;
            g.isPaused = false;
            g.quit = false;
            g.showSummaryWindow = false;
            
            SDL_Delay(100);
        }
    }

    igSetCursorPosX(center_pos_x);
    if (igButton("Exit to main menu", button_size)) {
        *showLevelSelection = true;
        g.isPaused = false;
        g.quit = false;
        g.showSummaryWindow = false;
    }

    igEnd();
    igPopStyleVar(1);
    igPopStyleColor(1);
}

void loadSummary(int screen_width, int screen_height, const char** levelFiles, int* selectedLevelIndex){
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
                                            ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar;

    ImVec2 center = {screen_width * 0.5f, screen_height * 0.5f};
    ImVec2 window_size = {600.0f, 300.0f};
    ImVec2 window_pos = {center.x, center.y - window_size.y * 0.5f};
    ImVec2 button_size = {160.0f, 30.0f};
    float window_width = igGetWindowWidth();
    float center_pos_x = (window_width - button_size.x) * 0.5f;

    igSetNextWindowPos(window_pos, ImGuiCond_Always, (ImVec2){0.5f, 0.5f});
    igSetNextWindowSize(window_size, ImGuiCond_Always);
    igPushStyleVar_Float(ImGuiStyleVar_Alpha, 1.0f);
    igPushStyleColor_Vec4(ImGuiCol_WindowBg, (ImVec4){0.0f, 0.0f, 0.0f, 0.9f});

    igBegin("Summary", NULL, window_flags);
    igSetCursorPosX(center_pos_x);
    igText("Player 1: Score: %d,\n Time: %.2fs,\n Health: %d\n", g.shooters[0].score, g.shooters[0].time, g.shooters[0].health);
    igSetCursorPosX(center_pos_x);
    igText("Player 2: Score: %d,\n Time: %.2fs,\n Health: %d\n", g.shooters[1].score, g.shooters[1].time, g.shooters[1].health);
    
    igSetCursorPosX(center_pos_x);
    if (g.shooters[0].health == 0)
    {
        igText("Player 2 wins!");
    } else if (g.shooters[1].health == 0)
    {
        igText("Player 1 wins!");
    } else {
        if (g.shooters[0].score > g.shooters[1].score)
        {
            igText("Player 1 wins!");
        } else if (g.shooters[0].score < g.shooters[1].score)
        {
            igText("Player 2 wins");
        } else
        {
            if (g.shooters[0].time < g.shooters[1].time)
            {
                igText("Player 1 wins!");
            } else if (g.shooters[0].time > g.shooters[1].time)
            {
                igText("Player 2 wins!");
            } else {
                igText("Tie!!");
            }
        }
        
    }

    igSetCursorPosX(center_pos_x);
    if (igButton("Next Level", button_size)) {
        g.showSummaryWindow = false;
        *selectedLevelIndex += 1;
        cleanupGameState(&g);
        initializeGame(&g, levelFiles[*selectedLevelIndex], screen_width, screen_height);
    }

    igSetCursorPosX(center_pos_x);
    if (igButton("Exit", button_size)) {
        g.quit = true;
    }

    igEnd();
    igPopStyleVar(1);
    igPopStyleColor(1);
}

int main(int argc, char* argv[]) {
    // Initialize SDL and other components
    if (!init()) {
        printf("Failed to initialize!\n");
        return 1;
    }

    int screen_width, screen_height;
    SDL_GetWindowSize(window, &screen_width, &screen_height);

    if (!loadMedia()) {
        printf("Failed to load media!\n");
        return 1;
    }

    float terrainSizes[] = {50.0f, 100.0f, 200.0f};
    initHillNoise(hn, terrainSizes, sizeof(terrainSizes) / sizeof(terrainSizes[0]));

    SDL_Event e;
    const char* levelFiles[] = {"level1.json", "level2.json"};
    int selectedLevelIndex = 0; 
    bool showLevelSelection = true;

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
                        shootBullet(&g.shooters[g.isPlayer1Turn ? 0 : 1], mouseX, mouseY);
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
        ImGui_ImplSDL2_NewFrame(window);
        igNewFrame();

        if (showLevelSelection) {
            loadMainMenu(screen_width, screen_height, levelFiles, (sizeof(levelFiles)/sizeof(levelFiles[0])), &selectedLevelIndex, &showLevelSelection);
        }
        if (!g.isPaused && !showLevelSelection) {
            updateGame(screen_width, screen_height, levelFiles, selectedLevelIndex, leftPressed, rightPressed, spacePressed);
        }
        if (g.isPaused && !g.showSummaryWindow) {
            loadPause(screen_width, screen_height, levelFiles, selectedLevelIndex, &showLevelSelection);
        }
        if (g.showSummaryWindow) {
            loadSummary(screen_width, screen_height, levelFiles, &selectedLevelIndex);
        }
        if (g.quit) break;

        igRender();
        ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
        SDL_GL_SwapWindow(window);
        SDL_Delay(1);
    }

    clear();
    freeHillNoise(hn);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    return 0;
}