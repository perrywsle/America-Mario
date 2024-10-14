#ifndef SHOOTER_H
#define SHOOTER_H

#include <stdbool.h>
#include "init.h"

void initBullets();
void shootBullet(Bullet bullets[], float shooterX, float shooterY, float targetX, float targetY, int* ammo);
void updateBullets(float deltaTime, Enemy enemies1[], Enemy enemies2[], Bullet bullets[], float cameraX, int* ammo);
void updateShooterPosition(float* shooterX, float* shooterY, float* velocityY, bool* onGround, float deltaTime, bool leftPressed, bool rightPressed, bool spacePressed, Platform platforms[]);
void updateCollectibles(float shooterX, float shooterY, Collectible collectibles[], int* score);
void updateAmmos(float shooterX, float shooterY, Collectible ammos[], int* ammo);
void updateEnemies(Enemy enemies1[], Enemy enemies2[], float shooterX, float shooterY, float deltaTime, Platform platforms[], float cameraX);
void handleEnemyCollisions(float* shooterX, float* shooterY, float* velocityY, bool* onGround, int* health, Enemy enemies1[], Enemy enemies2[], float* cameraX);

extern HillNoise* hn;

#endif
