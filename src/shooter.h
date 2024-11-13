#ifndef SHOOTER_H
#define SHOOTER_H

#include <stdbool.h>
#include "init.h"

void shootBullet(Shooter* shooter, float targetX, float targetY);
bool checkFinish();
void updatePlayer(Shooter* shooter, bool leftPressed, bool rightPressed, bool spacePressed, int screen_width, int screen_height);

extern HillNoise* hn;

#endif
