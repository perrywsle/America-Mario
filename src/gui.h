#ifndef UI_H
#define UI_H

#include <SDL2/SDL.h>
#include "init.h"

void loadMainMenu(GameData* g, int screen_width, int screen_height);
void loadPause(GameData* g, int screen_width, int screen_height);
void loadSummary(GameData* g, int screen_width, int screen_height);
int loadLevelFiles(const char* folderPath, char*** levelFiles);
bool saveGame(GameData* state);
SaveFileInfo* loadSaveFiles(int* count);
void freeLevelFiles(char** levelFiles, int levelCount);

#endif