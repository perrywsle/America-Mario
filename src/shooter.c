#include "shooter.h"

// shoot bullet on mouse click
void shootBullet(GameData* g, float targetX, float targetY) {
    Shooter* shooter = &g->shooters[g->isPlayer1Turn? 0:1];
    // No shoot if no ammo
    if (shooter->ammo <= 0) return;

    // Adjust target position for camera
    targetX += g->cameraX;

    // Calculate direction from actual shooter position
    float shooterCenterX = shooter->x + 50; 
    float shooterCenterY = shooter->y + 50;
    
    // Account for camera position in both target and shooter positions
    float deltaX = (targetX) - (shooterCenterX);
    float deltaY = targetY - shooterCenterY;
    float length = sqrtf(deltaX * deltaX + deltaY * deltaY);
    
    // Normalize direction
    float dirX = (length != 0) ? deltaX / length : 1;
    float dirY = (length != 0) ? deltaY / length : 0;

    for (int i = 0; i < shooter->ammo; i++) { 
        if (!g->bullets[i].active) {
            // Initialize bullet at shooter's actual position
            g->bullets[i].x = shooterCenterX;  
            g->bullets[i].y = shooterCenterY;
            g->bullets[i].dirX = dirX;
            g->bullets[i].dirY = dirY;
            g->bullets[i].speed = 500.0f;
            g->bullets[i].active = true;
            g->bullets[i].lifespan = 1000.0f;
            shooter->ammo--;
            break;
        }
    }
}

bool collideFromLeft(float previousX, Platform platform, Shooter* shooter) {
    return previousX + 100 <= platform.x && shooter->x + 100 >= platform.x;
}

bool collideFromRight(float previousX, Platform platform, Shooter* shooter) {
    return previousX >= platform.x + platform.width && shooter->x <= platform.x + platform.width;
}

bool collideFromAbove(float previousY, Platform platform, Shooter* shooter) {
    return previousY + 100 >= platform.y && shooter->y + 100 <= platform.y + platform.height;
}

bool collideFromBelow(float previousY, Platform platform, Shooter* shooter) {
    return previousY <= platform.y + platform.height && shooter->y >= platform.y - platform.height;
}

bool checkCollectibleCollision(Shooter* shooter, Collectible* collectible) {
    bool horizontalOverlap = shooter->x + 100 >= collectible->x && 
                            shooter->x <= collectible->x + collectible->width;
    bool verticalOverlap = shooter->y + 100 >= collectible->y && 
                          shooter->y <= collectible->y + collectible->height;
    return horizontalOverlap && verticalOverlap;
}

bool checkEnemyCollision(Shooter* shooter, Enemy* enemy) {
    if (!enemy->active) return false;
    bool horizontalOverlap = shooter->x + 50 >= enemy->x && 
                            shooter->x <= enemy->x + enemy->width;
    bool verticalOverlap = shooter->y + 100 >= enemy->y && 
                          shooter->y <= enemy->y + enemy->height;
    return horizontalOverlap && verticalOverlap;
}

bool checkBulletEnemyCollision(float bulletX, float bulletY, Enemy* enemy) {
    bool horizontalOverlap = bulletX + 10 >= enemy->x && 
                            bulletX <= enemy->x + enemy->width;
    bool verticalOverlap = bulletY + 10 >= enemy->y && 
                          bulletY <= enemy->y + enemy->height;
    return horizontalOverlap && verticalOverlap;
}

void updateShooterPosition(GameData* g, bool leftPressed, bool rightPressed, bool spacePressed) {
    Shooter* shooter = &g->shooters[g->isPlayer1Turn? 0:1];
    float previousX = shooter->x;
    float previousY = shooter->y;
    
    // Store intended movement
    float intendedX = shooter->x;
    if (leftPressed) intendedX -= SHOOTER_SPEED * g->deltaTime;
    if (rightPressed) intendedX += SHOOTER_SPEED * g->deltaTime;

    // Apply horizontal movement first
    shooter->x = intendedX;

    // Check horizontal collisions
    for (int i = 0; i < g->numPlatforms; i++) {
        Platform platform = g->platforms[i];
        
        if (shooter->y + 100 >= platform.y && shooter->y <= platform.y + platform.height) {
            if (collideFromLeft(previousX, platform, shooter)) {
                shooter->x = platform.x - 100;
            } else if (collideFromRight(previousX, platform, shooter)) {
                shooter->x = platform.x + platform.width;
            }
        }
    }

    // Handle jumping and vertical movement
    if (spacePressed && shooter->onGround) {
        shooter->velocityY = -JUMP_SPEED;
        shooter->onGround = false;
    }

    if (!shooter->onGround) {
        shooter->velocityY += GRAVITY * g->deltaTime;
    }

    // Apply vertical movement
    shooter->y += shooter->velocityY * g->deltaTime;
    bool collisionDetected = false;

    // Check vertical collisions
    for (int i = 0; i < g->numPlatforms; i++) {
        Platform platform = g->platforms[i];

        if (shooter->x + 50 >= platform.x && shooter->x <= platform.x + platform.width) {
            if (collideFromAbove(previousY, platform, shooter) && shooter->velocityY > 0) {
                shooter->y = platform.y - 100;
                shooter->velocityY = 0;
                shooter->onGround = true;
                collisionDetected = true;
            } else if (collideFromBelow(previousY, platform, shooter) && shooter->velocityY < 0) {
                shooter->y = platform.y + platform.height;
                shooter->velocityY = 0;
                collisionDetected = true;
            }
        }
    }

    // Ground collision
    if (!collisionDetected && shooter->y >= GROUND_LEVEL) {
        shooter->y = GROUND_LEVEL;
        shooter->velocityY = 0;
        shooter->onGround = true;
    } else if (!collisionDetected && shooter->y < GROUND_LEVEL) {
        shooter->onGround = false;
    }
    if (shooter->x < LEFT_BOUNDARY) {
        shooter->x = LEFT_BOUNDARY; 
    }
}

