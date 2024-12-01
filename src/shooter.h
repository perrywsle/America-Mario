#ifndef SHOOTER_H
#define SHOOTER_H

#include <stdbool.h>
#include "init.h"

void shootBullet(GameData* g, float targetX, float targetY);
bool checkFinish(GameData* g);
void updatePlayer(GameData* g, Shooter* shooter, bool leftPressed, bool rightPressed, bool spacePressed, int screen_width, int screen_height);

#endif
