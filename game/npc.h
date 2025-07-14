#ifndef NPC_H
#define NPC_H

#include <SDL2/SDL.h>
#include "entity.h"
#include "constante.h"

typedef struct NPC
{
    Entity baseEntity;
    float speed;
    float actionTimer;
    float actionDuration;
    int currentAction;
    // ajouter name etc
} NPC;

bool NPC_Init(NPC *npc, SDL_Renderer *renderer, const char *spriteSheetPath,
              int spriteWidth, int spriteHeight, float x, float y,
              int hitboxWidth, int hitboxHeight, float speed);

void NPC_Free(NPC *npc);
void NPC_Update(NPC *npc, float deltaTime);
void NPC_Draw(NPC *npc, SDL_Renderer *renderer);

#endif // NPC_H