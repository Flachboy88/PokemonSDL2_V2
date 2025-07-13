#include "game.h"
#include <unistd.h>
#include <limits.h>

static Map *Game_LoadAndInitMap(const char *name, SDL_Renderer *renderer);
static bool Game_HandleInputEvents(Game *game, SDL_Event *event);
void Game_UpdateData(Game *game, float deltaTime);
static void Game_UpdateGraphics(Game *game, Uint32 currentTime);

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

    game->lastTime = SDL_GetTicks();

    return game;
}

void Game_Free(Game *game)
{
    if (!game)
        return;

    Map_Free(game->current_map);

    if (game->renderer)
    {
        SDL_DestroyRenderer(game->renderer);
        game->renderer = NULL;
    }
    if (game->window)
    {
        SDL_DestroyWindow(game->window);
        game->window = NULL;
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
        break;
    default:
        break;
    }
}

static void Game_UpdateGraphics(Game *game, Uint32 currentTime)
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
            // player
            Map_RenderLayer(game->current_map, game->renderer, "SecondPlanCalque");
        }

        break;

    default:
        break;
    }

    SDL_RenderPresent(game->renderer);
}

void Game_HandleEvent(Game *game)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            game->running = false;
        }
    }
    Game_HandleGameStateEvent(game);
}

void Game_Render(Game *game)
{
    Game_UpdateGraphics(game, SDL_GetTicks());
}

void Game_HandleGameStateEvent(Game *game)
{
    switch (game->state)
    {
    case MODE_WORLD:
        // input joueur, collisions etc
        break;
    default:
        break;
    }
}
