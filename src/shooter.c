#include "shooter.h"

void shootBullet(Bullet bullets[], float shooterX, float shooterY, float targetX, float targetY, int* ammo) {
    for (int i = 0; i < *ammo; i++) {
        if (!bullets[i].active) {
            bullets[i].x = shooterX + 25; // Center bullet in shooter
            bullets[i].y = shooterY + 25; 
            
            float deltaX = targetX - bullets[i].x;
            float deltaY = targetY - bullets[i].y;
            float length = sqrtf(deltaX * deltaX + deltaY * deltaY);
            
            if (length != 0) { 
                bullets[i].dirX = deltaX / length;
                bullets[i].dirY = deltaY / length;
            } else {
                bullets[i].dirX = 1; // Default to shooting right if no direction
                bullets[i].dirY = 0;
            }

            bullets[i].speed = 500.0f;
            bullets[i].active = true;
            bullets[i].lifespan = 2.0f; // 2 seconds lifespan
            (*ammo)--;
            break;
        }
    }
}

bool collideFromAbove(float shooterY, float previousY, Platform platform) {
    return previousY + 50 >= platform.y && shooterY + 50 <= platform.y + platform.height;
}

bool collideFromBelow(float shooterY, float previousY, Platform platform) {
    return previousY <= platform.y + platform.height && shooterY >= platform.y;
}

bool checkCollectibleCollision(float shooterX, float shooterY, Collectible* collectible) {
    if (shooterX + 50 >= collectible->x && shooterX <= collectible->x + collectible->width &&
        shooterY + 50 >= collectible->y && shooterY <= collectible->y + collectible->height) {
        return true;
    }
    return false;
}

bool checkEnemyCollision(float shooterX, float shooterY, Enemy* enemy) {
    if (!enemy->active) return false; 

    return (shooterX + 50 >= enemy->x && shooterX <= enemy->x + enemy->width &&
            shooterY + 50 >= enemy->y && shooterY <= enemy->y + enemy->height);
}

bool checkBulletEnemyCollision(float bulletX, float bulletY, Enemy* enemy) {
    return (bulletX >= enemy->x && bulletX <= enemy->x + enemy->width &&
            bulletY >= enemy->y && bulletY <= enemy->y + enemy->height);
}

bool onSamePlatform(float shooterX, float shooterY, Platform platforms[]) {
    for (int i = 0; i < NUM_PLATFORM; i++) {
        if (shooterX >= platforms[i].x && 
            shooterX <= platforms[i].x + platforms[i].width &&
            shooterY >= platforms[i].y - 50) { // Player should be above the platform
            return true;
        }
    }
    return false;
}

void updateShooterPosition(float* shooterX, float* shooterY, float* velocityY, bool* onGround, float deltaTime, bool leftPressed, bool rightPressed, bool spacePressed, Platform platforms[]) {
    if (leftPressed) {
        *shooterX -= SHOOTER_SPEED * deltaTime;
    }
    if (rightPressed) {
        *shooterX += SHOOTER_SPEED * deltaTime;
    }

    if (spacePressed && *onGround) {
        *velocityY = -JUMP_SPEED; 
        *onGround = false;
    }

    if (!*onGround) {
        *velocityY += GRAVITY * deltaTime;
    }

    float previousY = *shooterY;
    *shooterY += *velocityY * deltaTime;

    bool collisionDetected = false;

    for (int i = 0; i < NUM_PLATFORM; i++) {
        Platform platform = platforms[i];

        if (*shooterX + 50 >= platform.x && *shooterX <= platform.x + platform.width) {
            // Check collision from above
            if (collideFromAbove(*shooterY, previousY, platform)) {
                if (*velocityY > 0) {
                    *shooterY = platform.y - 50; // Adjust position just above the platform
                    *velocityY = 0; // Reset vertical velocity
                    *onGround = true; // Set on ground flag
                    collisionDetected = true;
                }
            } 
            // Check collision from below
            else if (collideFromBelow(*shooterY, previousY, platform)) {
                if (*velocityY < 0) {
                    *shooterY = platform.y + platform.height; // Adjust position just below the platform
                    *velocityY = 0; // Reset vertical velocity
                    collisionDetected = true;
                }
            }
        }
    }

    // Ensure shooter doesn't fall below ground level
    if (!collisionDetected && *shooterY >= GROUND_LEVEL) {
        *shooterY = GROUND_LEVEL;
        *velocityY = 0;
        *onGround = true;
    }

    if (!collisionDetected && *shooterY < GROUND_LEVEL) {
        *onGround = false;
    }
}