void updateCollectibles(GameData* g) {
    Shooter* shooter = &g->shooters[g->isPlayer1Turn? 0:1];
    for (int i = 0; i < g->numCollectibles; i++) {
        if (!g->collectibles[i].collected && checkCollectibleCollision(shooter, &g->collectibles[i])) {
            g->collectibles[i].collected = true;
            shooter->score += 5;
        }
    }
}

void updateAmmos(GameData* g) {
    Shooter* shooter = &g->shooters[g->isPlayer1Turn? 0:1];
    for (int i = 0; i < g->numAmmos; i++) {
        if (!g->ammos[i].collected && checkCollectibleCollision(shooter, &g->ammos[i])) {
            g->ammos[i].collected = true;
            shooter->ammo += 3;
        }
    }
}

void updateEnemies(GameData* g, int screen_width) {
    Shooter* shooter = &g->shooters[g->isPlayer1Turn? 0:1];
    for (int i = 0; i < g->numEnemies1; i++) {
        if (g->enemies1[i].active) {
            // move towards shooter if in screen
            if (g->enemies1[i].x >= g->cameraX && g->enemies1[i].x <= g->cameraX + screen_width) {
                float dx = shooter->x - g->enemies1[i].x;
                float dy = shooter->y - g->enemies1[i].y;
                float distance = sqrt(dx * dx + dy * dy);
                
                if (distance > 0) {
                    g->enemies1[i].x += (dx / distance) * g->enemies1[i].speed * g->deltaTime;
                    g->enemies1[i].y += (dy / distance) * g->enemies1[i].speed * g->deltaTime;
                }
            }
        }
    }

    for (int i = 0; i < g->numEnemies2; i++) {
        if (g->enemies2[i].active) {
            // Check if enemy is on screen first
            if (g->enemies2[i].x >= g->cameraX && g->enemies2[i].x <= g->cameraX + screen_width) {
                // Handle movement if on a valid platform
                int pIndex = g->enemies2[i].platformIndex;
                if (pIndex >= 0 && pIndex < g->numPlatforms) {
                    // Platform-specific logic
                    g->enemies2[i].y = g->platforms[pIndex].y - g->enemies2[i].height;

                    float targetX = shooter->x;
                    float dx = targetX - g->enemies2[i].x;
                    float moveSpeed = g->enemies2[i].speed * g->deltaTime;

                    if (fabs(dx) > moveSpeed) {
                        g->enemies2[i].x += (dx > 0) ? moveSpeed : -moveSpeed;
                    }

                    // Restrict enemy movement to platform bounds
                    g->enemies2[i].x = fmax(g->platforms[pIndex].x, 
                                         fmin(g->enemies2[i].x, 
                                             g->platforms[pIndex].x + g->platforms[pIndex].width - g->enemies2[i].width));
                }
            }
        }
    }
}

void handleEnemyCollisions(GameData* g) {
    Shooter* shooter = &g->shooters[g->isPlayer1Turn? 0:1];
    for (int i = 0; i < g->numEnemies1; i++) {
        if (checkEnemyCollision(shooter, &g->enemies1[i])) {
            shooter->health--;
            if (shooter->health <= 0) {
                shooter->dead = true;
                return;
            }
            shooter->x = 0.0f;
            shooter->y = GROUND_LEVEL;
            shooter->velocityY = 0.0f;
            shooter->onGround = true; 
            g->cameraX = 0.0f;
            break; 
        }
    }
    
    for (int i = 0; i < g->numEnemies2; i++) {
        if (checkEnemyCollision(shooter, &g->enemies2[i])) {
            shooter->health--;
            if (shooter->health <= 0) {
                shooter->dead = true;
                return;
            }
            shooter->x = 0.0f;
            shooter->y = GROUND_LEVEL;
            shooter->velocityY = 0.0f;
            shooter->onGround = true; 
            g->cameraX = 0.0f;
            break; 
        }
    }
}

