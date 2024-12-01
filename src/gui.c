#include <gui.h>

#define BUTTON_WIDTH 160.0f
#define BUTTON_HEIGHT 30.0f
#define WINDOW_ALPHA 1.0f
#define MENU_WINDOW_WIDTH 400.0f
#define MENU_WINDOW_HEIGHT 300.0f
#define PAUSE_WINDOW_WIDTH 200.0f
#define PAUSE_WINDOW_HEIGHT 160.0f
#define SUMMARY_WINDOW_WIDTH 600.0f
#define SUMMARY_WINDOW_HEIGHT 300.0f

// Helper functions for ImGui
void setupCenteredWindow(const char* title, ImVec2 size, ImGuiWindowFlags flags) {
    ImVec2 center = {screen_width * 0.5f, screen_height * 0.5f};
    ImVec2 window_pos = {center.x, center.y - size.y * 0.5f};
    
    igSetNextWindowPos(window_pos, ImGuiCond_Always, (ImVec2){0.5f, 0.5f});
    igSetNextWindowSize(size, ImGuiCond_Always);
    igPushStyleVar_Float(ImGuiStyleVar_Alpha, WINDOW_ALPHA);
    igPushStyleColor_Vec4(ImGuiCol_WindowBg, (ImVec4){0.0f, 0.0f, 0.0f, 0.9f});
}

bool createCenteredButton(const char* label, ImVec2 size) {
    float window_width = igGetWindowWidth();
    float center_pos_x = (window_width - size.x) * 0.5f;
    igSetCursorPosX(center_pos_x);
    return igButton(label, size);
}

void createCenteredText(const char* text, ...) {
    float window_width = igGetWindowWidth();
    ImVec2 text_size;
    va_list args;
    va_start(args, text);
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), text, args);
    va_end(args);
    
    igCalcTextSize(&text_size, buffer, NULL, false, -1.0f);
    igSetCursorPosX((window_width - text_size.x) * 0.5f);
    igText("%s", buffer);
}

void handleLevelCompletion(int* selectedLevelIndex, const char* levelFiles[], bool isPlayer1) {
    if (isPlayer1) {
        cleanupGameState(&g);
        initializeGame(&g, levelFiles[*selectedLevelIndex], screen_width, screen_height);
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