void updateCollectibles(float shooterX, float shooterY, Collectible collectibles[], int* score) {
    for (int i = 0; i < NUM_COLLECTIBLES; i++) {
        if (!collectibles[i].collected && checkCollectibleCollision(shooterX, shooterY, &collectibles[i])) {
            collectibles[i].collected = true;
            *score += 10;
        }
    }
}

void updateAmmos(float shooterX, float shooterY, Collectible ammos[], int* ammo) {
    for (int i = 0; i < NUM_AMMOS; i++) {
        if (!ammos[i].collected && checkCollectibleCollision(shooterX, shooterY, &ammos[i])) {
            ammos[i].collected = true;
            *ammo += 3;
        }
    }
}

void updateEnemies(Enemy enemies1[], Enemy enemies2[], float shooterX, float shooterY, float deltaTime, Platform platforms[], float cameraX) {
    // Update for enemies1 (flying enemies)
    for (int i = 0; i < NUM_ENEMIES1; i++) {
        if (enemies1[i].active) {
            // Check if the enemy is on screen
            if (enemies1[i].x >= cameraX && enemies1[i].x <= cameraX + SCREEN_WIDTH) {
                float dx = shooterX - enemies1[i].x;
                float dy = shooterY - enemies1[i].y;
                float distance = sqrt(dx * dx + dy * dy);
                
                if (distance > 0) {
                    enemies1[i].x += (dx / distance) * enemies1[i].speed * deltaTime;
                    enemies1[i].y += (dy / distance) * enemies1[i].speed * deltaTime;
                }
                
                // Update animation frame
                enemies1[i].currentFrame = (enemies1[i].currentFrame + 1) % 4;
            }
        }
    }

    // Update for enemies2 (platform enemies)
    for (int i = 0; i < NUM_ENEMIES2; i++) {
        if (enemies2[i].active) {
            // Check if the enemy is on screen
            if (enemies2[i].x >= cameraX && enemies2[i].x <= cameraX + SCREEN_WIDTH) {
                // Find the correct platform for the enemy
                for (int j = 0; j < NUM_PLATFORM; j++) {
                    // Check if the enemy is within the bounds of this platform
                    if (enemies2[i].x >= platforms[j].x && enemies2[i].x <= platforms[j].x + platforms[j].width) {
                        // Align the enemy to the platform
                        enemies2[i].y = platforms[j].y - enemies2[i].height;

                        // Move horizontally towards the player
                        float targetX = shooterX;
                        float dx = targetX - enemies2[i].x;
                        float moveSpeed = enemies2[i].speed * deltaTime;

                        // Limit movement to within the platform's width
                        if (fabs(dx) > moveSpeed) {
                            enemies2[i].x += (dx > 0) ? moveSpeed : -moveSpeed;
                        } else {
                            enemies2[i].x = targetX;
                        }

                        // Ensure the enemy doesn't move beyond the platform's edges
                        enemies2[i].x = fmax(platforms[j].x, fmin(enemies2[i].x, platforms[j].x + platforms[j].width - enemies2[i].width));

                        // Stop checking other platforms once we've found the correct one
                        break;
                    }
                }
            }
        }
    }
}

