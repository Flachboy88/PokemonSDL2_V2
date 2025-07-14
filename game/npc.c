#include "npc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

// Update the NPC_Init function definition
bool NPC_Init(NPC *npc, SDL_Renderer *renderer, const char *spriteSheetPath,
              int spriteWidth, int spriteHeight, float x, float y,
              int hitboxWidth, int hitboxHeight, float speed)
{
    if (!Entity_Init(&npc->baseEntity, spriteWidth, spriteHeight, x, y, hitboxWidth, hitboxHeight, false))
    {
        fprintf(stderr, "Failed to initialize base entity for NPC!\\n");
        return false;
    }

    npc->speed = speed; // Initialize the speed member
    npc->actionTimer = 0.0f;
    npc->actionDuration = 0.0f;

    if (!Entity_AddSpriteSheet(&npc->baseEntity, renderer, spriteSheetPath, "DEFAULT", spriteWidth, spriteHeight))
    {
        fprintf(stderr, "Failed to add default spritesheet for NPC!\\n");
        return false;
    }

    Entity_AddAnimation(&npc->baseEntity, "idle_down", "DEFAULT", 0, 0, 1, 200, false);
    Entity_AddAnimation(&npc->baseEntity, "idle_left", "DEFAULT", 1, 0, 1, 200, false);
    Entity_AddAnimation(&npc->baseEntity, "idle_right", "DEFAULT", 2, 0, 1, 200, false);
    Entity_AddAnimation(&npc->baseEntity, "idle_top", "DEFAULT", 3, 0, 1, 200, false);

    Entity_SetAnimation(&npc->baseEntity, "idle_down");

    return true;
}

void NPC_Free(NPC *npc)
{
    if (npc)
    {
        Entity_Free(&npc->baseEntity);
    }
}

void NPC_Update(NPC *npc, float deltaTime)
{
    Entity_UpdateAnimation(&npc->baseEntity, deltaTime);

    npc->baseEntity.hitbox.x = (int)(npc->baseEntity.x + npc->baseEntity.spriteWidth / 2 - NPC_HITBOX_WIDTH / 2);
    npc->baseEntity.hitbox.y = (int)(npc->baseEntity.y + npc->baseEntity.spriteHeight - NPC_HITBOX_HEIGHT);
}

void NPC_Draw(NPC *npc, SDL_Renderer *renderer)
{
    Entity_Draw(&npc->baseEntity, renderer);
}

void NPC_AddAnimation(NPC *npc, const char *animationName, const char *spriteSheetName, int frameDurationMs, bool loop, int startRow, int startCol, int frameCount)
{
    Entity_AddAnimation(&npc->baseEntity, animationName, spriteSheetName, startRow, startCol, frameCount, frameDurationMs, loop);
}