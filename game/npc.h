#ifndef NPC_H
#define NPC_H

#include "entity.h"

typedef enum
{
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

typedef struct
{
    Entity baseEntity;   // L'entité de base encapsulée
    char name[64];       // Nom du PNJ
    float speed;         // Vitesse de déplacement du PNJ
    Direction direction; // Direction actuelle du PNJ

} NPC;

bool NPC_Init(NPC *npc, SDL_Renderer *renderer, const char *spriteSheetPath,
              int spriteWidth, int spriteHeight, float x, float y,
              int hitboxWidth, int hitboxHeight, bool traversable,
              const char *name, float speed, Direction direction);

void NPC_Update(NPC *npc);
void NPC_Draw(NPC *npc, SDL_Renderer *renderer);
void NPC_Free(NPC *npc);

#endif // NPC_H