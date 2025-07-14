#include "player.h"
#include <stdio.h>

const int LARGEUR_HITBOX = PLAYER_HITBOX_WIDTH;
const int HAUTEUR_HITBOX = PLAYER_HITBOX_HEIGHT;

bool Player_Init(Player *player, SDL_Renderer *renderer, const char *initialSpriteSheetPath,
                 int spriteWidth, int spriteHeight, float x, float y,
                 int hitboxWidth, int hitboxHeight)
{
    if (!Entity_Init(&player->baseEntity, spriteWidth, spriteHeight, x, y, hitboxWidth, hitboxHeight, true))
    {
        return false;
    }

    player->speed = PLAYER_SPEED;

    if (!Entity_AddSpriteSheet(&player->baseEntity, renderer, initialSpriteSheetPath, "WALK", spriteWidth, spriteHeight))
    {
        return false;
    }

    Entity_AddAnimation(&player->baseEntity, "idle_down", "WALK", 0, 0, 1, 200, false);
    Entity_AddAnimation(&player->baseEntity, "idle_left", "WALK", 1, 0, 1, 200, false);
    Entity_AddAnimation(&player->baseEntity, "idle_right", "WALK", 2, 0, 1, 200, false);
    Entity_AddAnimation(&player->baseEntity, "idle_top", "WALK", 3, 0, 1, 150, false);

    Entity_AddAnimation(&player->baseEntity, "walk_down", "WALK", 0, 0, 4, 200, true);
    Entity_AddAnimation(&player->baseEntity, "walk_left", "WALK", 1, 0, 4, 200, true);
    Entity_AddAnimation(&player->baseEntity, "walk_right", "WALK", 2, 0, 4, 200, true);
    Entity_AddAnimation(&player->baseEntity, "walk_top", "WALK", 3, 0, 4, 150, true);

    Entity_SetAnimation(&player->baseEntity, "idle_down");
    player->baseEntity.currentAnimation = &player->baseEntity.animations[0];

    return true;
}
void Player_Update(Player *player)
{
    // mise à jour de l'animation
    Entity_UpdateAnimation(&player->baseEntity);

    // recalculer la hitbox
    Entity *entity = &player->baseEntity;
    entity->hitbox.x = (int)(entity->x + entity->spriteWidth / 2 - LARGEUR_HITBOX / 2);
    entity->hitbox.y = (int)(entity->y + entity->spriteHeight - HAUTEUR_HITBOX);
}

void Player_Draw(Player *player, SDL_Renderer *renderer)
{
    Entity_Draw(&player->baseEntity, renderer);
}

void Player_Free(Player *player)
{
    Entity_Free(&player->baseEntity);
}