void updateBullets(GameData* g, int screen_width, int screen_height) {
    int totalBulletSlots = g->ammo + (g->numAmmos * 3);
    
    for (int i = 0; i < totalBulletSlots; i++) {
        if (g->bullets[i].active) {
            // Update bullet position
            g->bullets[i].x += g->bullets[i].dirX * g->bullets[i].speed * g->deltaTime;
            g->bullets[i].y += g->bullets[i].dirY * g->bullets[i].speed * g->deltaTime;
            g->bullets[i].lifespan -= g->deltaTime;

            // Check if bullet should be deactivated relative to camera position
            bool shouldDeactivate = 
                g->bullets[i].lifespan <= 0 || 
                (g->bullets[i].x - g->cameraX) > screen_width || 
                (g->bullets[i].x - g->cameraX) < 0 ||
                g->bullets[i].y > screen_height || 
                g->bullets[i].y < 0;

            if (shouldDeactivate) {
                g->bullets[i].active = false;
                continue;
            }

            // Check bullet-platform collision
            for (int j = 0; j < g->numPlatforms; j++) {
                Platform platform = g->platforms[j];
                if (g->bullets[i].x + 10 >= platform.x && 
                    g->bullets[i].x <= platform.x + platform.width &&
                    g->bullets[i].y + 10 >= platform.y && 
                    g->bullets[i].y <= platform.y + platform.height) {
                    g->bullets[i].active = false;
                    break;
                }
            }
        }
    }
}

void handleBulletEnemyCollisions(GameData* g) {
    Shooter* shooter = &g->shooters[g->isPlayer1Turn? 0:1];
    // Use same totalBulletSlots calculation as in updateBullets
    int totalBulletSlots = g->ammo + (g->numAmmos * 3);
    
    for (int i = 0; i < totalBulletSlots; i++) {
        if (g->bullets[i].active) {
            // Check collisions with type 1 enemies
            for (int j = 0; j < g->numEnemies1; j++) {
                if (g->enemies1[j].active && 
                    checkBulletEnemyCollision(g->bullets[i].x, g->bullets[i].y, &g->enemies1[j])) {
                    g->enemies1[j].active = false;
                    g->bullets[i].active = false;
                    shooter->score += 15;
                    break;
                }
            }
            
            // Only check type 2 enemies if bullet is still active
            if (g->bullets[i].active) {
                for (int k = 0; k < g->numEnemies2; k++) {
                    if (g->enemies2[k].active && 
                        checkBulletEnemyCollision(g->bullets[i].x, g->bullets[i].y, &g->enemies2[k])) {
                        g->enemies2[k].active = false;
                        g->bullets[i].active = false;
                        shooter->score += 10;
                        break;
                    }
                }
            }
        }
    }
}

bool checkFinish(GameData* g) {
    Shooter* shooter = &g->shooters[g->isPlayer1Turn? 0:1];
    return shooter->x + 100 >= WORLD_WIDTH;
}

void updatePlayer(GameData* g, Shooter* shooter, bool leftPressed, bool rightPressed, bool spacePressed, int screen_width, int screen_height) {
    updateShooterPosition(g, leftPressed, rightPressed, spacePressed);
    updateCollectibles(g);
    updateAmmos(g);
    updateEnemies(g, screen_width);
    handleEnemyCollisions(g);
    // Update camera position based on current shooter
    if (shooter->x >= screen_width / 2.0f) {
        g->cameraX = shooter->x - screen_width / 2.0f;
    }
    updateBullets(g, screen_width, screen_height);
    handleBulletEnemyCollisions(g);
}

void updateGame(GameData* g, HillNoise* hn, int screen_width, int screen_height, bool leftPressed, bool rightPressed, bool spacePressed) {
    static int player1Score, player1Health;
    static double player1Time;

    int currentPlayerIndex = g->isPlayer1Turn ? 0 : 1;
    Shooter* currentShooter = &g->shooters[currentPlayerIndex];

    // Update the current player's state
    updatePlayer(g, currentShooter, leftPressed, rightPressed, spacePressed, screen_width, screen_height);
    currentShooter->time += g->deltaTime;

    // Store Player 1's state to be retrieved after player 2 starts game
    if (g->isPlayer1Turn) {
        player1Score = currentShooter->score;
        player1Health = currentShooter->health;
        player1Time = currentShooter->time;
    }

    // Handle game completion or player death
    if (currentShooter->dead || checkFinish(g)) {
        if (g->isPlayer1Turn) {
            cleanupGameState(g);
            initializeGame(g, g->levelFiles[g->selectedLevelIndex], screen_width, screen_height);
            g->isPlayer1Turn = !g->isPlayer1Turn;
            g->isPaused = true;
            g->shooters[0].score = player1Score;
            g->shooters[0].health = player1Health;
            g->shooters[0].time = player1Time;
            if (currentShooter->dead) currentShooter->dead = true;
        } else {
            g->showSummaryWindow = true;
            g->isPaused = true;
        }
    }

    // Render the game state
    render(*g, g->renderer, g->font, hn, screen_width, screen_height);
}