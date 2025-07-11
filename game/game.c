#include "game.h"

static Map *Game_LoadAndInitMap(const char *name, SDL_Renderer *renderer);
static bool Game_HandleInputEvents(Game *game, SDL_Event *event);
static void Game_UpdateData(Game *game, float deltaTime);
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
}

bool Game_InitPlayer(Game *game)
{
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

    game->lastTime = SDL_GetTicks();

    return game;
}

void Game_Free(Game *game)
{

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
    // free maps

    IMG_Quit();
    SDL_Quit();
    free(game);
}

static void Game_UpdateData(Game *game, float deltaTime)
{
    // switch (game->state)
}

static void Game_UpdateGraphics(Game *game, Uint32 currentTime)
{
    SDL_SetRenderDrawColor(game->renderer, 30, 30, 30, 255);
    SDL_RenderClear(game->renderer);

    // affihcer map et joueur

    SDL_RenderPresent(game->renderer);
}

void Game_HandleEvent(Game *game)
{
}

void Game_Update(Game *game)
{
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - game->lastTime) / 1000.0f;
    game->lastTime = currentTime;

    Game_UpdateData(game, deltaTime);
}

void Game_Render(Game *game)
{
}

void Game_Run(Game *game)
{
    while (game->running)
    {
        Game_HandleEvent(game);
        Game_Update(game);
        Game_Render(game);
    }
}