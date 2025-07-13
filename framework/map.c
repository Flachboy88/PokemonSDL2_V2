#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // Ajout explicite ici aussi, bien que map.h l'inclue

static int Map_LoadTilesets(Map *map, SDL_Renderer *renderer);
static void Map_LoadCollisions(Map *map);
static void Map_RenderTileLayer(Map *map, SDL_Renderer *renderer, tmx_layer *layer);
static tmx_object *tmx_find_object_by_name(tmx_object_group *objgr, const char *name);
static void Map_DefaultSpawn(Map *map);

static int count_objects_in_layer(tmx_layer *layer, const char *layer_name, bool check_name_property)
{
    int count = 0;
    tmx_layer *current = layer;

    while (current)
    {
        if (current->type == L_OBJGR && strcmp(current->name, layer_name) == 0)
        {
            tmx_object *obj = current->content.objgr->head;
            while (obj)
            {
                if (!check_name_property || tmx_get_property(obj->properties, "Name"))
                {
                    count++;
                }
                obj = obj->next;
            }
            break;
        }
        current = current->next;
    }
    return count;
}

void Map_DEBUG(Map *map)
{
    // --- DEBUG: Affichage des informations de collision ---
    printf("\n--- Informations de Collision (DEBUG) ---\n");
    if (map->collision_count == 0)
    {
        printf("Aucune collision trouvée.\n");
    }
    else
    {
        for (int i = 0; i < map->collision_count; i++)
        {
            printf("Collision %d:\n", i);
            printf("  Nom: %s\n", map->collisions[i].name ? map->collisions[i].name : "N/A");
            printf("  Rect: x=%d, y=%d, w=%d, h=%d\n",
                   map->collisions[i].rect.x,
                   map->collisions[i].rect.y,
                   map->collisions[i].rect.w,
                   map->collisions[i].rect.h);
        }
    }

    // --- DEBUG: Affichage des informations des PNJ ---
    printf("\n--- Informations des PNJ (DEBUG) ---\n");
    if (map->pnj_count == 0)
    {
        printf("Aucun PNJ trouvé.\n");
    }
    else
    {
        for (int i = 0; i < map->pnj_count; i++)
        {
            if (map->pnj_list[i])
            {
                printf("PNJ %d:\n", i);
                printf("  Nom: %s\n", map->pnj_list[i]->Name ? map->pnj_list[i]->Name : "N/A");
                printf("  Chemin du sprite: %s\n", map->pnj_list[i]->sprite_path ? map->pnj_list[i]->sprite_path : "N/A");
                printf("  Vitesse: %.2f\n", map->pnj_list[i]->speed);
                printf("  Direction: %d\n", map->pnj_list[i]->direction);
                printf("  Traversable (Hitbox): %s\n", map->pnj_list[i]->is_throughable ? "Oui" : "Non");
            }
        }
    }
    printf("---------------------------------------\n");
}

static void Map_SetDefaultSpawn(Map *map)
{
    tmx_layer *spawn_layer = tmx_find_layer_by_name(map->tmx_map, "PlayerObject");
    if (spawn_layer && spawn_layer->type == L_OBJGR)
    {
        tmx_object *spawn_obj = tmx_find_object_by_name(spawn_layer->content.objgr, "PlayerSpawn");
        if (spawn_obj)
        {
            map->spawn_x = spawn_obj->x;
            map->spawn_y = spawn_obj->y;
        }
    }

    // printf("Spawn position: (%.2f, %.2f)\n", map->spawn_x, map->spawn_y);
}

Map *Map_Load(const char *filename, SDL_Renderer *renderer)
{
    Map *map = malloc(sizeof(Map));
    if (!map)
        return NULL;

    // Initialisation
    memset(map, 0, sizeof(Map));
    map->filename = strdup(filename);

    // Chargement de la map TMX
    map->tmx_map = tmx_load(filename);
    if (!map->tmx_map)
    {
        printf("Erreur lors du chargement de la map: %s (libtmx error: %s)\n", filename, tmx_strerr());
        Map_Free(map);
        return NULL;
    }

    // Chargement des tilesets
    if (!Map_LoadTilesets(map, renderer))
    {
        printf("Erreur lors du chargement des tilesets\n");
        Map_Free(map);
        return NULL;
    }

    // Chargement des collisions
    Map_LoadCollisions(map);

    // Chargement des tiles animées
    Map_LoadAnimatedTiles(map);

    // Chargement des PNJ
    Map_LoadPNJ(map);

    // Charger la position du spawn par défaut
    Map_SetDefaultSpawn(map);

    // printf("Map chargée avec succès: %s\n", filename);
    // Map_DEBUG(map);

    return map;
}

