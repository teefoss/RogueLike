//
//  intermission.c
//  RogueLike
//
//  Created by Thomas Foster on 5/10/23.
//

#include "game.h"
#include "game_state.h"
#include "video.h"
#include <SDL_events.h>

bool Intermission_ProcessEvent(Game * game, const SDL_Event * event)
{
    switch ( event->type ) {
        case SDL_KEYDOWN:
            // load next level
            // change state
            return true;
        default:
            break;
    }
    return false;
}


void Intermission_Render(const Game * game)
{
    const char * level_string = "Level %d";

    V_SetRGBA(0, 0, 0, 0);
    int width = V_PrintString(0, 0, level_string, game->level);

    V_SetRGB(255, 255, 255);
    int x = (GAME_WIDTH - width) / 2;
    int y = (GAME_HEIGHT - V_CharHeight()) / 2;
    V_PrintString(x, y, level_string, game->level);
}


void Intermission_OnEnter(Game * game)
{
    StartFadeIn(&game->fade_state, 2.0f);
}


void Intermission_OnExit(Game * game)
{
    LoadLevel(game, game->level, true);
    game->player_info.turns = INITIAL_TURNS;
    PlayerCastSight(&game->world, &game->render_info);

    StartFadeIn(&game->fade_state, 1.0f);
}


const GameState gs_intermission = {
    .process_event      = NULL,
    .update             = NULL,
    .render             = Intermission_Render,
    .on_enter           = Intermission_OnEnter,
    .on_exit            = Intermission_OnExit,
    .finite_duration    = true,
    .duration_ticks     = MS2TICKS(3000, FPS),
    .next_state         = &gs_level_idle,
};
