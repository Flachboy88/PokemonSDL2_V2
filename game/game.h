#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <SDL2/SDL_image.h>

#include "../framework/map.h"

typedef enum
{
    MODE_WORLD,
    MODE_MENU,
    MODE_COMBAT,
    MODE_CINEMATIC

} GameState;

typedef struct Game
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int window_width;
    int window_height;

    GameState state;

    Map *current_map;
    // Player *player;
    Uint32 lastTime;

    bool running;
} Game;

Game *Game_Create(const char *title, int width, int height);
void Game_Free(Game *game);
void Game_HandleEvent(Game *game);
void Game_HandleGameStateEvent(Game *game);
void Game_UpdateData(Game *game, float deltaTime);
void Game_Render(Game *game);

// Fonctions d'initialisation internes
bool Game_InitSDL(Game *game, const char *title, int width, int height);
bool Game_InitMap(Game *game, const char *map_name);
bool Game_InitPlayer(Game *game);

#endif // GAME_H