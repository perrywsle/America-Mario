#include <gui.h>

void loadMainMenu(GameData* g, int screen_width, int screen_height) {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
                                    ImGuiWindowFlags_NoScrollWithMouse;

    float base_height = 100.0f; // Base height for title, spacing, and buttons
    float button_height = 30.0f;
    float spacing = igGetStyle()->ItemSpacing.y;
    float total_height = base_height + (g->levelCount + 1) * (button_height + spacing); // Levels

    int saveCount;
    SaveFileInfo* saves = loadSaveFiles(&saveCount);
    if (saveCount > 0) {
        total_height += saveCount * 2 * (button_height + spacing); // Save files
    } else {
        total_height += button_height + spacing; // "No saved games found"
    }

    ImVec2 window_size = {400.0f, total_height}; 
    ImVec2 center = {screen_width * 0.5f, screen_height * 0.5f};
    ImVec2 window_pos = {center.x, center.y - window_size.y * 0.5f};

    igSetNextWindowPos(window_pos, ImGuiCond_Always, (ImVec2){0.5f, 0.5f});
    igSetNextWindowSize(window_size, ImGuiCond_Always);
    igBegin("Main Menu", NULL, window_flags);

    // Title
    float window_width = igGetWindowWidth();
    ImVec2 button_size = {160.0f, button_height};
    float center_pos_x = (window_width - button_size.x) * 0.5f;

    ImVec2 title_size;
    igCalcTextSize(&title_size, "Select Level or Continue", NULL, false, -1.0f); 
    igSetCursorPosX((window_width - title_size.x) * 0.5f);
    igText("Select Level or Continue");

    igSpacing();

    // New Game Section
    igText("New Game:");
    igSpacing();
    for (int i = 0; i < g->levelCount; i++) {
        igSetCursorPosX(center_pos_x);
        char buttonLabel[32];
        snprintf(buttonLabel, sizeof(buttonLabel), "Level %d", i + 1);
        if (igButton(buttonLabel, button_size)) {
            g->selectedLevelIndex = i;
            initializeGame(g, g->levelFiles[g->selectedLevelIndex], screen_width, screen_height);
            g->showLevelSelection = false;
        }
    }

    igSpacing();
    igSeparator();
    igSpacing();

    // Load Game Section
    igText("Continue Saved Game:");
    igSpacing();
    if (saveCount > 0) {
        ImVec2 save_button_size = {320.0f, button_height};
        for (int i = 0; i < saveCount; i++) {
            igSetCursorPosX((window_width - save_button_size.x) * 0.5f);
            if (igButton(saves[i].displayName, save_button_size)) {
                initializeGame(g, saves[i].filename, screen_width, screen_height);
                g->showLevelSelection = false;
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
        g->quit = true;
    }

    igEnd();
}

void loadPause(GameData* g, int screen_width, int screen_height){
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
                                            ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar;

    ImVec2 center = {screen_width * 0.5f, screen_height * 0.5f};
    ImVec2 window_size = {200.0f, 160.0f}; 
    ImVec2 window_pos = {center.x, center.y - window_size.y * 0.5f};

    igSetNextWindowPos(window_pos, ImGuiCond_Always, (ImVec2){0.5f, 0.5f});
    igSetNextWindowSize(window_size, ImGuiCond_Always);
    igPushStyleVar_Float(ImGuiStyleVar_Alpha, 1.0f);
    igPushStyleColor_Vec4(ImGuiCol_WindowBg, (ImVec4){0.0f, 0.0f, 0.0f, 0.9f});

    igBegin("Pause Menu", NULL, window_flags);

    float window_width = igGetWindowWidth();
    ImVec2 button_size = {160.0f, 30.0f};
    float center_pos_x = (window_width - button_size.x) * 0.5f;

    // Buttons options
    igSetCursorPosX(center_pos_x);
    if (igButton("Resume", button_size)) {
        g->isPaused = false;
        SDL_Delay(100);
    }

    igSetCursorPosX(center_pos_x);
    if (igButton("Restart", button_size)) {
        initializeGame(g, g->levelFiles[g->selectedLevelIndex], screen_width, screen_height);
        g->isPaused = false;
        SDL_Delay(100);
    }

    igSetCursorPosX(center_pos_x);
    if (igButton("Save & Exit to Menu", button_size)) {
        if (saveGame(g)) {
            cleanupGameState(g);
            g->showLevelSelection= true;
            g->isPaused = false;
            g->quit = false;
            g->showSummaryWindow = false;

            SDL_Delay(100);
        }
    }

    igSetCursorPosX(center_pos_x);
    if (igButton("Exit to Main Menu", button_size)) {
        g->showLevelSelection = true;
        g->isPaused = false;
        g->quit = false;
        g->showSummaryWindow = false;
    }

    igEnd();
    igPopStyleVar(1);
    igPopStyleColor(1);
}

void loadSummary(GameData* g, int screen_width, int screen_height){
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
                                            ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar;

    ImVec2 center = {screen_width * 0.5f, screen_height * 0.5f};
    ImVec2 window_size = {300.0f, 300.0f};
    ImVec2 window_pos = {center.x, center.y - window_size.y * 0.5f};
    ImVec2 button_size = {150.0f, 30.0f};
    float window_width = igGetWindowWidth();
    float center_pos_x = (window_width - window_size.x) * 0.5f;

    igSetNextWindowPos(window_pos, ImGuiCond_Always, (ImVec2){0.5f, 0.5f});
    igSetNextWindowSize(window_size, ImGuiCond_Always);
    igPushStyleVar_Float(ImGuiStyleVar_Alpha, 1.0f);
    igPushStyleColor_Vec4(ImGuiCol_WindowBg, (ImVec4){0.0f, 0.0f, 0.0f, 0.9f});

    igBegin("Summary", NULL, window_flags);
    igSetCursorPosX(center_pos_x);
    igText("Player 1: Score: %d,\n Time: %.2fs,\n Health: %d\n", g->shooters[0].score, g->shooters[0].time, g->shooters[0].health);
    igSetCursorPosX(center_pos_x);
    igText("Player 2: Score: %d,\n Time: %.2fs,\n Health: %d\n", g->shooters[1].score, g->shooters[1].time, g->shooters[1].health);
    
    igSetCursorPosX(center_pos_x);
    // Conditions of winner
    // Whoever dies will lose
    if (g->shooters[0].health == 0 && g->shooters[1].health > 0) {
        igText("Player 2 wins!");
    } else if (g->shooters[1].health == 0 && g->shooters[0].health > 0) {
        igText("Player 1 wins!");
    } else if (g->shooters[0].health > 0 && g->shooters[1].health > 0) {
        // Both players are alive, compare scores
        if (g->shooters[0].score > g->shooters[1].score) {
            igText("Player 1 wins!");
        } else if (g->shooters[0].score < g->shooters[1].score) {
            igText("Player 2 wins!");
        } else {
            // Scores are tied, compare time
            if (g->shooters[0].time < g->shooters[1].time) {
                igText("Player 1 wins!");
            } else if (g->shooters[0].time > g->shooters[1].time) {
                igText("Player 2 wins!");
            } else {
                // Everything is tied
                igText("Tie!!");
            }
        }
    } else {
        // Both died
        igText("Tie!!");
    }


    igSetCursorPosX(center_pos_x);
    if (igButton("Next Level", button_size)) {
        g->showSummaryWindow = false;
        g->selectedLevelIndex += 1;
        cleanupGameState(g);
        initializeGame(g, g->levelFiles[g->selectedLevelIndex], screen_width, screen_height);
    }

    igSetCursorPosX(center_pos_x);
    if (igButton("Exit to main menu", button_size)) {
        g->showLevelSelection = true;
        g->isPaused = false;
        g->quit = false;
        g->showSummaryWindow = false;
    }

    igEnd();
    igPopStyleVar(1);
    igPopStyleColor(1);
}

int loadLevelFiles(const char* folderPath, char*** levelFiles) {
    struct dirent* ent; // directory entries
    DIR* dir = opendir(folderPath);

    if (!dir) {
        fprintf(stderr, "Failed to open directory: %s\n", folderPath);
        return -1;
    }

    int count = 0;
    size_t capacity = 10; // Initial capacity for the array
    *levelFiles = malloc(capacity * sizeof(char*));

    // read each entry until NULL
    while ((ent = readdir(dir)) != NULL) {
        // Check for ".json" files
        if (strstr(ent->d_name, ".json")) {
            // Ensure capacity is sufficient
            if (count >= capacity) {
                capacity *= 2;
                *levelFiles = realloc(*levelFiles, capacity * sizeof(char*));
            }

            // Build full file path
            size_t pathLength = strlen(folderPath) + strlen(ent->d_name) + 2; //2 is null terminator
            (*levelFiles)[count] = malloc(pathLength);
            snprintf((*levelFiles)[count], pathLength, "%s/%s", folderPath, ent->d_name);

            count++;
        }
    }

    closedir(dir);
    return count; 
}

void ensureSavesDirectoryExists() {
    // Just to make sure
    #ifdef _WIN32
        _mkdir("saves");
    #else
        mkdir("saves", 0777);
    #endif
}

bool saveGame(GameData* state) {
    ensureSavesDirectoryExists();
    
    // Generate timestamp for filename
    time_t now = time(NULL);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", localtime(&now));
    
    // Create filename
    char filename[512];
    snprintf(filename, sizeof(filename), "saves/save_%s.json", timestamp);
    
    // Create JSON object
    cJSON* root = cJSON_CreateObject();

    // Create an array for each entities
    cJSON* shooters = cJSON_CreateArray();
    for (int i = 0; i < 2; i++)
    {
        cJSON* shooter = cJSON_CreateObject();
        cJSON_AddNumberToObject(shooter, "x", state->shooters[i].x);
        cJSON_AddNumberToObject(shooter, "y", state->shooters[i].y);
        cJSON_AddNumberToObject(shooter, "width", state->shooters[i].width);
        cJSON_AddNumberToObject(shooter, "height", state->shooters[i].height);
        cJSON_AddNumberToObject(shooter, "health", state->shooters[i].health);
        cJSON_AddNumberToObject(shooter, "ammo", state->shooters[i].ammo);
        cJSON_AddNumberToObject(shooter, "score", state->shooters[i].score);
        cJSON_AddNumberToObject(shooter, "velocityY", state->shooters[i].velocityY);
        cJSON_AddNumberToObject(shooter, "time", state->shooters[i].time);
        cJSON_AddBoolToObject(shooter, "onGround", state->shooters[i].onGround);
        cJSON_AddStringToObject(shooter, "textureLocation", state->shooters[i].textureLocation);
        cJSON_AddNumberToObject(shooter, "currentFrame", state->shooters[i].currentFrame);
        cJSON_AddNumberToObject(shooter, "spriteWidth", state->shooters[i].frameWidth);
        cJSON_AddNumberToObject(shooter, "spriteHeight", state->shooters[i].frameHeight);
        cJSON_AddNumberToObject(shooter, "totalFrames", state->shooters[i].totalFrames);
        cJSON_AddNumberToObject(shooter, "animationTimer", 0.0);
        cJSON_AddNumberToObject(shooter, "frameDelay", state->shooters[i].frameDelay);
        cJSON_AddBoolToObject(shooter, "dead", state->shooters[i].dead);
        cJSON_AddItemToArray(shooters, shooter);
    }
    cJSON_AddItemToObject(root, "shooters", shooters);
    
    
    cJSON_AddNumberToObject(root, "deltaTime", state->deltaTime);
    cJSON_AddBoolToObject(root, "isPlayer1Turn", state->isPlayer1Turn);

    // Save platforms
    cJSON* platforms = cJSON_CreateArray();
    for (int i = 0; i < state->numPlatforms; i++) {
        cJSON* platform = cJSON_CreateObject();
        cJSON_AddNumberToObject(platform, "x", state->platforms[i].x);
        cJSON_AddNumberToObject(platform, "y", state->platforms[i].y);
        cJSON_AddNumberToObject(platform, "width", state->platforms[i].width);
        cJSON_AddNumberToObject(platform, "height", state->platforms[i].height);
        cJSON_AddItemToArray(platforms, platform);
    }
    cJSON_AddItemToObject(root, "platforms", platforms);
    
    // Save enemies1
    cJSON* enemies1 = cJSON_CreateArray();
    for (int i = 0; i < state->numEnemies1; i++) {
        cJSON* enemy = cJSON_CreateObject();
        cJSON_AddNumberToObject(enemy, "x", state->enemies1[i].x);
        cJSON_AddNumberToObject(enemy, "y", state->enemies1[i].y);
        cJSON_AddNumberToObject(enemy, "width", state->enemies1[i].width);
        cJSON_AddNumberToObject(enemy, "height", state->enemies1[i].height);
        cJSON_AddBoolToObject(enemy, "active", state->enemies1[i].active);
        cJSON_AddNumberToObject(enemy, "currentFrame", state->enemies1[i].currentFrame);
        cJSON_AddNumberToObject(enemy, "speed", state->enemies1[i].speed);
        cJSON_AddStringToObject(enemy, "textureLocation", state->enemies1[i].textureLocation);
        cJSON_AddNumberToObject(enemy, "spriteWidth", state->enemies1[i].frameWidth);
        cJSON_AddNumberToObject(enemy, "spriteHeight", state->enemies1[i].frameHeight);
        cJSON_AddNumberToObject(enemy, "totalFrames", state->enemies1[i].totalFrames);
        cJSON_AddNumberToObject(enemy, "animationTimer", 0.0);
        cJSON_AddNumberToObject(enemy, "frameDelay", state->enemies1[i].frameDelay);
        cJSON_AddItemToArray(enemies1, enemy);
    }
    cJSON_AddItemToObject(root, "enemies1", enemies1);
    
    // Save enemies2
    cJSON* enemies2 = cJSON_CreateArray();
    for (int i = 0; i < state->numEnemies2; i++) {
        cJSON* enemy = cJSON_CreateObject();
        cJSON_AddNumberToObject(enemy, "x", state->enemies2[i].x);
        cJSON_AddNumberToObject(enemy, "y", state->enemies2[i].y);
        cJSON_AddNumberToObject(enemy, "width", state->enemies2[i].width);
        cJSON_AddNumberToObject(enemy, "height", state->enemies2[i].height);
        cJSON_AddBoolToObject(enemy, "active", state->enemies2[i].active);
        cJSON_AddNumberToObject(enemy, "speed", state->enemies2[i].speed);
        cJSON_AddNumberToObject(enemy, "platformIndex", state->enemies2[i].platformIndex);
        cJSON_AddStringToObject(enemy, "textureLocation", state->enemies2[i].textureLocation);
        cJSON_AddNumberToObject(enemy, "spriteWidth", state->enemies2[i].frameWidth);
        cJSON_AddNumberToObject(enemy, "spriteHeight", state->enemies2[i].frameHeight);
        cJSON_AddNumberToObject(enemy, "totalFrames", state->enemies2[i].totalFrames);
        cJSON_AddNumberToObject(enemy, "currentFrame", state->enemies2[i].currentFrame);
        cJSON_AddNumberToObject(enemy, "animationTimer", 0.0);
        cJSON_AddNumberToObject(enemy, "frameDelay", state->enemies2[i].frameDelay);
        cJSON_AddItemToArray(enemies2, enemy);
    }
    cJSON_AddItemToObject(root, "enemies2", enemies2);
    
    // Save collectibles
    cJSON* collectibles = cJSON_CreateArray();
    for (int i = 0; i < state->numCollectibles; i++) {
        cJSON* collectible = cJSON_CreateObject();
        cJSON_AddNumberToObject(collectible, "x", state->collectibles[i].x);
        cJSON_AddNumberToObject(collectible, "y", state->collectibles[i].y);
        cJSON_AddNumberToObject(collectible, "width", state->collectibles[i].width);
        cJSON_AddNumberToObject(collectible, "height", state->collectibles[i].height);
        cJSON_AddBoolToObject(collectible, "collected", state->collectibles[i].collected);
        cJSON_AddItemToArray(collectibles, collectible);
    }
    cJSON_AddItemToObject(root, "collectibles", collectibles);
    
    // Save ammos
    cJSON* ammos = cJSON_CreateArray();
    for (int i = 0; i < state->numAmmos; i++) {
        cJSON* ammo = cJSON_CreateObject();
        cJSON_AddNumberToObject(ammo, "x", state->ammos[i].x);
        cJSON_AddNumberToObject(ammo, "y", state->ammos[i].y);
        cJSON_AddNumberToObject(ammo, "width", state->ammos[i].width);
        cJSON_AddNumberToObject(ammo, "height", state->ammos[i].height);
        cJSON_AddBoolToObject(ammo, "collected", state->ammos[i].collected);
        cJSON_AddItemToArray(ammos, ammo);
    }
    cJSON_AddItemToObject(root, "ammos", ammos);
    
    // Write JSON to file
    char* json_str = cJSON_Print(root);
    FILE* file = fopen(filename, "w");
    if (!file) {
        cJSON_Delete(root);
        free(json_str);
        return false;
    }
    
    fprintf(file, "%s", json_str);
    fclose(file);
    
    // Cleanup
    cJSON_Delete(root);
    free(json_str);
    
    return true;
}

SaveFileInfo* loadSaveFiles(int* count) {
    DIR* dir; 
    struct dirent* ent; // directory entries
    *count = 0;
    
    // count number of save files
    dir = opendir("saves");
    if (dir == NULL) {
        ensureSavesDirectoryExists();
        dir = opendir("saves");
        if (dir == NULL) return NULL;
    }
    
    while ((ent = readdir(dir)) != NULL) {
        if (strstr(ent->d_name, ".json") != NULL) {
            (*count)++;
        }
    }
    closedir(dir);
    
    if (*count == 0) return NULL;
    
    // Allocate array for save files
    SaveFileInfo* saves = (SaveFileInfo*)malloc(sizeof(SaveFileInfo) * (*count));
    
    // Fill array with save file info
    dir = opendir("saves");
    int i = 0;
    while ((ent = readdir(dir)) != NULL) {
        if (strstr(ent->d_name, ".json") != NULL) {
            snprintf(saves[i].filename, sizeof(saves[i].filename), "saves/%s", ent->d_name);
            
            // Extract date/time from filename
            char year[5], month[3], day[3], hour[3], min[3], sec[3];
            sscanf(ent->d_name, "save_%4s%2s%2s_%2s%2s%2s.json", 
                   year, month, day, hour, min, sec);
            
            // Store into array for display in load game
            snprintf(saves[i].displayName, sizeof(saves[i].displayName),
                    "Saved: %s/%s/%s %s:%s:%s",
                    year, month, day, hour, min, sec);
            
            i++;
        }
    }
    closedir(dir);
    
    return saves;
}

void freeLevelFiles(char** levelFiles, int levelCount){
    for (int i = 0; i < levelCount; i++) {
        free(levelFiles[i]);
    }
    free(levelFiles);
}