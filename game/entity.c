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

// Fonction d'aide pour trouver l'index d'une feuille de sprites par son nom
static int findSpriteSheetIndex(Entity *entity, const char *spriteSheetName)
{
    for (int i = 0; i < entity->spriteSheetCount; ++i)
    {
        if (strcmp(entity->spriteSheets[i].name, spriteSheetName) == 0)
        {
            return i;
        }
    }
    return -1;
}

// Initialisation de l'entité, sans chargement de spriteSheet.
bool Entity_Init(Entity *entity, int spriteWidth, int spriteHeight, float x, float y, int hitboxWidth, int hitboxHeight, bool traversable)
{
    memset(entity, 0, sizeof(Entity)); // Initialise à zéro

    entity->x = x;
    entity->y = y;
    // La hitbox est initialisée ici mais ses dimensions peuvent être ajustées plus tard
    entity->hitbox.x = (int)(x + spriteWidth / 2 - hitboxWidth / 2);
    entity->hitbox.y = (int)(y + spriteHeight - hitboxHeight);
    entity->hitbox.w = hitboxWidth;
    entity->hitbox.h = hitboxHeight;
    entity->traversable = traversable;

    // Initialisation des listes de spriteSheets et d'animations
    entity->spriteSheets = NULL;
    entity->spriteSheetCount = 0;
    entity->animations = NULL;
    entity->animationCount = 0;

    entity->currentAnimationIndex = -1;
    entity->currentFrameIndex = 0;
    entity->frameTimer = 0.0f;
    entity->animationPaused = false;
    entity->spriteWidth = spriteWidth;
    entity->spriteHeight = spriteHeight;
    entity->currentAnimation = NULL;

    return true;
}

// Fonction pour ajouter une feuille de sprites à l'entité
bool Entity_AddSpriteSheet(Entity *entity, SDL_Renderer *renderer,
                           const char *spriteSheetPath, const char *name,
                           int spriteWidth, int spriteHeight)
{
    if (findSpriteSheetIndex(entity, name) != -1)
    {
        fprintf(stderr, "Une feuille de sprites avec le nom '%s' existe déjà pour cette entité.\n", name);
        return false;
    }

    // Réallouer le tableau de spriteSheets
    entity->spriteSheetCount++;
    entity->spriteSheets = (SpriteSheet *)realloc(entity->spriteSheets, entity->spriteSheetCount * sizeof(SpriteSheet));
    if (!entity->spriteSheets)
    {
        fprintf(stderr, "Erreur d'allocation mémoire pour les feuilles de sprites.\n");
        exit(EXIT_FAILURE);
    }

    SpriteSheet *newSheet = &entity->spriteSheets[entity->spriteSheetCount - 1];
    strncpy(newSheet->name, name, sizeof(newSheet->name) - 1);
    newSheet->name[sizeof(newSheet->name) - 1] = '\0';

    SDL_Surface *surface = IMG_Load(spriteSheetPath);
    if (!surface)
    {
        fprintf(stderr, "Erreur de chargement de la feuille de sprites %s: %s\n", spriteSheetPath, IMG_GetError());
        return false;
    }

    newSheet->texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!newSheet->texture)
    {
        fprintf(stderr, "Erreur de création de la texture pour %s: %s\n", spriteSheetPath, SDL_GetError());
        SDL_FreeSurface(surface);
        return false;
    }
    newSheet->sheetWidth = surface->w;
    newSheet->sheetHeight = surface->h;
    newSheet->spriteWidth = spriteWidth;
    newSheet->spriteHeight = spriteHeight;
    SDL_FreeSurface(surface);

    return true;
}

void Entity_AddAnimation(Entity *entity, const char *animationName,
                         const char *spriteSheetName,
                         int startRow, int startCol, int frameCount,
                         int frameDurationMs, bool loop)
{
    if (findAnimationIndex(entity, animationName) != -1)
    {
        fprintf(stderr, "L'animation '%s' existe déjà pour cette entité.\n", animationName);
        return;
    }

    int spriteSheetIdx = findSpriteSheetIndex(entity, spriteSheetName);
    if (spriteSheetIdx == -1)
    {
        fprintf(stderr, "Erreur: La feuille de sprites '%s' n'existe pas pour cette entité. Impossible d'ajouter l'animation '%s'.\n", spriteSheetName, animationName);
        return;
    }

    // Réallouer le tableau d'animations
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
    newAnim->spriteSheetIndex = spriteSheetIdx; // ENREGISTRE L'INDEX DE LA FEUILLE DE SPRITES

    newAnim->frames = (Frame *)malloc(frameCount * sizeof(Frame));
    if (!newAnim->frames)
    {
        fprintf(stderr, "Erreur d'allocation mémoire pour les cadres de l'animation.\n");
        exit(EXIT_FAILURE);
    }

    SpriteSheet *usedSheet = &entity->spriteSheets[spriteSheetIdx];

    for (int i = 0; i < frameCount; ++i)
    {
        newAnim->frames[i].x = (startCol + i) * usedSheet->spriteWidth;
        newAnim->frames[i].y = startRow * usedSheet->spriteHeight;
        newAnim->frames[i].w = usedSheet->spriteWidth;
        newAnim->frames[i].h = usedSheet->spriteHeight;

        // Assurez-vous que les cadres ne dépassent pas la taille de la feuille de sprites
        if (newAnim->frames[i].x + newAnim->frames[i].w > usedSheet->sheetWidth ||
            newAnim->frames[i].y + newAnim->frames[i].h > usedSheet->sheetHeight)
        {
            fprintf(stderr, "Avertissement: Le cadre %d de l'animation '%s' dépasse la feuille de sprites '%s'.\n", i, animationName, usedSheet->name);
        }
    }

    // Si c'est la première animation ajoutée, la définir comme courante
    if (entity->animationCount == 1)
    {
        entity->currentAnimationIndex = 0;
        entity->currentAnimation = &entity->animations[0];
        entity->currentFrameIndex = 0;
        entity->frameTimer = 0.0f;
        // Mettre à jour les dimensions du sprite de l'entité avec celles de la première animation
        entity->spriteWidth = usedSheet->spriteWidth;
        entity->spriteHeight = usedSheet->spriteHeight;
    }
}

