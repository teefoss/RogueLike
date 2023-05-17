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
    // Generate a forest as the title screen background.
    // u32 seed = (u32)time(NULL);
    u32 seed = 0;
    printf("title screen seed: %d\n", seed);
    GenerateWorld(game, AREA_FOREST, seed, game->forest_size, game->forest_size);

    // TODO: check if this is still needed.
    for ( int i = 0; i < game->world.map.width * game->world.map.height; i++ ) {
        Tile * tile = &game->world.map.tiles[i];
        tile->light = game->world.info->revealed_light;
    }

    // Remove all actors.
    RemoveAllActors(&game->world.actor_list);

    // Center camera in world.
    TileCoord center = {
        game->world.map.width / 2,
        game->world.map.height / 2
    };
    game->render_info.camera = TileCoordToScaledWorldCoord(center, vec2_zero);

    StartFadeIn(&game->fade_state, 1.0f);
    menu_state = MENU_MAIN;
    PushState(game, &gs_menu);
}


const GameState gs_title_screen = {
    .render = TitleScreen_Render,
    .on_enter = TitleScreen_OnEnter,
};
