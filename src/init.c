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
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    
    if (window == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return false;
    }

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
        printf("Error loading sprite sheet\n");
        success = false;
    }


    enemy2SpriteSheet = IMG_LoadTexture(renderer, "Assets/Characters/Enemies/Crab/Spritesheets/crab-walk.png");
    if (!enemy2SpriteSheet) {
        printf("Error loading sprite sheet\n");
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

void initializeGame(GameData* state) {
    state->shooterX = 0.0f;
    state->shooterY = GROUND_LEVEL;
    state->velocityY = 0.0f;
    state->onGround = true;
    state->cameraX = 0.0f;
    state->health = 3;
    state->ammo = 3;
    state->score = 0;

    state->deltaTime = 0.0f; 
    state->lastTime = SDL_GetTicks(); 
    state->isPaused = false; 
    state->quit = false; 
    state->showDebugWindow = false;

    Platform initialPlatforms[NUM_PLATFORM] = {
        {100, 500, 600, 30},
        {800, 400, 600, 30},
        {1500, 300, 600, 30}
    };

    // Initialize enemies
    Enemy initialEnemies1[NUM_ENEMIES1] = {
        {500, 400, 30, 30, true, 0, 100.0f, enemy1SpriteSheet, 30, 30, 4},
        {1200, 300, 30, 30, true, 0, 100.0f, enemy1SpriteSheet, 30, 30, 4},
        {1900, 200, 30, 30, true, 0, 100.0f, enemy1SpriteSheet, 30, 30, 4}
    };

    Enemy initialEnemies2[NUM_ENEMIES2] = {
        {250, 470, 30, 30, true, 0, 100.0f, enemy2SpriteSheet, 40, 30, 4},  
        {1200, 370, 30, 30, true, 0, 100.0f, enemy2SpriteSheet, 40, 30, 4},
        {1900, 270, 30, 30, true, 0, 100.0f, enemy2SpriteSheet, 40, 30, 4}
    };

    // Initialize collectibles
    Collectible initialCollectibles[NUM_COLLECTIBLES] = {
        {200, 400, 20, 20, false},
        {900, 350, 20, 20, false},
        {1700, 250, 20, 20, false}
    };

    Collectible initialAmmos[NUM_AMMOS] = {
        {200, 400, 20, 20, false},
        {900, 350, 20, 20, false},
        {1700, 250, 20, 20, false}
    };

    // Initialize bullets
    for (int i = 0; i < (state->ammo); i++) {
        state->bullets[i].active = false;
    }

    memcpy(state->platforms, initialPlatforms,sizeof(initialPlatforms));
    memcpy(state->enemies1, initialEnemies1,sizeof(initialEnemies1));
    memcpy(state->enemies2, initialEnemies2,sizeof(initialEnemies2));
    memcpy(state->collectibles, initialCollectibles,sizeof(initialCollectibles));
    memcpy(state->ammos , initialAmmos,sizeof(initialAmmos));
}
