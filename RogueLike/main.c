//
//  main.c
//  RogueLike
//
//  Created by Thomas Foster on 3/27/23.
//

#include "genlib.h"
#include "video.h"
#include "mathlib.h"
#include "sound.h"
#include "game.h"
#include "debug.h"
#include "world.h"

int main(void)
{
    Randomize();

    if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0 ) {
        Error("Could not init SDL: %s", SDL_GetError());
    }

    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    printf("desktop size: %d x %d\n", display_mode.w, display_mode.h);
    float aspect = (float)display_mode.h / (float)display_mode.w;

    int game_height = 18 * SCALED(TILE_SIZE);
    int game_width = (float)game_height / aspect;

    video_info_t info = {
        .window_width = game_width,
        .window_height = game_height,
//        .render_flags = SDL_RENDERER_PRESENTVSYNC,
        .window_flags = SDL_WINDOW_ALLOW_HIGHDPI,
        .render_flags = 0,
    };

    V_InitVideo(&info);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetLogicalSize(renderer, game_width, game_height);
    V_SetFont(FONT_4X6);
    V_SetTextScale(DRAW_SCALE, DRAW_SCALE);

    S_InitSound();

    printf("size of Game: %zu bytes\n", sizeof(Game));
    printf("size of World: %zu bytes\n", sizeof(World));
    printf("size of Map: %zu bytes\n", sizeof(Map));
    printf("size of Actor: %zu bytes\n", sizeof(Actor));

    Game * game = InitGame(game_width, game_height);

    int old_time = SDL_GetTicks();
    const float target_dt = 1.0f / FPS;

    while ( game->is_running ) {
        int new_time = SDL_GetTicks();
        float dt = (float)(new_time - old_time) / 1000.0f;

        if ( dt < target_dt ) {
            SDL_Delay(1);
            continue;
        }

        PROFILE(DoFrame(game, target_dt), frame_msec);
        if ( frame_msec > max_frame_msec ) {
            max_frame_msec = frame_msec;
        }

        old_time = new_time;
    }


    FreeDistanceMapQueue();
    DestroyActorList(&game->world.actor_list);
    FreeRenderAssets(&game->render_info);
    FreeVisibleActorsArray();
    free(game->world.map.tiles);
    free(game->world.map.tile_ids);
    free(game);

    return 0;
}
