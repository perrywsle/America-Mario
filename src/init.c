#include "init.h"
#include <SDL2/SDL_image.h>

#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include "GL/gl3w.h"    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* font = NULL;

SDL_Texture* backgroundTexture = NULL;
SDL_Texture* pauseTexture = NULL;
SDL_Texture* enemy1SpriteSheet = NULL;
SDL_Texture* enemy2SpriteSheet = NULL;
SDL_Texture* bulletSpriteSheet = NULL;
SDL_Texture* shooter1SpriteSheetIdle = NULL;
SDL_Texture* shooter2SpriteSheetIdle = NULL;

SDL_Surface* imgPause = NULL;

bool init() {
    bool success = true;
    
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }

    // Setup OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    window = SDL_CreateWindow(
        "Shooter Game with ImGui",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP
    );
    
    if (window == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return false;
    }

    int screen_width, screen_height;
    
    SDL_GetWindowSize(window, &screen_width, &screen_height);

    // Create OpenGL context
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL) {
        printf("OpenGL context creation failed: %s\n", SDL_GetError());
        return false;
    }
    
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

        // Initialize OpenGL loader
    #if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
    #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
    #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
    #endif
    if (err)
    {
        SDL_Log("Failed to initialize OpenGL loader!");
        return 1;
    }

    // Initialize ImGui
    igCreateContext(NULL);
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    ImGuiIO* io = igGetIO();
    io->DisplaySize = (ImVec2){(float)windowWidth, (float)windowHeight};
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 130");
    igStyleColorsDark(NULL);

    // Create renderer with OpenGL support
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return false;
    }

    if (TTF_Init() == -1) {
        printf("Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        return false;
    }

    font = TTF_OpenFont("arial.ttf", 24);
    if (font == NULL) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return false;
    }

    return success;
}