void resetEnemies(Enemy enemies1[], Enemy enemies2[]) {
    // Reinitialize enemies1 and enemies2 just like in initializeGame
    Enemy initialEnemies1[NUM_ENEMIES1] = {
        {250, 400, 30, 30, true, 0, 100.0f},  // Reset to initial positions, speeds, etc.
        {1200, 300, 30, 30, true, 0, 100.0f},
        {1900, 200, 30, 30, true, 0, 100.0f}
    };

    Enemy initialEnemies2[NUM_ENEMIES2] = {
        {500, 470, 30, 30, true, 0, 100.0f},  // Reset to initial positions, speeds, etc.
        {1200, 370, 30, 30, true, 0, 100.0f},
        {1900, 270, 30, 30, true, 0, 100.0f}
    };

    // Copy the initial states back to enemies1 and enemies2
    memcpy(enemies1, initialEnemies1, sizeof(initialEnemies1));
    memcpy(enemies2, initialEnemies2, sizeof(initialEnemies2));
}

void handleEnemyCollisions(float* shooterX, float* shooterY, float* velocityY, bool* onGround, int* health, Enemy enemies1[], Enemy enemies2[],float* cameraX) {
    for (int i = 0; i < NUM_ENEMIES1; i++) {
        if (checkEnemyCollision(*shooterX, *shooterY, &enemies1[i])) {
            (*health)--;
            if (*health <= 0) {
                printf("Game Over!\n");
                SDL_Quit();
                exit(0); // Exit game if no hearts left
            }
            // Reset game state
            *shooterX = 0.0f;
            *shooterY = GROUND_LEVEL;
            *velocityY = 0.0f;
            *onGround = true; 
            *cameraX = 0.0f;
            resetEnemies(enemies1, enemies2);
            break; 
        }
    }
    for (int i = 0; i < NUM_ENEMIES2; i++) {
        if (checkEnemyCollision(*shooterX, *shooterY, &enemies2[i])) {
            (*health)--;
            if (*health <= 0) {
                printf("Game Over!\n");
                SDL_Quit();
                exit(0); // Exit game if no hearts left
            }
            // Reset game state
            *shooterX = 0.0f;
            *shooterY = GROUND_LEVEL;
            *velocityY = 0.0f;
            *onGround = true; 
            *cameraX = 0.0f;
            break; 
        }
    }
}

void updateBullets(float deltaTime, Enemy enemies1[], Enemy enemies2[], Bullet bullets[], float cameraX, int* ammo) {
    for (int i = 0; i < *ammo + 1; i++) {
        if (bullets[i].active) {
            bullets[i].x += bullets[i].dirX * bullets[i].speed * deltaTime;
            bullets[i].y += bullets[i].dirY * bullets[i].speed * deltaTime;
            bullets[i].lifespan -= deltaTime;

            if (bullets[i].lifespan <= 0 || 
                bullets[i].x - cameraX > SCREEN_WIDTH || bullets[i].x - cameraX < 0 || 
                bullets[i].y < 0 || bullets[i].y > SCREEN_HEIGHT) {
                bullets[i].active = false;
                continue;
            }

            // Check for collision with enemies
            for (int j = 0; j < NUM_ENEMIES1; j++) {
                if (checkBulletEnemyCollision(bullets[i].x, bullets[i].y, &enemies1[j])) {
                    enemies1[j].active = false; // Deactivate enemy on hit
                    bullets[i].active = false;  // Deactivate bullet on hit
                    break;
                }
            }
            for (int j = 0; j < NUM_ENEMIES2; j++) {
                if (checkBulletEnemyCollision(bullets[i].x, bullets[i].y, &enemies2[j])) {
                    enemies2[j].active = false; // Deactivate enemy on hit
                    bullets[i].active = false;  // Deactivate bullet on hit
                    break;
                }
            }

            // Deactivate bullet if it goes off-screen or out of bounds
            if (bullets[i].x > WORLD_WIDTH || bullets[i].x < 0 || 
                bullets[i].y < 0 || bullets[i].y > SCREEN_HEIGHT) {
                bullets[i].active = false;
            }
        }
    }
}