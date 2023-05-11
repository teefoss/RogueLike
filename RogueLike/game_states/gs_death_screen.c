//
//  death_screen.c
//  RogueLike
//
//  Created by Thomas Foster on 5/10/23.
//

#include "game.h"
#include "game_state.h"
#include "video.h"
#include <SDL.h>

bool DeathScreen_ProcessEvent(Game * game, const SDL_Event * event)
{
    switch ( event->type ) {
        case SDL_KEYDOWN:
            if ( event->key.keysym.sym == SDLK_RETURN ) {
                ChangeStateAndFadeIn(game, &gs_title_screen, 0.5f);
                return true;
            }
            break;
        default:
            break;
    }

    return false;
}


void DeathScreen_Render(const Game * game)
{
    V_SetRGBA(0, 0, 0, 0);
    int width = V_PrintString(0, 0, game->kill_message);

    V_SetColor(palette[GOLINE_RED]);
    int x = (GAME_WIDTH - width) / 2;
    int y = (GAME_HEIGHT - V_CharHeight()) / 2;
    V_PrintString(x, y, game->kill_message);
}


const GameState gs_death_screen = {
    .process_event      = DeathScreen_ProcessEvent,
    .render             = DeathScreen_Render,
};
