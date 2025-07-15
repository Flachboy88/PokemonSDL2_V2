#include "game.h"
#include <unistd.h>
#include <limits.h>

static Map *Game_LoadAndInitMap(const char *name, SDL_Renderer *renderer);
static bool Game_HandleInputEvents(Game *game, SDL_Event *event);
void Game_UpdateData(Game *game, float deltaTime);
static void Game_UpdateGraphics(Game *game);

bool Game_InitSDL(Game *game, const char *title, int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        fprintf(stderr, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_Quit();
        return false;
    }

    game->window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (!game->window)
    {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!game->renderer)
    {
        fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(game->window);
        IMG_Quit();
        SDL_Quit();
        return false;
    }

    game->window_width = width;
    game->window_height = height;

    return true;
}

bool Game_InitMap(Game *game, const char *map_name)
{
    char map_path[256];
    snprintf(map_path, sizeof(map_path), "resources/maps/%s.tmx", map_name);

    game->current_map = Map_Load(map_path, game->renderer);
    if (!game->current_map)
    {
        fprintf(stderr, "Failed to load map: %s\n", map_path);
        return false;
    }

    return true;
}

bool Game_InitPlayer(Game *game)
{

    game->player = (Player *)malloc(sizeof(Player));
    if (!game->player)
    {
        fprintf(stderr, "Failed to allocate memory for player!\n");
        return false;
    }

    if (!Player_Init(game->player, game->renderer, "resources/sprites/player.png", 25, 32, 100.0f, 100.0f, PLAYER_HITBOX_WIDTH, PLAYER_HITBOX_HEIGHT))
    {
        fprintf(stderr, "Failed to initialize player!\n");
        free(game->player);
        game->player = NULL;
        return false;
    }
    return true;
}

bool testAjoutSpriteSheet(Game *game, SDL_Renderer *renderer)
{

    // exemple ajout sprite sheet
    if (!Entity_AddSpriteSheet(&game->player->baseEntity, game->renderer,
                               "resources/sprites/pnj.png", "player_combat", 25, 32))
    {
        fprintf(stderr, "Failed to load combat spritesheet!\n");
        Player_Free(game->player);
        free(game->player);
        game->player = NULL;
        return false;
    }

    Entity_AddAnimation(&game->player->baseEntity, "test", "player_combat", 1, 0, 4, 200, true);
}

Game *Game_Create(const char *title, int width, int height)
{

    Game *game = (Game *)malloc(sizeof(Game));
    if (!game)
    {
        fprintf(stderr, "Failed to allocate Game structure\n");
        return NULL;
    }

    memset(game, 0, sizeof(Game));

    game->running = true;
    game->state = MODE_WORLD;

    if (!Game_InitSDL(game, title, width, height))
    {
        Game_Free(game);
        return NULL;
    }

    if (!Game_InitMap(game, "map3"))
    {
        Game_Free(game);
        return NULL;
    }

    if (!Game_InitPlayer(game))
    {
        Game_Free(game);
        return NULL;
    }

    return game;
}

void Game_Free(Game *game)
{
    if (!game)
        return;

    Map_Free(game->current_map);
    printf("Map freed\n");

    if (game->renderer)
    {
        SDL_DestroyRenderer(game->renderer);
        game->renderer = NULL;
        printf("Renderer freed\n");
    }
    if (game->window)
    {
        SDL_DestroyWindow(game->window);
        game->window = NULL;
        printf("Window freed\n");
    }

    if (game->player)
    {
        Player_Free(game->player);
        free(game->player);
        game->player = NULL;
        printf("Player freed\n");
    }

    IMG_Quit();
    SDL_Quit();
    free(game);
}

void Game_UpdateData(Game *game, float deltaTime)
{
    switch (game->state)
    {
    case MODE_WORLD:
        Map_Update(game->current_map, deltaTime);
        Game_UpdatePlayerMovement(game, deltaTime);
        Player_Update(game->player, deltaTime);
        break;
    default:
        break;
    }
}
static void Game_UpdateGraphics(Game *game)
{
    SDL_SetRenderDrawColor(game->renderer, 30, 30, 30, 255);
    SDL_RenderClear(game->renderer);

    switch (game->state)
    {
    case MODE_WORLD:
        // Afficher la map
        if (game->current_map)
        {
            Map_RenderLayer(game->current_map, game->renderer, "BackgroundCalque");
            Map_RenderLayer(game->current_map, game->renderer, "PremierPlanCalque");
            Player_Draw(game->player, game->renderer);
            Map_RenderNPC(game->current_map, game->renderer);
            Map_RenderLayer(game->current_map, game->renderer, "SecondPlanCalque");
        }

        break;

    default:
        break;
    }

    SDL_RenderPresent(game->renderer);
}

