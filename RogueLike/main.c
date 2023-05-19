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
#include "config.h"

static SDL_Rect InitVideo(void)
{
    // Create the window with the same asepct ratio as the desktop resoltuion.

    // Calculate the monitor's aspect ratio.
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    float aspect = (float)display_mode.h / (float)display_mode.w;

    SDL_Rect size;
    size.h = 18 * SCALED(TILE_SIZE); // Height is fixed as 18 tiles high.
    size.w = (float)size.h / aspect; // Width derived from aspect ratio.

    u32 window_flags = SDL_WINDOW_ALLOW_HIGHDPI;
    if ( cfg_fullscreen ) {
        window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    video_info_t info = {
        .window_width = size.w * cfg_window_scale,
        .window_height = size.h * cfg_window_scale,
//        .render_flags = SDL_RENDERER_PRESENTVSYNC,
        .window_flags = window_flags,
        .render_flags = 0,
    };

    V_InitVideo(&info);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderSetLogicalSize(renderer, size.w, size.h);
    V_SetFont(FONT_4X6);
    V_SetTextScale(DRAW_SCALE, DRAW_SCALE);

    return size;
}

int main(void)
{
    if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0 ) {
        Error("Could not init SDL: %s", SDL_GetError());
    }

    Randomize();
    LoadConfigFile();
    SDL_Rect game_size = InitVideo();
    S_InitSound();
    Game * game = InitGame(game_size.w, game_size.h);

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

    SaveConfigFile();

    FreeDistanceMapQueue();
    DestroyActorList(&game->world.map->actor_list);
    FreeRenderAssets(&game->render_info);
    FreeVisibleActorsArray();
    free(game->world.map->tiles);
    free(game->world.map->tile_ids);
    free(game);

    return 0;
}
