#include "player.h"
#include <stdio.h>

const int LARGEUR_HITBOX = PLAYER_HITBOX_WIDTH;
const int HAUTEUR_HITBOX = PLAYER_HITBOX_HEIGHT;

bool Player_Init(Player *player, SDL_Renderer *renderer, const char *spriteSheetPath, int spriteWidth, int spriteHeight, float x, float y, int hitboxWidth, int hitboxHeight)
{
    if (!Entity_Init(&player->baseEntity, renderer, spriteSheetPath, spriteWidth, spriteHeight, x, y, hitboxWidth, hitboxHeight, true))
    {
        return false;
    }

    Entity_AddAnimation(&player->baseEntity, "idle_down", 0, 0, 4, 200, true);
    Entity_AddAnimation(&player->baseEntity, "walk_down", 1, 0, 4, 150, true);
    Entity_SetAnimation(&player->baseEntity, "idle_down");

    return true;
}
void Player_Update(Player *player)
{

    Entity_UpdateAnimation(&player->baseEntity);

    // Met à jour la hitbox
}

void Player_Draw(Player *player, SDL_Renderer *renderer)
{
    Entity_Draw(&player->baseEntity, renderer);
}

void Player_Free(Player *player)
{
    Entity_Free(&player->baseEntity);
}