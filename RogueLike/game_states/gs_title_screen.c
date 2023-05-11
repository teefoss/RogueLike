//
//  title_screen.c
//  RogueLike
//
//  Created by Thomas Foster on 5/10/23.
//

#include "game.h"
#include "game_state.h"
#include "menu.h"

void TitleScreen_Render(const Game * game)
{
    RenderWorld(&game->world, &game->render_info, game->ticks);
}


void TitleScreen_OnEnter(Game * game)
{
    StartFadeIn(&game->fade_state, 1.0f);
    menu_state = MENU_MAIN;
    PushState(game, &gs_menu);
}


const GameState gs_title_screen = {
    .render = TitleScreen_Render,
    .on_enter = TitleScreen_OnEnter,
};
