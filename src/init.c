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

bool init(GameData* g) {
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

    g->window = SDL_CreateWindow(
        "America Mario",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP
    );
    
    if (g->window == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return false;
    }

    int screen_width, screen_height;
    
    SDL_GetWindowSize(g->window, &screen_width, &screen_height);

    // Create OpenGL context
    SDL_GLContext gl_context = SDL_GL_CreateContext(g->window);
    if (gl_context == NULL) {
        printf("OpenGL context creation failed: %s\n", SDL_GetError());
        return false;
    }
    
    SDL_GL_MakeCurrent(g->window, gl_context);
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
    SDL_GetWindowSize(g->window, &windowWidth, &windowHeight);
    ImGuiIO* io = igGetIO();
    io->DisplaySize = (ImVec2){(float)windowWidth, (float)windowHeight};
    ImGui_ImplSDL2_InitForOpenGL(g->window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 130");
    igStyleColorsDark(NULL);

    // Create renderer with OpenGL support
    g->renderer = SDL_CreateRenderer(g->window, -1, SDL_RENDERER_ACCELERATED);
    if (g->renderer == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return false;
    }

    if (TTF_Init() == -1) {
        printf("Failed to initialize SDL_ttf: %s\n", TTF_GetError());
        return false;
    }

    g->font = TTF_OpenFont("arial.ttf", 24);
    if (g->font == NULL) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return false;
    }

    return success;
}

bool loadMedia(GameData* g) {
    bool success = true;

    g->backgroundTexture = IMG_LoadTexture(g->renderer, "images/background.png");
    if (g->backgroundTexture == NULL) {
        printf("Failed to load background texture! SDL_image Error: %s\n", IMG_GetError());
        success = false;
    }

    SDL_Surface* imgPause;
    imgPause = IMG_Load("images/pause.png");
    if (imgPause == NULL)
    {
        printf("Failed to load pause image! SDL_image Error: %s\n", IMG_GetError());
        success = false;
    }
    g->pauseTexture = SDL_CreateTextureFromSurface(g->renderer, imgPause);
    if (g->pauseTexture == NULL) {
        printf("Failed to load pause texture! SDL_image Error: %s\n", IMG_GetError());
        success = false;
    }
    SDL_FreeSurface(imgPause);

    g->enemy1SpriteSheet = IMG_LoadTexture(g->renderer, "Assets/Characters/Enemies/Ghost/Spritesheets/ghost.png");
    if (!g->enemy1SpriteSheet) {
        printf("Error loading sprite sheet1\n");
        success = false;
    }
    g->enemy2SpriteSheet = IMG_LoadTexture(g->renderer, "Assets/Characters/Enemies/Crab/Spritesheets/crab-idle.png");
    if (!g->enemy2SpriteSheet) {
        printf("Error loading sprite sheet2\n");
        success = false;
    }
    g->bulletSpriteSheet = IMG_LoadTexture(g->renderer, "Assets/Fx/Spritesheets/player-shoot.png");
    if (!g->bulletSpriteSheet) {
        printf("Error loading bullet sprite sheet\n");
        success = false;
    }
    g->shooter1SpriteSheetIdle = IMG_LoadTexture(g->renderer, "Assets/Characters/Player/spritesheets/player-idle.png");
    if (!g->shooter1SpriteSheetIdle) {
        printf("Error loading shooter 1 idle sprite sheet\n");
        success = false;
    }
    g->shooter2SpriteSheetIdle = IMG_LoadTexture(g->renderer, "Assets/Characters/Player/spritesheets/player-run.png");
    if (!g->shooter2SpriteSheetIdle) {
        printf("Error loading shooter 1 idle sprite sheet\n");
        success = false;
    }

    return success;
}

void clear(GameData* g) {
    if (g->backgroundTexture != NULL) {
        SDL_DestroyTexture(g->backgroundTexture);
        g->backgroundTexture = NULL;
    }

    if (g->renderer != NULL) {
        SDL_DestroyRenderer(g->renderer);
        g->renderer = NULL;
    }

    if (g->window != NULL) {
        SDL_DestroyWindow(g->window);
        g->window = NULL;
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

    state->cameraX = 0.0f;
    state->deltaTime = (float)cJSON_GetObjectItem(root, "deltaTime")->valuedouble;
    state->lastTime = SDL_GetTicks();
    state->isPaused = false;
    state->showSummaryWindow = false;
    state->quit = false;
    state->isPlayer1Turn = cJSON_IsTrue(cJSON_GetObjectItem(root, "isPlayer1Turn"));
    printf("Game data loaded\n");

    // Load only the shooter for the current turn
    cJSON* shooters = cJSON_GetObjectItem(root, "shooters");
    state->shooters = (Shooter*)(malloc(2 * sizeof(Shooter)));
    for (int i = 0; i < 2; i++)
    {
        cJSON* shooterItem = cJSON_GetArrayItem(shooters, i);
        state->shooters[i].x = (float)cJSON_GetObjectItem(shooterItem, "x")->valuedouble;
        state->shooters[i].y = (float)cJSON_GetObjectItem(shooterItem, "y")->valuedouble;
        state->shooters[i].width = cJSON_GetObjectItem(shooterItem, "width")->valueint;
        state->shooters[i].height = cJSON_GetObjectItem(shooterItem, "height")->valueint;
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
        state->enemies1[i].texture = state->enemy1SpriteSheet;
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
        state->enemies2[i].texture = state->enemy2SpriteSheet;
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
