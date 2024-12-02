#ifndef SHOOTER_H
#define SHOOTER_H

#include <stdbool.h>
#include "render.h"

void shootBullet(GameData* g, float targetX, float targetY);
void updateGame(GameData* g, HillNoise* hn, int screen_width, int screen_height, char** levelFiles, int selectedLevelIndex, bool leftPressed, bool rightPressed, bool spacePressed);

#endif
