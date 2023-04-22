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

//    SDL_DisplayMode display_mode;
//    SDL_GetCurrentDisplayMode(0, &display_mode);
//    printf("desktop size: %d x %d\n", display_mode.w, display_mode.h);

    video_info_t info = {
        .window_width = GAME_WIDTH * 1.5,
        .window_height = GAME_HEIGHT * 1.5,
//        .render_flags = SDL_RENDERER_PRESENTVSYNC,
        .window_flags = SDL_WINDOW_ALLOW_HIGHDPI,
        .render_flags = 0,
    };

    V_InitVideo(&info);

    SDL_RenderSetLogicalSize(renderer, GAME_WIDTH, GAME_HEIGHT);
    V_SetFont(FONT_4X6);
    V_SetTextScale(DRAW_SCALE, DRAW_SCALE);

    S_InitSound();

    Game * game = InitGame();

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
    free(game->world.map.tiles);
    free(game->world.map.tile_ids);
    free(game);

    return 0;
}