void Map_Free(Map *map)
{
    if (!map)
        return;

    // Libération des textures de tilesets
    for (int i = 0; i < map->tileset_count; i++)
    {
        if (map->tileset_textures[i])
        {
            SDL_DestroyTexture(map->tileset_textures[i]);
        }
    }
    free(map->tileset_textures);

    // Libération des collisions
    for (int i = 0; i < map->collision_count; i++)
    {
        free(map->collisions[i].name);
    }
    free(map->collisions);

    // Libération des tiles animées
    for (int i = 0; i < map->animated_tile_count; i++)
    {
        free(map->animated_tiles[i].frame_ids);
    }
    free(map->animated_tiles);

    // Libération des PNJ
    for (int i = 0; i < map->pnj_count; i++)
    {
        if (map->pnj_list[i])
        {
            free(map->pnj_list[i]->Name);
            free(map->pnj_list[i]->sprite_path);
            free(map->pnj_list[i]);
        }
    }
    free(map->pnj_list);

    // Libération de la map TMX
    if (map->tmx_map)
    {
        tmx_map_free(map->tmx_map);
    }

    free(map->filename);
    free(map);
}

void Map_Update(Map *map, float deltaTime)
{
    if (!map)
        return;

    // Mise à jour des tiles animées
    for (int i = 0; i < map->animated_tile_count; i++)
    {
        AnimatedTile *tile = &map->animated_tiles[i];
        Uint32 current_time = SDL_GetTicks();

        if (current_time - tile->last_update >= tile->frame_duration)
        {
            tile->current_frame = (tile->current_frame + 1) % tile->frame_count;
            tile->last_update = current_time;
        }
    }
}

void Map_RenderLayer(Map *map, SDL_Renderer *renderer, const char *layer_name)
{
    if (!map || !layer_name)
        return;

    tmx_layer *layer = tmx_find_layer_by_name(map->tmx_map, layer_name);
    if (!layer)
    {
        printf("Layer '%s' introuvable\n", layer_name);
        return;
    }

    if (layer->type == L_LAYER)
    {
        Map_RenderTileLayer(map, renderer, layer);
    }
}

void Map_RenderAllLayers(Map *map, SDL_Renderer *renderer)
{
    if (!map)
        return;

    tmx_layer *layer = map->tmx_map->ly_head;
    while (layer)
    {
        if (layer->type == L_LAYER && layer->visible)
        {
            Map_RenderTileLayer(map, renderer, layer);
        }
        layer = layer->next;
    }
}

int Map_CheckCollision(Map *map, SDL_Rect *rect)
{
    if (!map || !rect)
        return 0;

    for (int i = 0; i < map->collision_count; i++)
    {
        if (SDL_HasIntersection(rect, &map->collisions[i].rect))
        {
            return 1;
        }
    }
    return 0;
}

void Map_GetSpawnPosition(Map *map, float *x, float *y)
{
    if (!map || !x || !y)
        return;
    *x = map->spawn_x;
    *y = map->spawn_y;
}

// Fonctions internes

static int Map_LoadTilesets(Map *map, SDL_Renderer *renderer)
{
    tmx_tileset_list *ts_list = map->tmx_map->ts_head;

    // Compter et allouer en une passe
    map->tileset_count = 0;
    tmx_tileset_list *temp = ts_list;
    while (temp)
    {
        map->tileset_count++;
        temp = temp->next;
    }

    map->tileset_textures = calloc(map->tileset_count, sizeof(SDL_Texture *));
    if (!map->tileset_textures)
        return 0;

    // Charger les textures
    char full_path[1024], *map_dir = strdup(map->filename);
    char *last_slash = strrchr(map_dir, '/');
    if (last_slash)
        *last_slash = '\0';
    else
        strcpy(map_dir, ".");

    for (int i = 0; ts_list; ts_list = ts_list->next, i++)
    {
        if (!ts_list->tileset->image)
            continue;

        snprintf(full_path, sizeof(full_path), "%s/%s", map_dir, ts_list->tileset->image->source);
        SDL_Surface *surface = IMG_Load(full_path);
        if (!surface)
            surface = IMG_Load(ts_list->tileset->image->source);

        if (surface)
        {
            map->tileset_textures[i] = SDL_CreateTextureFromSurface(renderer, surface);
            SDL_FreeSurface(surface);
        }
    }

    free(map_dir);
    return 1;
}

