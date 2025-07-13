#include "game/game.h"
#include "game/player.h"
#include "game/npc.h"
#include "game/constante.h"

int main(int argc, char *argv[])
{

    Game *game = Game_Create(SCREEN_TITLE, SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!game)
    {
        fprintf(stderr, "Failed to create game instance. Exiting.\n");
        return 1;
    }

    while (game->running)
    {
        Game_HandleEvent(game);
        Game_UpdateData(game, game->lastTime);

        SDL_SetRenderDrawColor(game->renderer, 30, 30, 30, 255);
        SDL_RenderClear(game->renderer);

        Game_Render(game);
    }

    Game_Free(game);

    return 0;
}
