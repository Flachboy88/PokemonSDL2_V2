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

    player->speed = PLAYER_SPEED;

    Entity_AddAnimation(&player->baseEntity, "idle_down", 0, 0, 4, 200, true);
    Entity_AddAnimation(&player->baseEntity, "walk_down", 1, 0, 4, 150, true);
    Entity_SetAnimation(&player->baseEntity, "idle_down");

    return true;
}
void Player_Update(Player *player)
{
    // mise à jour de l'animation
    Entity_UpdateAnimation(&player->baseEntity);

    // recalculer la hitbox
    player->baseEntity.hitbox.x = (int)(player->baseEntity.x + player->baseEntity.spriteWidth / 2 - LARGEUR_HITBOX / 2);
    player->baseEntity.hitbox.y = (int)(player->baseEntity.y + player->baseEntity.spriteHeight - HAUTEUR_HITBOX);
}

void Player_Draw(Player *player, SDL_Renderer *renderer)
{
    Entity_Draw(&player->baseEntity, renderer);
}

void Player_Free(Player *player)
{
    Entity_Free(&player->baseEntity);
}