static void Map_LoadCollisions(Map *map)
{
    map->collision_count = count_objects_in_layer(map->tmx_map->ly_head, "CollisionObject", false);
    if (map->collision_count == 0)
        return;

    map->collisions = malloc(map->collision_count * sizeof(Collision));
    if (!map->collisions)
        return;

    tmx_layer *layer = map->tmx_map->ly_head;
    while (layer && (layer->type != L_OBJGR || strcmp(layer->name, "CollisionObject") != 0))
    {
        layer = layer->next;
    }

    if (layer)
    {
        tmx_object *obj = layer->content.objgr->head;
        for (int i = 0; obj && i < map->collision_count; obj = obj->next, i++)
        {
            map->collisions[i] = (Collision){
                .rect = {obj->x, obj->y, obj->width, obj->height},
                .name = strdup(obj->name ? obj->name : "")};
        }
    }
}
// Implémentation de Map_LoadAnimatedTiles
void Map_LoadAnimatedTiles(Map *map)
{
    if (!map || !map->tmx_map)
        return;

    // Compter les tiles animées
    map->animated_tile_count = 0;
    tmx_tileset_list *ts_list = map->tmx_map->ts_head;
    while (ts_list)
    {
        for (unsigned int i = 0; i < ts_list->tileset->tilecount; i++)
        {
            if (ts_list->tileset->tiles[i].animation_len > 0)
            {
                map->animated_tile_count++;
            }
        }
        ts_list = ts_list->next;
    }

    if (map->animated_tile_count == 0)
        return;

    map->animated_tiles = calloc(map->animated_tile_count, sizeof(AnimatedTile));
    if (!map->animated_tiles)
        return;

    // Remplir les tiles animées
    int idx = 0;
    ts_list = map->tmx_map->ts_head;
    while (ts_list)
    {
        for (unsigned int i = 0; i < ts_list->tileset->tilecount; i++)
        {
            tmx_tile *tile = &ts_list->tileset->tiles[i];
            if (tile->animation_len > 0)
            {
                AnimatedTile *anim = &map->animated_tiles[idx++];
                anim->tile_id = ts_list->firstgid + tile->id;
                anim->frame_count = tile->animation_len;
                anim->frame_duration = tile->animation ? tile->animation[0].duration : 0;
                anim->last_update = SDL_GetTicks();

                anim->frame_ids = malloc(anim->frame_count * sizeof(int));
                if (anim->frame_ids)
                {
                    for (int j = 0; j < anim->frame_count; j++)
                    {
                        anim->frame_ids[j] = ts_list->firstgid + tile->animation[j].tile_id;
                    }
                }
            }
        }
        ts_list = ts_list->next;
    }
}

// Implémentation de Map_LoadPNJ
void Map_LoadPNJ(Map *map)
{
    if (!map || !map->tmx_map)
        return;

    map->pnj_count = count_objects_in_layer(map->tmx_map->ly_head, "PNJObject", true);
    if (map->pnj_count == 0)
        return;

    map->pnj_list = calloc(map->pnj_count, sizeof(PNJ_init *));
    if (!map->pnj_list)
        return;

    tmx_layer *layer = map->tmx_map->ly_head;
    while (layer && (layer->type != L_OBJGR || strcmp(layer->name, "PNJObject") != 0))
    {
        layer = layer->next;
    }

    if (layer)
    {
        tmx_object *obj = layer->content.objgr->head;
        int idx = 0;
        while (obj && idx < map->pnj_count)
        {
            tmx_property *prop_name = tmx_get_property(obj->properties, "Name");
            if (prop_name && prop_name->type == PT_STRING)
            {
                PNJ_init *pnj = calloc(1, sizeof(PNJ_init));
                if (pnj)
                {
                    pnj->Name = strdup(prop_name->value.string);

                    tmx_property *prop;
                    if ((prop = tmx_get_property(obj->properties, "sprite")) && prop->type == PT_STRING)
                        pnj->sprite_path = strdup(prop->value.string);
                    if ((prop = tmx_get_property(obj->properties, "speed")) && prop->type == PT_FLOAT)
                        pnj->speed = prop->value.decimal;
                    if ((prop = tmx_get_property(obj->properties, "dir")) && prop->type == PT_INT)
                        pnj->direction = prop->value.integer;
                    if ((prop = tmx_get_property(obj->properties, "Hitbox")) && prop->type == PT_BOOL)
                        pnj->is_throughable = prop->value.boolean;

                    map->pnj_list[idx++] = pnj;
                }
            }
            obj = obj->next;
        }
    }
}