bool loadMedia() {
    bool success = true;

    backgroundTexture = IMG_LoadTexture(renderer, "images/background.png");
    if (backgroundTexture == NULL) {
        printf("Failed to load background texture! SDL_image Error: %s\n", IMG_GetError());
        success = false;
    }

    imgPause = IMG_Load("images/pause.png");
    if (imgPause == NULL)
    {
        printf("Failed to load pause image! SDL_image Error: %s\n", IMG_GetError());
        success = false;
    }
    pauseTexture = SDL_CreateTextureFromSurface(renderer, imgPause);
    if (pauseTexture == NULL) {
        printf("Failed to load pause texture! SDL_image Error: %s\n", IMG_GetError());
        success = false;
    }
    SDL_FreeSurface(imgPause);

    enemy1SpriteSheet = IMG_LoadTexture(renderer, "Assets/Characters/Enemies/Ghost/Spritesheets/ghost.png");
    if (!enemy1SpriteSheet) {
        printf("Error loading sprite sheet1\n");
        success = false;
    }
    enemy2SpriteSheet = IMG_LoadTexture(renderer, "Assets/Characters/Enemies/Crab/Spritesheets/crab-idle.png");
    if (!enemy2SpriteSheet) {
        printf("Error loading sprite sheet2\n");
        success = false;
    }
    bulletSpriteSheet = IMG_LoadTexture(renderer, "Assets/Fx/Spritesheets/player-shoot.png");
    if (!bulletSpriteSheet) {
        printf("Error loading bullet sprite sheet\n");
        success = false;
    }
    shooter1SpriteSheetIdle = IMG_LoadTexture(renderer, "Assets/Characters/Player/spritesheets/player-idle.png");
    if (!shooter1SpriteSheetIdle) {
        printf("Error loading shooter 1 idle sprite sheet\n");
        success = false;
    }
    shooter2SpriteSheetIdle = IMG_LoadTexture(renderer, "Assets/Characters/Player/spritesheets/player-run.png");
    if (!shooter2SpriteSheetIdle) {
        printf("Error loading shooter 1 idle sprite sheet\n");
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

void initializeGame(GameData* state, const char* levelFile, int screen_width, int screen_height) {
    // Open JSON file
    FILE* file = fopen(levelFile, "r");
    if (!file) {
        fprintf(stderr, "Error opening game_data.json\n");
        return;
    }

    // Read file content into a string
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* data = (char*)malloc(length + 1);
    fread(data, 1, length, file);
    data[length] = '\0';
    fclose(file);

    // Parse JSON data
    cJSON* root = cJSON_Parse(data);
    if (!root) {
        fprintf(stderr, "Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        free(data);
        return;
    }

        // Load only the shooter for the current turn
    cJSON* shooters = cJSON_GetObjectItem(root, "shooters");
    state->shooters = (Shooter*)(malloc(2 * sizeof(Shooter)));
    for (int i = 0; i < 2; i++)
    {
        cJSON* shooterItem = cJSON_GetArrayItem(shooters, i);
        state->shooters[i].x = (float)cJSON_GetObjectItem(shooterItem, "x")->valuedouble;
        state->shooters[i].y = (float)cJSON_GetObjectItem(shooterItem, "y")->valuedouble;
        state->shooters[i].velocityY = (float)cJSON_GetObjectItem(shooterItem, "velocityY")->valuedouble;
        state->shooters[i].health = cJSON_GetObjectItem(shooterItem, "health")->valueint;
        state->shooters[i].ammo = cJSON_GetObjectItem(shooterItem, "ammo")->valueint;
        state->shooters[i].score = cJSON_GetObjectItem(shooterItem, "score")->valueint;
        state->shooters[i].onGround = cJSON_IsTrue(cJSON_GetObjectItem(shooterItem, "onGround"));
        state->shooters[i].currentFrame = cJSON_GetObjectItem(shooterItem, "currentFrame")->valueint;
        state->shooters[i].frameWidth = cJSON_GetObjectItem(shooterItem, "spriteWidth")->valueint;
        state->shooters[i].frameHeight = cJSON_GetObjectItem(shooterItem, "spriteHeight")->valueint;
        state->shooters[i].totalFrames = cJSON_GetObjectItem(shooterItem, "totalFrames")->valueint;
        state->shooters[i].animationTimer = cJSON_GetObjectItem(shooterItem, "animationTimer")->valueint;
        state->shooters[i].frameDelay = cJSON_GetObjectItem(shooterItem, "frameDelay")->valueint;
        state->shooters[i].time = cJSON_GetObjectItem(shooterItem, "time")->valuedouble;
        state->shooters[i].dead = cJSON_IsTrue(cJSON_GetObjectItem(shooterItem, "dead"));
    }
    printf("Shooters data loaded\n");

    state->cameraX = 0.0f;
    state->deltaTime = (float)cJSON_GetObjectItem(root, "deltaTime")->valuedouble;
    state->lastTime = SDL_GetTicks();
    state->isPaused = false;
    state->showSummaryWindow = false;
    state->quit = false;
    state->isPlayer1Turn = cJSON_IsTrue(cJSON_GetObjectItem(root, "isPlayer1Turn"));
    printf("Game data loaded\n");

    // Load platforms
    cJSON* platforms = cJSON_GetObjectItem(root, "platforms");
    int numPlatforms = cJSON_GetArraySize(platforms);
    state->platforms = (Platform*)malloc(numPlatforms * sizeof(Platform));
    state->numPlatforms = numPlatforms;
    for (int i = 0; i < numPlatforms; i++) {
        cJSON* platformItem = cJSON_GetArrayItem(platforms, i);
        state->platforms[i].x = (float)cJSON_GetObjectItem(platformItem, "x")->valuedouble;
        state->platforms[i].y = (float)cJSON_GetObjectItem(platformItem, "y")->valuedouble;
        state->platforms[i].width = (float)cJSON_GetObjectItem(platformItem, "width")->valuedouble;
        state->platforms[i].height = (float)cJSON_GetObjectItem(platformItem, "height")->valuedouble;
    }
    printf("Platforms data loaded\n");

    // Load enemies1
    cJSON* enemies1 = cJSON_GetObjectItem(root, "enemies1");
    int numEnemies1 = cJSON_GetArraySize(enemies1);
    state->enemies1 = (Enemy*)malloc(numEnemies1 * sizeof(Enemy));
    state->numEnemies1 = numEnemies1;
    for (int i = 0; i < numEnemies1; i++) {
        cJSON* enemyItem = cJSON_GetArrayItem(enemies1, i);
        state->enemies1[i].x = (float)cJSON_GetObjectItem(enemyItem, "x")->valuedouble;
        state->enemies1[i].y = (float)cJSON_GetObjectItem(enemyItem, "y")->valuedouble;
        state->enemies1[i].width = cJSON_GetObjectItem(enemyItem, "width")->valueint;
        state->enemies1[i].height = cJSON_GetObjectItem(enemyItem, "height")->valueint;
        state->enemies1[i].active = cJSON_IsTrue(cJSON_GetObjectItem(enemyItem, "active"));
        state->enemies1[i].currentFrame = cJSON_GetObjectItem(enemyItem, "currentFrame")->valueint;
        state->enemies1[i].speed = (float)cJSON_GetObjectItem(enemyItem, "speed")->valuedouble;
        state->enemies1[i].texture = enemy1SpriteSheet;
        state->enemies1[i].frameWidth = cJSON_GetObjectItem(enemyItem, "spriteWidth")->valueint;
        state->enemies1[i].frameHeight = cJSON_GetObjectItem(enemyItem, "spriteHeight")->valueint;
        state->enemies1[i].totalFrames = cJSON_GetObjectItem(enemyItem, "totalFrames")->valueint;
        state->enemies1[i].animationTimer = cJSON_GetObjectItem(enemyItem, "animationTimer")->valueint;
        state->enemies1[i].frameDelay = cJSON_GetObjectItem(enemyItem, "frameDelay")->valueint;
    }
    printf("Enemy 1 data loaded\n");

    // Load enemies2 with platformIndex
    cJSON* enemies2 = cJSON_GetObjectItem(root, "enemies2");
    int numEnemies2 = cJSON_GetArraySize(enemies2);
    state->enemies2 = (Enemy*)malloc(numEnemies2 * sizeof(Enemy));
    state->numEnemies2 = numEnemies2;
    for (int i = 0; i < numEnemies2; i++) {
        cJSON* enemyItem = cJSON_GetArrayItem(enemies2, i);
        state->enemies2[i].x = (float)cJSON_GetObjectItem(enemyItem, "x")->valuedouble;
        state->enemies2[i].y = (float)cJSON_GetObjectItem(enemyItem, "y")->valuedouble;
        state->enemies2[i].width = (float)cJSON_GetObjectItem(enemyItem, "width")->valuedouble;
        state->enemies2[i].height = (float)cJSON_GetObjectItem(enemyItem, "height")->valuedouble;
        state->enemies2[i].active = cJSON_IsTrue(cJSON_GetObjectItem(enemyItem, "active"));
        state->enemies2[i].currentFrame = cJSON_GetObjectItem(enemyItem, "currentFrame")->valueint;
        state->enemies2[i].speed = (float)cJSON_GetObjectItem(enemyItem, "speed")->valuedouble;
        state->enemies2[i].platformIndex = cJSON_GetObjectItem(enemyItem, "platformIndex")->valueint;
        state->enemies2[i].texture = enemy2SpriteSheet;
        state->enemies2[i].frameWidth = cJSON_GetObjectItem(enemyItem, "spriteWidth")->valueint;
        state->enemies2[i].frameHeight = cJSON_GetObjectItem(enemyItem, "spriteHeight")->valueint;
        state->enemies2[i].totalFrames = cJSON_GetObjectItem(enemyItem, "totalFrames")->valueint;
        state->enemies2[i].animationTimer = cJSON_GetObjectItem(enemyItem, "animationTimer")->valueint;
        state->enemies2[i].frameDelay = cJSON_GetObjectItem(enemyItem, "frameDelay")->valueint;
    }
    printf("Enemy 2 data loaded\n");

    // Load collectibles
    cJSON* collectibles = cJSON_GetObjectItem(root, "collectibles");
    int numCollectibles = cJSON_GetArraySize(collectibles);
    state->collectibles = (Collectible*)malloc(numCollectibles * sizeof(Collectible));
    state->numCollectibles = numCollectibles;
    for (int i = 0; i < numCollectibles; i++) {
        cJSON* collectibleItem = cJSON_GetArrayItem(collectibles, i);
        state->collectibles[i].x = (float)cJSON_GetObjectItem(collectibleItem, "x")->valuedouble;
        state->collectibles[i].y = (float)cJSON_GetObjectItem(collectibleItem, "y")->valuedouble;
        state->collectibles[i].width = (float)cJSON_GetObjectItem(collectibleItem, "width")->valuedouble;
        state->collectibles[i].height = (float)cJSON_GetObjectItem(collectibleItem, "height")->valuedouble;
        state->collectibles[i].collected = cJSON_IsTrue(cJSON_GetObjectItem(collectibleItem, "collected"));
    }
    printf("Collectibles data loaded\n");

    // Load ammos
    cJSON* ammos = cJSON_GetObjectItem(root, "ammos");
    int numAmmos = cJSON_GetArraySize(ammos);
    state->ammos = (Collectible*)malloc(numAmmos * sizeof(Collectible));
    state->numAmmos = numAmmos;
    for (int i = 0; i < numAmmos; i++) {
        cJSON* ammoItem = cJSON_GetArrayItem(ammos, i);
        state->ammos[i].x = (float)cJSON_GetObjectItem(ammoItem, "x")->valuedouble;
        state->ammos[i].y = (float)cJSON_GetObjectItem(ammoItem, "y")->valuedouble;
        state->ammos[i].width = (float)cJSON_GetObjectItem(ammoItem, "width")->valuedouble;
        state->ammos[i].height = (float)cJSON_GetObjectItem(ammoItem, "height")->valuedouble;
        state->ammos[i].collected = cJSON_IsTrue(cJSON_GetObjectItem(ammoItem, "collected"));
    }
    printf("Ammos data loaded\n");

    // Free JSON resources
    cJSON_Delete(root);
    free(data);
}

void ensureSavesDirectoryExists() {
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

    cJSON* shooters = cJSON_CreateArray();
    for (int i = 0; i < 2; i++)
    {
        cJSON* shooter = cJSON_CreateObject();
        cJSON_AddNumberToObject(shooter, "x", state->shooters[i].x);
        cJSON_AddNumberToObject(shooter, "y", state->shooters[i].y);
        cJSON_AddNumberToObject(shooter, "health", state->shooters[i].health);
        cJSON_AddNumberToObject(shooter, "ammo", state->shooters[i].ammo);
        cJSON_AddNumberToObject(shooter, "score", state->shooters[i].score);
        cJSON_AddNumberToObject(shooter, "velocityY", state->shooters[i].velocityY);
        cJSON_AddNumberToObject(shooter, "time", state->shooters[i].time);
        cJSON_AddBoolToObject(shooter, "onGround", state->shooters[i].onGround);
        cJSON_AddNumberToObject(shooter, "currentFrame", state->shooters[i].currentFrame);
        cJSON_AddNumberToObject(shooter, "spriteWidth", state->shooters[i].frameWidth);
        cJSON_AddNumberToObject(shooter, "spriteHeight", state->shooters[i].frameHeight);
        cJSON_AddNumberToObject(shooter, "totalFrames", state->shooters[i].totalFrames);
        cJSON_AddNumberToObject(shooter, "animationTimer", 0.0);
        cJSON_AddNumberToObject(shooter, "frameDelay", 0.95);
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
        cJSON_AddNumberToObject(enemy, "spriteWidth", state->enemies1[i].frameWidth);
        cJSON_AddNumberToObject(enemy, "spriteHeight", state->enemies1[i].frameHeight);
        cJSON_AddNumberToObject(enemy, "totalFrames", state->enemies1[i].totalFrames);
        cJSON_AddNumberToObject(enemy, "animationTimer", 0.0);
        cJSON_AddStringToObject(enemy, "sprite", "enemy1SpriteSheet");
        cJSON_AddNumberToObject(enemy, "frameDelay", 0.95);
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
        cJSON_AddNumberToObject(enemy, "spriteWidth", state->enemies2[i].frameWidth);
        cJSON_AddNumberToObject(enemy, "spriteHeight", state->enemies2[i].frameHeight);
        cJSON_AddNumberToObject(enemy, "totalFrames", state->enemies2[i].totalFrames);
        cJSON_AddNumberToObject(enemy, "currentFrame", state->enemies2[i].currentFrame);
        cJSON_AddNumberToObject(enemy, "animationTimer", 0.0);
        cJSON_AddNumberToObject(enemy, "frameDelay", 0.95);
        cJSON_AddStringToObject(enemy, "sprite", "enemy2SpriteSheet");
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

SaveFileInfo* getSaveFiles(int* count) {
    DIR* dir;
    struct dirent* ent;
    *count = 0;
    
    // First count number of save files
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
    int index = 0;
    while ((ent = readdir(dir)) != NULL) {
        if (strstr(ent->d_name, ".json") != NULL) {
            snprintf(saves[index].filename, sizeof(saves[index].filename), "saves/%s", ent->d_name);
            
            // Extract date/time from filename (assuming format: save_YYYYMMDD_HHMMSS.json)
            char year[5], month[3], day[3], hour[3], min[3], sec[3];
            sscanf(ent->d_name, "save_%4s%2s%2s_%2s%2s%2s.json", 
                   year, month, day, hour, min, sec);
            
            snprintf(saves[index].displayName, sizeof(saves[index].displayName),
                    "Saved: %s/%s/%s %s:%s:%s",
                    year, month, day, hour, min, sec);
            
            index++;
        }
    }
    closedir(dir);
    
    return saves;
}

void cleanupGameState(GameData* state) {
    // Free allocated memory
    if (state->platforms) {
        free(state->platforms);
        state->platforms = NULL;
    }
    if (state->enemies1) {
        free(state->enemies1);
        state->enemies1 = NULL;
    }
    if (state->enemies2) {
        free(state->enemies2);
        state->enemies2 = NULL;
    }
    if (state->collectibles) {
        free(state->collectibles);
        state->collectibles = NULL;
    }
    
    // Reset state variables
    state->numPlatforms = 0;
    state->numEnemies1 = 0;
    state->numEnemies2 = 0;
    state->numCollectibles = 0;
    state->numAmmos = 0;
    state->isPaused = false;
    state->showSummaryWindow = false;

    state->cameraX = 0.0f;
}
