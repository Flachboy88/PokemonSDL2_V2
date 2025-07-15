#include "player.h"
#include <stdio.h>

const int LARGEUR_HITBOX = PLAYER_HITBOX_WIDTH;
const int HAUTEUR_HITBOX = PLAYER_HITBOX_HEIGHT;

static void Player_StartMovement(Player *player, Direction direction, float targetX, float targetY);
static void Player_FinishMovement(Player *player, float deltaTime);
static float Player_AlignToGrid(float position, int gridSize);
void Player_UpdateAnimation(Player *player);

bool Player_Init(Player *player, SDL_Renderer *renderer, const char *initialSpriteSheetPath,
                 int spriteWidth, int spriteHeight, float x, float y,
                 int hitboxWidth, int hitboxHeight)
{
    if (!Entity_Init(&player->baseEntity, spriteWidth, spriteHeight, x, y, hitboxWidth, hitboxHeight, true))
    {
        return false;
    }

    player->state = PLAYER_STATE_IDLE;
    player->currentDirection = DIRECTION_DOWN;
    player->targetDirection = DIRECTION_NONE;
    player->lastDirection = DIRECTION_DOWN;
    player->targetX = x;
    player->targetY = y;
    player->hasTarget = false;
    player->wasMovingLastFrame = false;
    player->currentMovementMode = MOVEMENT_WALK;
    player->walkSpeed = PLAYER_SPEED;
    player->runSpeed = PLAYER_SPEED * 1.5f;
    player->bikeSpeed = PLAYER_SPEED * 2.0f;
    player->speed = player->walkSpeed;

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

    Entity_AddSpriteSheet(&player->baseEntity, renderer, "resources/sprites/player_bike.png", "BIKE", spriteWidth, spriteHeight);
    Entity_AddAnimation(&player->baseEntity, "bike_idle_down", "BIKE", 0, 0, 1, 100, false);
    Entity_AddAnimation(&player->baseEntity, "bike_idle_left", "BIKE", 1, 0, 1, 100, false);
    Entity_AddAnimation(&player->baseEntity, "bike_idle_right", "BIKE", 2, 0, 1, 100, false);
    Entity_AddAnimation(&player->baseEntity, "bike_idle_top", "BIKE", 3, 0, 1, 100, false);

    Entity_AddAnimation(&player->baseEntity, "bike_down", "BIKE", 0, 0, 4, 100, true);
    Entity_AddAnimation(&player->baseEntity, "bike_left", "BIKE", 1, 0, 4, 100, true);
    Entity_AddAnimation(&player->baseEntity, "bike_right", "BIKE", 2, 0, 4, 100, true);
    Entity_AddAnimation(&player->baseEntity, "bike_top", "BIKE", 3, 0, 4, 100, true);

    Entity_SetAnimation(&player->baseEntity, "idle_down");
    player->baseEntity.currentAnimation = &player->baseEntity.animations[0];

    return true;
}
void Player_Update(Player *player, float deltaTime)
{
    // Mise à jour de l'animation
    Entity_UpdateAnimation(&player->baseEntity, deltaTime);

    // Recalculer la hitbox
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

void Player_HandleInput(Player *player)
{
    Direction inputDir = Player_GetInputDirection();
    bool hasMovementInput = (inputDir != DIRECTION_NONE);

    const Uint8 *state = SDL_GetKeyboardState(NULL);
    static bool spaceWasPressed = false;
    bool spaceIsPressed = state[SDL_SCANCODE_SPACE];

    if (spaceIsPressed && !spaceWasPressed)
    {
        if (player->currentMovementMode == MOVEMENT_WALK)
        {
            Player_SetMovementMode(player, MOVEMENT_BIKE);
        }
        else if (player->currentMovementMode == MOVEMENT_BIKE)
        {
            Player_SetMovementMode(player, MOVEMENT_WALK);
        }
    }
    if (hasMovementInput)
    {
        player->targetDirection = inputDir;
        player->hasTarget = false;
        player->currentDirection = inputDir;
        Player_UpdateAnimation(player);
    }
    else if (player->wasMovingLastFrame)
    {
        // Demander alignement sur la grille
        player->hasTarget = true;
        player->targetDirection = DIRECTION_NONE;
    }
    else
    {
        player->targetDirection = DIRECTION_NONE;
    }

    player->wasMovingLastFrame = hasMovementInput;
}

Direction Player_GetInputDirection(void)
{
    const Uint8 *state = SDL_GetKeyboardState(NULL);

    if (state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_W])
    {
        return DIRECTION_UP;
    }
    if (state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_S])
    {
        return DIRECTION_DOWN;
    }
    if (state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_A])
    {
        return DIRECTION_LEFT;
    }
    if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_D])
    {
        return DIRECTION_RIGHT;
    }

    return DIRECTION_NONE;
}

bool Player_TryMove(Player *player, Direction direction, float newX, float newY)
{
    if (player->state != PLAYER_STATE_IDLE)
    {
        return false;
    }

    Player_StartMovement(player, direction, newX, newY);
    return true;
}

static void Player_StartMovement(Player *player, Direction direction, float targetX, float targetY)
{
    player->state = PLAYER_STATE_MOVING;
    player->currentDirection = direction;
    player->targetX = targetX;
    player->targetY = targetY;
    Player_UpdateAnimation(player);
}

