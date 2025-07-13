#include "entity.h"
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Fonctions d'aide internes ---

static int findAnimationIndex(Entity *entity, const char *animationName)
{
    for (int i = 0; i < entity->animationCount; ++i)
    {
        if (strcmp(entity->animations[i].name, animationName) == 0)
        {
            return i;
        }
    }
    return -1;
}

bool Entity_Init(Entity *entity, SDL_Renderer *renderer, const char *spriteSheetPath, int spriteWidth, int spriteHeight, float x, float y, int hitboxWidth, int hitboxHeight, bool traversable)
{
    memset(entity, 0, sizeof(Entity)); // Initialise à zéro

    entity->x = x;
    entity->y = y;
    entity->hitbox.x = x + spriteWidth / 2 - hitboxWidth / 2;
    entity->hitbox.y = y + spriteHeight - hitboxHeight;
    entity->hitbox.w = hitboxWidth;
    entity->hitbox.h = hitboxHeight;
    entity->traversable = traversable;

    // Chargement de la feuille de sprites
    SDL_Surface *surface = IMG_Load(spriteSheetPath);
    if (!surface)
    {
        fprintf(stderr, "Erreur de chargement de la feuille de sprites %s: %s\n", spriteSheetPath, IMG_GetError());
        return false;
    }
    entity->spriteSheet.texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!entity->spriteSheet.texture)
    {
        fprintf(stderr, "Erreur de création de la texture pour %s: %s\n", spriteSheetPath, SDL_GetError());
        SDL_FreeSurface(surface);
        return false;
    }
    entity->spriteSheet.sheetWidth = surface->w;
    entity->spriteSheet.sheetHeight = surface->h;
    entity->spriteSheet.spriteWidth = spriteWidth;
    entity->spriteSheet.spriteHeight = spriteHeight;
    SDL_FreeSurface(surface);

    entity->animations = NULL;
    entity->animationCount = 0;
    entity->currentAnimationIndex = -1;
    entity->currentFrameIndex = 0;
    entity->lastFrameTime = SDL_GetTicks();
    entity->animationPaused = false;

    return true;
}

void Entity_AddAnimation(Entity *entity, const char *animationName,
                         int startRow, int startCol, int frameCount,
                         int frameDurationMs, bool loop)
{
    if (findAnimationIndex(entity, animationName) != -1)
    {
        fprintf(stderr, "L'animation '%s' existe déjà pour cette entité.\n", animationName);
        return;
    }

    entity->animationCount++;
    entity->animations = (Animation *)realloc(entity->animations, entity->animationCount * sizeof(Animation));
    if (!entity->animations)
    {
        fprintf(stderr, "Erreur d'allocation mémoire pour les animations.\n");
        exit(EXIT_FAILURE);
    }

    Animation *newAnim = &entity->animations[entity->animationCount - 1];
    strncpy(newAnim->name, animationName, sizeof(newAnim->name) - 1);
    newAnim->name[sizeof(newAnim->name) - 1] = '\0';
    newAnim->frameCount = frameCount;
    newAnim->frameDurationMs = frameDurationMs;
    newAnim->loop = loop;
    newAnim->frames = (Frame *)malloc(frameCount * sizeof(Frame));
    if (!newAnim->frames)
    {
        fprintf(stderr, "Erreur d'allocation mémoire pour les cadres de l'animation.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < frameCount; ++i)
    {
        newAnim->frames[i].x = (startCol + i) * entity->spriteSheet.spriteWidth;
        newAnim->frames[i].y = startRow * entity->spriteSheet.spriteHeight;
        newAnim->frames[i].w = entity->spriteSheet.spriteWidth;
        newAnim->frames[i].h = entity->spriteSheet.spriteHeight;

        // Assurez-vous que les cadres ne dépassent pas la taille de la feuille de sprites
        if (newAnim->frames[i].x + newAnim->frames[i].w > entity->spriteSheet.sheetWidth ||
            newAnim->frames[i].y + newAnim->frames[i].h > entity->spriteSheet.sheetHeight)
        {
            fprintf(stderr, "Avertissement: Le cadre %d de l'animation '%s' dépasse la feuille de sprites.\n", i, animationName);
            // On peut choisir d'ajuster ou d'ignorer, ici on signale juste.
        }
    }

    // Si c'est la première animation ajoutée, la définir comme courante
    if (entity->animationCount == 1)
    {
        entity->currentAnimationIndex = 0;
        entity->currentFrameIndex = 0;
        entity->lastFrameTime = SDL_GetTicks();
    }
}

void Entity_SetAnimation(Entity *entity, const char *animationName)
{
    int newIndex = findAnimationIndex(entity, animationName);
    if (newIndex != -1 && newIndex != entity->currentAnimationIndex)
    {
        entity->currentAnimationIndex = newIndex;
        entity->currentFrameIndex = 0;          // Réinitialise le cadre au début de la nouvelle animation
        entity->lastFrameTime = SDL_GetTicks(); // Réinitialise le temps du dernier cadre
    }
    else if (newIndex == -1)
    {
        fprintf(stderr, "Animation '%s' non trouvée pour l'entité.\n", animationName);
    }
}

void Entity_UpdateAnimation(Entity *entity)
{
    if (entity->animationPaused || entity->currentAnimationIndex == -1)
    {
        return;
    }

    Animation *currentAnim = &entity->animations[entity->currentAnimationIndex];
    Uint32 currentTime = SDL_GetTicks();

    if (currentTime - entity->lastFrameTime >= currentAnim->frameDurationMs)
    {
        entity->currentFrameIndex++;
        if (entity->currentFrameIndex >= currentAnim->frameCount)
        {
            if (currentAnim->loop)
            {
                entity->currentFrameIndex = 0; // Boucle
            }
            else
            {
                entity->currentFrameIndex = currentAnim->frameCount - 1; // S'arrête au dernier cadre
                entity->animationPaused = true;                          // Met en pause l'animation à la fin si elle ne boucle pas
            }
        }
        entity->lastFrameTime = currentTime;
    }
}

void Entity_Draw(Entity *entity, SDL_Renderer *renderer)
{
    if (entity->currentAnimationIndex == -1 || !entity->spriteSheet.texture)
    {
        return;
    }

    Animation *currentAnim = &entity->animations[entity->currentAnimationIndex];
    Frame *currentFrame = &currentAnim->frames[entity->currentFrameIndex];

    SDL_Rect srcRect = {currentFrame->x, currentFrame->y, currentFrame->w, currentFrame->h};
    SDL_Rect destRect = {(int)entity->x, (int)entity->y, currentFrame->w, currentFrame->h};

    SDL_RenderCopy(renderer, entity->spriteSheet.texture, &srcRect, &destRect);

    // Pour le débogage: Dessiner la hitbox
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Rouge
    SDL_RenderDrawRect(renderer, &entity->hitbox);
}

void Entity_PauseAnimation(Entity *entity, bool pause)
{
    entity->animationPaused = pause;
}

void Entity_Free(Entity *entity)
{
    if (entity->spriteSheet.texture)
    {
        SDL_DestroyTexture(entity->spriteSheet.texture);
        entity->spriteSheet.texture = NULL;
    }
    if (entity->animations)
    {
        for (int i = 0; i < entity->animationCount; ++i)
        {
            free(entity->animations[i].frames);
            entity->animations[i].frames = NULL;
        }
        free(entity->animations);
        entity->animations = NULL;
    }
    memset(entity, 0, sizeof(Entity)); // Optionnel: effacer la structure après libération
}