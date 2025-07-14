#ifndef ENTITY_H
#define ENTITY_H

#include <SDL.h>
#include <stdbool.h>

// --- Structures pour l'animation ---
typedef struct
{
    int x, y, w, h;
} Frame;

typedef struct
{
    Frame *frames;
    int frameCount;
    int frameDurationMs;  // Durée de chaque frame en millisecondes
    bool loop;            // Indique si l'animation doit boucler
    char name[64];        // Nom de l'animation (ex: "idle_down", "walk_left")
    int spriteSheetIndex; // NOUVEAU: Index de la feuille de sprites à utiliser pour cette animation
} Animation;

typedef struct
{
    SDL_Texture *texture;
    int sheetWidth;
    int sheetHeight;
    int spriteWidth;  // Largeur d'un sprite sur la feuille
    int spriteHeight; // Hauteur d'un sprite sur la feuille
    char name[64];    // Nom de la feuille de sprites (pour référence)
} SpriteSheet;

// --- Structure de base de l'entité ---
typedef struct
{
    float x, y;                  // Position de l'entité dans le monde
    SDL_Rect hitbox;             // Rectangle de collision (dépend de x,y)
    SpriteSheet *spriteSheets;   // NOUVEAU: Tableau de feuilles de sprites de l'entité
    int spriteSheetCount;        // NOUVEAU: Nombre de feuilles de sprites chargées
    Animation *animations;       // Tableau de toutes les animations de l'entité
    int animationCount;          // Nombre total d'animations
    int currentAnimationIndex;   // Index de l'animation en cours
    int currentFrameIndex;       // Index du cadre actuel de l'animation
    Uint32 lastFrameTime;        // Temps du dernier changement de cadre
    bool animationPaused;        // Indique si l'animation est en pause
    bool traversable;            // Indique si l'entité peut être traversée
    int spriteWidth;             // Largeur d'un sprite sur la feuille (peut devenir spécifique à la SpriteSheet active)
    int spriteHeight;            // Hauteur d'un sprite sur la feuille (peut devenir spécifique à la SpriteSheet active)
    Animation *currentAnimation; // Pointeur vers l'animation en cours

} Entity;

// --- Fonctions de gestion de l'entité et des animations ---

bool Entity_Init(Entity *entity, int spriteWidth, int spriteHeight, float x, float y,
                 int hitboxWidth, int hitboxHeight, bool traversable);

bool Entity_AddSpriteSheet(Entity *entity, SDL_Renderer *renderer,
                           const char *spriteSheetPath, const char *name,
                           int spriteWidth, int spriteHeight);

void Entity_AddAnimation(Entity *entity, const char *animationName,
                         const char *spriteSheetName,
                         int startRow, int startCol, int frameCount,
                         int frameDurationMs, bool loop);

void Entity_SetAnimation(Entity *entity, const char *animationName);
void Entity_UpdateAnimation(Entity *entity);
void Entity_Draw(Entity *entity, SDL_Renderer *renderer);
void Entity_PauseAnimation(Entity *entity, bool pause);
void Entity_Free(Entity *entity);

#endif // ENTITY_H