static void Player_UpdateMovement(Player *player, float deltaTime)
{
    if (player->hasTarget)
    {
        // Mouvement vers la target avec racine carrée
        float dx = player->targetX - player->baseEntity.x;
        float dy = player->targetY - player->baseEntity.y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance > 1.0f)
        {
            float moveDistance = player->speed * deltaTime;
            if (moveDistance > distance)
                moveDistance = distance;

            float newX = player->baseEntity.x + (dx / distance) * moveDistance;
            float newY = player->baseEntity.y + (dy / distance) * moveDistance;

            player->baseEntity.x = newX;
            player->baseEntity.y = newY;
        }
        else
        {
            // Arrivé à la target
            player->baseEntity.x = player->targetX;
            player->baseEntity.y = player->targetY;
            player->hasTarget = false;
            player->state = PLAYER_STATE_IDLE;
            Player_UpdateAnimation(player);
        }
    }
    else if (player->targetDirection != DIRECTION_NONE)
    {
        // Mouvement libre continu
        float moveDistance = player->speed * deltaTime;
        float newX = player->baseEntity.x;
        float newY = player->baseEntity.y;

        switch (player->targetDirection)
        {
        case DIRECTION_UP:
            newY -= moveDistance;
            break;
        case DIRECTION_DOWN:
            newY += moveDistance;
            break;
        case DIRECTION_LEFT:
            newX -= moveDistance;
            break;
        case DIRECTION_RIGHT:
            newX += moveDistance;
            break;
        default:
            break;
        }

        player->baseEntity.x = newX;
        player->baseEntity.y = newY;
    }
}

static void Player_FinishMovement(Player *player, float deltaTime)
{
    float moveDistance = player->speed * deltaTime;

    // Seulement aligner dans la direction du mouvement, pas perpendiculaire
    float alignedX = player->baseEntity.x;
    float alignedY = player->baseEntity.y;

    // Aligner dans la direction du mouvement
    if (player->currentDirection == DIRECTION_UP || player->currentDirection == DIRECTION_DOWN)
    {
        // Mouvement vertical, aligner Y
        alignedY = Player_AlignToGrid(player->baseEntity.y, 16);
    }
    else if (player->currentDirection == DIRECTION_LEFT || player->currentDirection == DIRECTION_RIGHT)
    {
        // Mouvement horizontal, aligner X
        alignedX = Player_AlignToGrid(player->baseEntity.x, 16);
    }

    float dx = alignedX - player->baseEntity.x;
    float dy = alignedY - player->baseEntity.y;

    if (fabs(dx) <= moveDistance && fabs(dy) <= moveDistance)
    {
        // Alignement terminé
        player->baseEntity.x = alignedX;
        player->baseEntity.y = alignedY;
        player->state = PLAYER_STATE_IDLE;
        player->lastDirection = player->currentDirection;
        player->currentDirection = DIRECTION_NONE;
        Player_UpdateAnimation(player);
    }
    else
    {
        // Continuer l'alignement
        if (dx != 0)
        {
            player->baseEntity.x += (dx > 0) ? moveDistance : -moveDistance;
        }
        if (dy != 0)
        {
            player->baseEntity.y += (dy > 0) ? moveDistance : -moveDistance;
        }
    }
}

static float Player_AlignToGrid(float position, int gridSize)
{
    return roundf(position / gridSize) * gridSize;
}

void Player_UpdateAnimation(Player *player)
{
    // Si on n'a pas de direction courante, garder la dernière direction connue
    Direction directionToUse = player->currentDirection;
    if (directionToUse == DIRECTION_NONE)
    {
        if (player->baseEntity.currentAnimation)
        {
            if (strstr(player->baseEntity.currentAnimation->name, "top"))
                directionToUse = DIRECTION_UP;
            else if (strstr(player->baseEntity.currentAnimation->name, "left"))
                directionToUse = DIRECTION_LEFT;
            else if (strstr(player->baseEntity.currentAnimation->name, "right"))
                directionToUse = DIRECTION_RIGHT;
            else
                directionToUse = DIRECTION_DOWN;
        }
        else
        {
            directionToUse = DIRECTION_DOWN;
        }
    }

    const char *modePrefix;
    switch (player->currentMovementMode)
    {
    case MOVEMENT_RUN:
        modePrefix = "run";
        break;
    case MOVEMENT_BIKE:
        modePrefix = "bike";
        break;
    default:
        modePrefix = "walk";
        break;
    }

    char animationName[64];
    const char *directionSuffix;

    switch (directionToUse)
    {
    case DIRECTION_UP:
        directionSuffix = "top";
        break;
    case DIRECTION_DOWN:
        directionSuffix = "down";
        break;
    case DIRECTION_LEFT:
        directionSuffix = "left";
        break;
    case DIRECTION_RIGHT:
        directionSuffix = "right";
        break;
    default:
        directionSuffix = "down";
        break;
    }

    if (player->state == PLAYER_STATE_MOVING)
    {
        snprintf(animationName, sizeof(animationName), "%s_%s", modePrefix, directionSuffix);
    }
    else
    {
        if (player->currentMovementMode == MOVEMENT_BIKE)
        {
            snprintf(animationName, sizeof(animationName), "bike_idle_%s", directionSuffix);
        }
        else if (player->currentMovementMode == MOVEMENT_RUN)
        {
            snprintf(animationName, sizeof(animationName), "run_idle_%s", directionSuffix);
        }
        else
        {
            snprintf(animationName, sizeof(animationName), "idle_%s", directionSuffix);
        }
    }

    Entity_SetAnimation(&player->baseEntity, animationName);
}

void Player_SetMovementMode(Player *player, MovementMode mode)
{
    player->currentMovementMode = mode;

    switch (mode)
    {
    case MOVEMENT_WALK:
        player->speed = player->walkSpeed;
        break;
    case MOVEMENT_RUN:
        player->speed = player->runSpeed;
        break;
    case MOVEMENT_BIKE:
        player->speed = player->bikeSpeed;
        break;
    }

    Player_UpdateAnimation(player);
}
