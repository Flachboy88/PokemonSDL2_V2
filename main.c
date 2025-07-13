#include "game/game.h"

int main(int argc, char *argv[])
{

    Game *game = Game_Create("PokemonSDL2", 500, 500);
    if (!game)
    {
        fprintf(stderr, "Failed to create game instance. Exiting.\n");
        return 1;
    }

    while (game->running)
    {
        Game_HandleEvent(game);
        Game_UpdateData(game, game->lastTime);
        Game_Render(game);
    }

    Game_Free(game);

    return 0;
}
