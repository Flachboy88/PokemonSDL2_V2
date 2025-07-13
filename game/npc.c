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

    // ... (rest of your NPC_Init function, adding animations etc.)
    Entity_AddAnimation(&npc->baseEntity, "idle_down", "DEFAULT", 0, 0, 1, 200, false);
    Entity_AddAnimation(&npc->baseEntity, "idle_left", "DEFAULT", 1, 0, 1, 200, false);
    Entity_AddAnimation(&npc->baseEntity, "idle_right", "DEFAULT", 2, 0, 1, 200, false);
    Entity_AddAnimation(&npc->baseEntity, "idle_top", "DEFAULT", 3, 0, 1, 200, false);

    Entity_AddAnimation(&npc->baseEntity, "walk_down", "DEFAULT", 0, 0, 4, 200, true);
    Entity_AddAnimation(&npc->baseEntity, "walk_left", "DEFAULT", 1, 0, 4, 200, true);
    Entity_AddAnimation(&npc->baseEntity, "walk_right", "DEFAULT", 2, 0, 4, 200, true);
    Entity_AddAnimation(&npc->baseEntity, "walk_top", "DEFAULT", 3, 0, 4, 150, true);

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
    Entity_UpdateAnimation(&npc->baseEntity);

    // Mettre à jour le timer
    npc->actionTimer += deltaTime;

    // Si l'action courante est terminée
    if (npc->actionTimer >= npc->actionDuration)
    {
        // Réinitialiser le timer
        npc->actionTimer = 0.0f;

        // Choisir une nouvelle action aléatoire
        int r = rand() % 5; // 0 à 4 : 4 directions + idle

        switch (r)
        {
        case 0:
            Entity_SetAnimation(&npc->baseEntity, "walk_down");
            npc->actionDuration = 1.0f + (rand() % 3); // 1 à 3 secondes
            break;
        case 1:
            Entity_SetAnimation(&npc->baseEntity, "walk_top");
            npc->actionDuration = 1.0f + (rand() % 3);
            break;
        case 2:
            Entity_SetAnimation(&npc->baseEntity, "walk_left");
            npc->actionDuration = 1.0f + (rand() % 3);
            break;
        case 3:
            Entity_SetAnimation(&npc->baseEntity, "walk_right");
            npc->actionDuration = 1.0f + (rand() % 3);
            break;
        case 4:
        default:
            Entity_SetAnimation(&npc->baseEntity, "idle_down");
            npc->actionDuration = 2.0f + (rand() % 4); // 2 à 5 secondes d'idle
            break;
        }
    }

    // Appliquer le déplacement selon l'animation courante
    const char *anim = npc->baseEntity.animations[npc->baseEntity.currentAnimationIndex].name;

    if (strcmp(anim, "walk_down") == 0)
    {
        npc->baseEntity.y += npc->speed * deltaTime;
    }
    else if (strcmp(anim, "walk_top") == 0)
    {
        npc->baseEntity.y -= npc->speed * deltaTime;
    }
    else if (strcmp(anim, "walk_left") == 0)
    {
        npc->baseEntity.x -= npc->speed * deltaTime;
    }
    else if (strcmp(anim, "walk_right") == 0)
    {
        npc->baseEntity.x += npc->speed * deltaTime;
    }

    // hitbox
    npc->baseEntity.hitbox.x = (int)(npc->baseEntity.x + npc->baseEntity.spriteWidth / 2 - NPC_HITBOX_WIDTH / 2);
    npc->baseEntity.hitbox.y = (int)(npc->baseEntity.y + npc->baseEntity.spriteHeight - NPC_HITBOX_HEIGHT);
}

void NPC_Draw(NPC *npc, SDL_Renderer *renderer)
{
    Entity_Draw(&npc->baseEntity, renderer);
}