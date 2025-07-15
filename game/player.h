#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include "constante.h"
#include <math.h>

typedef enum
{
    PLAYER_STATE_IDLE,
    PLAYER_STATE_MOVING,
    PLAYER_STATE_FINISHING_MOVE
} PlayerState;

typedef enum
{
    DIRECTION_NONE = 0,
    DIRECTION_UP = 1,
    DIRECTION_DOWN = 2,
    DIRECTION_LEFT = 4,
    DIRECTION_RIGHT = 8
} Direction;

typedef struct
{
    Entity baseEntity;
    int speed;
    PlayerState state;
    Direction currentDirection;
    Direction targetDirection;
    Direction lastDirection;
    float targetX, targetY;
    bool hasTarget;
    bool wasMovingLastFrame;
} Player;

bool Player_Init(Player *player, SDL_Renderer *renderer, const char *spriteSheetPath,
                 int spriteWidth, int spriteHeight, float x, float y,
                 int hitboxWidth, int hitboxHeight);

void Player_Update(Player *player, float deltaTime);
void Player_Draw(Player *player, SDL_Renderer *renderer);
void Player_Free(Player *player);

void Player_HandleInput(Player *player);
bool Player_TryMove(Player *player, Direction direction, float newX, float newY);
Direction Player_GetInputDirection(void);
void Player_UpdateAnimation(Player *player);

#endif // PLAYER_H