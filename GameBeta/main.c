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

int main(void)
{
    Randomize();

    float window_scale = 1.5;

    video_info_t info = {
        .window_width = GAME_WIDTH * window_scale,
        .window_height = GAME_HEIGHT * window_scale,
//        .render_flags = SDL_RENDERER_PRESENTVSYNC,
        .window_flags = SDL_WINDOW_ALLOW_HIGHDPI,
        .render_flags = 0,
    };

    V_InitVideo(&info);

    SDL_RenderSetLogicalSize(renderer, GAME_WIDTH, GAME_HEIGHT);
    V_SetFont(FONT_4X6);
    V_SetTextScale(DRAW_SCALE, DRAW_SCALE);

    S_InitSound();

    game_t * game = InitGame();

    int old_time = SDL_GetTicks();
    const float target_dt = 1.0f / FPS;

    while ( game->is_running ) {
        int new_time = SDL_GetTicks();
        float dt = (float)(new_time - old_time) / 1000.0f;

        if ( dt < target_dt ) {
            SDL_Delay(1);
            continue;
        }

//        PROFILE_START(frame_time);
        DoFrame(game, target_dt);
//        PROFILE_END(frame_time);
        old_time = new_time;
    }

    FreeDistanceMapQueue();
    free(game->map.tiles);
    free(game->map.tile_ids);
    free(game);

    return 0;
}
