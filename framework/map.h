#ifndef MAP_H
#define MAP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include "tmx.h"

typedef struct
{
    SDL_Rect rect;
    char *name;
} Collision;

typedef struct
{
    int tile_id;
    int current_frame;
    int frame_count;
    Uint32 last_update;
    int frame_duration;
    int *frame_ids;
} AnimatedTile;

typedef struct
{
    char *Name;
    char *sprite_path;
    float speed;
    int direction;
    bool is_throughable;
} PNJ_init;

typedef struct
{
    tmx_map *tmx_map;
    SDL_Texture **tileset_textures;
    int tileset_count;

    Collision *collisions;
    int collision_count;

    AnimatedTile *animated_tiles;
    int animated_tile_count;

    PNJ_init **pnj_list;
    int pnj_count;

    float spawn_x, spawn_y;
    char *filename;
} Map;

// Fonctions principales
Map *Map_Load(const char *filename, SDL_Renderer *renderer);
void Map_Free(Map *map);
void Map_Update(Map *map, float deltaTime);
void Map_RenderLayer(Map *map, SDL_Renderer *renderer, const char *layer_name);
void Map_RenderAllLayers(Map *map, SDL_Renderer *renderer);
static void Map_LoadPNJ(Map *map);

// Fonctions utilitaires
int Map_CheckCollision(Map *map, SDL_Rect *rect);
void Map_GetSpawnPosition(Map *map, float *x, float *y);

// Fonctions internes
static int Map_LoadTilesets(Map *map, SDL_Renderer *renderer);
static void Map_LoadCollisions(Map *map);
static void Map_LoadAnimatedTiles(Map *map);
static void Map_RenderTileLayer(Map *map, SDL_Renderer *renderer, tmx_layer *layer);
static void Map_DEBUG(Map *map);
static void Map_SetDefaultSpawn(Map *map);

#endif // MAP_H