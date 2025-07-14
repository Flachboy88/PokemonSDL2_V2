#ifndef PLAYER_H
#define PLAYER_H

#include "entity.h"
#include "constante.h"

typedef struct
{
    Entity baseEntity;
    int speed;
} Player;

bool Player_Init(Player *player, SDL_Renderer *renderer, const char *spriteSheetPath,
                 int spriteWidth, int spriteHeight, float x, float y,
                 int hitboxWidth, int hitboxHeight);

void Player_Update(Player *player, float deltaTime);
void Player_Draw(Player *player, SDL_Renderer *renderer);
void Player_Free(Player *player);

#endif // PLAYER_H