void Entity_SetAnimation(Entity *entity, const char *animationName)
{
    int newIndex = findAnimationIndex(entity, animationName);
    if (newIndex != -1 && newIndex != entity->currentAnimationIndex)
    {
        entity->currentAnimationIndex = newIndex;
        entity->currentAnimation = &entity->animations[newIndex];
        entity->currentFrameIndex = 0; // Réinitialise le cadre au début de la nouvelle animation
        entity->frameTimer = 0.0f;     // Réinitialise le temps du dernier cadre
        entity->animationPaused = false;

        // IMPORTANT: Mettre à jour les dimensions générales du sprite de l'entité
        // pour qu'elles correspondent à la feuille de sprites de l'animation courante
        Animation *currentAnim = &entity->animations[entity->currentAnimationIndex];
        SpriteSheet *usedSheet = &entity->spriteSheets[currentAnim->spriteSheetIndex];
        entity->spriteWidth = usedSheet->spriteWidth;
        entity->spriteHeight = usedSheet->spriteHeight;
    }
    else if (newIndex == -1)
    {
        fprintf(stderr, "Animation '%s' non trouvée pour l'entité.\n", animationName);
    }
}

void Entity_UpdateAnimation(Entity *entity, float deltaTime)
{
    if (entity->animationPaused || entity->currentAnimationIndex == -1)
    {
        return;
    }

    Animation *currentAnim = &entity->animations[entity->currentAnimationIndex];

    // Accumuler le temps écoulé
    entity->frameTimer += deltaTime * 1000.0f; // Convertir deltaTime en ms

    if (entity->frameTimer >= currentAnim->frameDurationMs)
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
        entity->frameTimer = 0.0f; // Réinitialiser le timer
    }
}

void Entity_Draw(Entity *entity, SDL_Renderer *renderer)
{
    if (entity->currentAnimationIndex == -1 || entity->spriteSheetCount == 0)
    {
        return;
    }

    if (!entity->currentAnimation)
        return;
    // S'assurer que l'index de la feuille de sprites est valide
    if (entity->currentAnimation->spriteSheetIndex >= entity->spriteSheetCount || entity->currentAnimation->spriteSheetIndex < 0)
    {
        fprintf(stderr, "Erreur: Index de feuille de sprites invalide pour l'animation '%s'.\n", entity->currentAnimation->name);
        return;
    }

    SpriteSheet *usedSheet = &entity->spriteSheets[entity->currentAnimation->spriteSheetIndex];
    if (!usedSheet->texture) // Vérifier si la texture est chargée
    {
        fprintf(stderr, "Erreur: Texture manquante pour la feuille de sprites '%s' utilisée par l'animation '%s'.\n", usedSheet->name, entity->currentAnimation->name);
        return;
    }

    Frame *currentFrame = &entity->currentAnimation->frames[entity->currentFrameIndex];

    SDL_Rect srcRect = {currentFrame->x, currentFrame->y, currentFrame->w, currentFrame->h};
    SDL_Rect destRect = {(int)entity->x, (int)entity->y, currentFrame->w, currentFrame->h};

    // Rendu de la bonne texture
    SDL_RenderCopy(renderer, usedSheet->texture, &srcRect, &destRect);

    // DrawHitbox(renderer, &entity->hitbox);
}

void DrawHitbox(SDL_Renderer *renderer, SDL_Rect *hitbox)
{
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Rouge
    SDL_RenderDrawRect(renderer, hitbox);
}

void Entity_PauseAnimation(Entity *entity, bool pause)
{
    entity->animationPaused = pause;
}

void Entity_Free(Entity *entity)
{

    if (entity->spriteSheets)
    {
        for (int i = 0; i < entity->spriteSheetCount; ++i)
        {
            if (entity->spriteSheets[i].texture)
            {
                SDL_DestroyTexture(entity->spriteSheets[i].texture);
                entity->spriteSheets[i].texture = NULL;
            }
        }
        free(entity->spriteSheets);
        entity->spriteSheets = NULL;
        entity->spriteSheetCount = 0;
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
        entity->animationCount = 0;
    }
    memset(entity, 0, sizeof(Entity));
}

void Entity_setHitbox(Entity *entity, int x, int y, int w, int h)
{
    entity->hitbox.x = x;
    entity->hitbox.y = y;
    entity->hitbox.w = w;
    entity->hitbox.h = h;
}