// Fonction utilitaire pour trouver un objet par son nom dans un groupe d'objets
static tmx_object *tmx_find_object_by_name(tmx_object_group *objgr, const char *name)
{
    if (!objgr || !name)
        return NULL;

    tmx_object *obj = objgr->head;
    while (obj)
    {
        if (obj->name && strcmp(obj->name, name) == 0)
        {
            return obj;
        }
        obj = obj->next;
    }
    return NULL;
}

static void Map_RenderTileLayer(Map *map, SDL_Renderer *renderer, tmx_layer *layer)
{
    unsigned int gid;
    tmx_tileset_list *ts_list_iter; // Utiliser tmx_tileset_list pour itérer et accéder firstgid
    tmx_tileset *current_tileset = NULL;
    SDL_Texture *texture_to_render = NULL;
    SDL_Rect src_rect;
    SDL_Rect dst_rect;

    long tile_x, tile_y; // Itérateurs de boucle pour les tuiles

    for (tile_y = 0; tile_y < map->tmx_map->height; tile_y++)
    {
        for (tile_x = 0; tile_x < map->tmx_map->width; tile_x++)
        {
            gid = layer->content.gids[(tile_y * map->tmx_map->width) + tile_x];

            // Si gid est 0, cela signifie pas de tuile (vide)
            if (gid == 0)
                continue;

            // Supprimer les drapeaux de retournement pour obtenir l'ID de tuile original
            unsigned int original_gid = gid & TMX_FLIP_BITS_REMOVAL;

            // Vérifier si cette tuile est animée
            int current_frame_gid = original_gid; // Par défaut, utiliser l'ID original
            for (int i = 0; i < map->animated_tile_count; i++)
            {
                if (map->animated_tiles[i].tile_id == original_gid)
                {
                    current_frame_gid = map->animated_tiles[i].frame_ids[map->animated_tiles[i].current_frame];
                    break;
                }
            }

            // Trouver le tileset et la texture pour le GID de l'image actuelle
            current_tileset = NULL;
            texture_to_render = NULL;
            ts_list_iter = map->tmx_map->ts_head;
            int tileset_idx = 0;
            while (ts_list_iter)
            {
                if (current_frame_gid >= ts_list_iter->firstgid &&
                    current_frame_gid < (ts_list_iter->firstgid + ts_list_iter->tileset->tilecount))
                {
                    current_tileset = ts_list_iter->tileset;
                    texture_to_render = map->tileset_textures[tileset_idx];
                    break;
                }
                ts_list_iter = ts_list_iter->next;
                tileset_idx++;
            }

            if (!current_tileset || !texture_to_render)
            {
                // Si le tileset ou la texture n'est pas trouvée pour ce GID, ignorer le rendu.
                continue;
            }

            // Calculer la position dans le tileset (rectangle source)
            int local_id = current_frame_gid - ts_list_iter->firstgid; // Utiliser firstgid du tmx_tileset_list
            int tiles_per_row = current_tileset->image->width / current_tileset->tile_width;
            src_rect.x = (local_id % tiles_per_row) * current_tileset->tile_width;
            src_rect.y = (local_id / tiles_per_row) * current_tileset->tile_height;
            src_rect.w = current_tileset->tile_width;
            src_rect.h = current_tileset->tile_height;

            // Calculer la position sur l'écran (rectangle de destination)
            dst_rect.x = tile_x * map->tmx_map->tile_width;
            dst_rect.y = tile_y * map->tmx_map->tile_height;
            dst_rect.w = map->tmx_map->tile_width;
            dst_rect.h = map->tmx_map->tile_height;

            // Gérer le retournement des tuiles (libtmx fournit des drapeaux dans gid)
            SDL_RendererFlip flip = SDL_FLIP_NONE;
            if (gid & TMX_FLIPPED_HORIZONTALLY)
                flip |= SDL_FLIP_HORIZONTAL;
            if (gid & TMX_FLIPPED_VERTICALLY)
                flip |= SDL_FLIP_VERTICAL;

            // Rendre la tuile
            SDL_RenderCopyEx(renderer, texture_to_render, &src_rect, &dst_rect, 0, NULL, flip);
        }
    }
}