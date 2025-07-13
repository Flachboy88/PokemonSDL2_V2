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

    Uint32 lastTime = SDL_GetTicks();

    while (game->running)
    {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (float)(currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        Game_HandleEvent(game, deltaTime);
        Game_UpdateData(game, deltaTime);
        Game_Render(game);
    }

    Game_Free(game);

    return 0;
}