void Game_HandleEvent(Game *game, float deltaTime)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            game->running = false;
        }
    }
    Game_HandleGameStateEvent(game, deltaTime);
}

void Game_Render(Game *game)
{
    Game_UpdateGraphics(game);
}

void Game_HandleGameStateEvent(Game *game, float deltaTime)
{
    switch (game->state)
    {
    case MODE_WORLD:

        HandlePlayerInput(game);

        break;
    default:
        break;
    }
}

static bool Game_CanPlayerMoveTo(Game *game, float newX, float newY)
{
    // Créer une hitbox temporaire
    SDL_Rect tempHitbox = {
        (int)(newX + game->player->baseEntity.spriteWidth / 2 - PLAYER_HITBOX_WIDTH / 2),
        (int)(newY + game->player->baseEntity.spriteHeight - PLAYER_HITBOX_HEIGHT),
        PLAYER_HITBOX_WIDTH,
        PLAYER_HITBOX_HEIGHT};

    // Vérifier les collisions avec la map
    if (Map_CheckCollision(game->current_map, &tempHitbox))
    {
        return false;
    }

    for (int i = 0; i < game->current_map->npc_count; i++)
    {
        NPC *npc = game->current_map->npc[i];
        // printf("NPC hitbox traversable : %s\n", npc->baseEntity.traversable ? "oui" : "non");
        if (!npc->baseEntity.traversable)
        {
            SDL_Rect npcHitbox = npc->baseEntity.hitbox;
            if (SDL_HasIntersection(&tempHitbox, &npcHitbox))
            {
                return false;
            }
        }
    }

    return true;
}

static void Game_UpdatePlayerMovement(Game *game, float deltaTime)
{
    Player *player = game->player;

    // Traitement des inputs de mouvement
    if (player->targetDirection != DIRECTION_NONE)
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

        // Vérifier les collisions et se coller si nécessaire
        if (Game_CanPlayerMoveTo(game, newX, newY))
        {
            player->baseEntity.x = newX;
            player->baseEntity.y = newY;
            player->state = PLAYER_STATE_MOVING;
        }
        else
        {
            // Se coller à la collision
            float stepX = (player->targetDirection == DIRECTION_LEFT) ? -1.0f : (player->targetDirection == DIRECTION_RIGHT) ? 1.0f
                                                                                                                             : 0.0f;
            float stepY = (player->targetDirection == DIRECTION_UP) ? -1.0f : (player->targetDirection == DIRECTION_DOWN) ? 1.0f
                                                                                                                          : 0.0f;

            // Avancer pixel par pixel jusqu'à collision
            float testX = player->baseEntity.x;
            float testY = player->baseEntity.y;

            while (Game_CanPlayerMoveTo(game, testX + stepX, testY + stepY))
            {
                testX += stepX;
                testY += stepY;
            }

            player->baseEntity.x = testX;
            player->baseEntity.y = testY;
            player->state = PLAYER_STATE_IDLE;
            Player_UpdateAnimation(player);
        }
    }
    else if (player->hasTarget)
    {
        // Alignement sur la grille
        int n = 16;
        float alignedX = player->baseEntity.x;
        float alignedY = player->baseEntity.y;

        // Calculer la prochaine position alignée selon la direction
        switch (player->currentDirection)
        {
        case DIRECTION_LEFT:
            alignedX = floor(player->baseEntity.x / n) * n;
            break;
        case DIRECTION_RIGHT:
            alignedX = ceil(player->baseEntity.x / n) * n;
            break;
        case DIRECTION_UP:
            alignedY = floor(player->baseEntity.y / n) * n;
            break;
        case DIRECTION_DOWN:
            alignedY = ceil(player->baseEntity.y / n) * n;
            break;
        default:
            break;
        }

        // Mouvement vers l'alignement
        float dx = alignedX - player->baseEntity.x;
        float dy = alignedY - player->baseEntity.y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance > 1.0f)
        {
            float moveDistance = player->speed * deltaTime;
            if (moveDistance > distance)
                moveDistance = distance;

            float newX = player->baseEntity.x + (dx / distance) * moveDistance;
            float newY = player->baseEntity.y + (dy / distance) * moveDistance;

            if (Game_CanPlayerMoveTo(game, newX, newY))
            {
                player->baseEntity.x = newX;
                player->baseEntity.y = newY;
            }
        }
        else
        {
            // Alignement terminé
            player->baseEntity.x = alignedX;
            player->baseEntity.y = alignedY;
            player->hasTarget = false;
            player->state = PLAYER_STATE_IDLE;
            Player_UpdateAnimation(player);
        }
    }
    else
    {
        player->state = PLAYER_STATE_IDLE;
        Player_UpdateAnimation(player);
    }
}

void HandlePlayerInput(Game *game)
{
    Player_HandleInput(game->player);
}