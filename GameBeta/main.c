//
//  main.c
//  GameBeta
//
//  Created by Thomas Foster on 11/2/22.
//

#include "main.h"
#include "debug.h"

#include "mathlib.h"
#include "sound.h"
#include "video.h"

#include <stdio.h>

bool GameInputIdle(game_t * game, const SDL_Event * event);
void GameUpdateIdle(game_t * game, float dt);

void GameUpdateIdle(game_t * game, float dt)
{
    // Update actor standing animations.
    for ( int i = 0; i < game->num_actors; i++ ) {
        actor_t * actor = &game->actors[i];

        if ( game->ticks % MS2TICKS(actor->frame_msec, FPS) == 0 ) {
            actor->frame = (actor->frame + 1) % actor->num_frames;
        }
    }
}

/// Run the move timer and move actors.
void GameUpdateActorAnimations(game_t * game, float dt)
{
    game->move_timer += 5.0f * dt;
    bool done = false;

    if ( game->move_timer >= 1.0f ) {
        // We're done.
        game->move_timer = 1.0f;
        game->update = GameUpdateIdle;
        game->do_input = GameInputIdle;
        done = true;
    }

    for ( int i = 0; i < game->num_actors; i++ ) {
        actor_t * actor = &game->actors[i];

        if ( actor->animation ) {
            if ( done ) {
                actor->offset = (vec2_t){ 0 };
            } else {
                actor->animation(actor, game->move_timer);
            }
        }
    }
}

void SetUpAnimationGameState(game_t * game)
{
    game->move_timer = 0.0f;
    game->update = GameUpdateActorAnimations;
    game->do_input = NULL;
}

void MovePlayer(game_t * game, int dx, int dy)
{
    actor_t * player = &game->actors[0];

    switch ( game->map.tiles[player->y + dy][player->x + dx].type ) {
        case TILE_WALL:
            SetUpBumpAnimation(player, dx, dy);
            SetUpAnimationGameState(game);
            S_Play("l32o0de-");
            return; // do nothing with other actors
        case TILE_FLOOR:
            if ( TryMoveActor(player, game, dx, dy) ) {
                PlayerCastSightLines(&game->map, player);
                UpdateDistanceMap(game->map.tiles, player->x, player->y);
                game->player_turns--;
            }
            break;
        default:
            break;
    }

    SetUpAnimationGameState(game);

    // Update all actors when player is out of turns.
    if ( game->player_turns == 0 ) {
        game->player_turns = INITIAL_TURNS;

        // Do all actor turns.
        for ( int i = 1; i < game->num_actors; i++ ) {
            actor_t * actor = &game->actors[i];

            if ( actor->action ) {
                actor->action(actor, game);
            }
        }
    }
}

bool GameInputIdle(game_t * game, const SDL_Event * event)
{
    switch ( event->type ) {
        case SDL_KEYDOWN:

            if ( event->key.repeat > 0 ) {
                return false;
            }

            switch ( event->key.keysym.sym ) {
                case SDLK_w:
                    MovePlayer(game, 0, -1);
                    return true;
                case SDLK_s:
                    MovePlayer(game, 0, 1);
                    return true;
                case SDLK_a:
                    MovePlayer(game, -1, 0);
                    return true;
                case SDLK_d:
                    MovePlayer(game, 1, 0);
                    return true;
                default:
                    return false;
            }
            break;
        default:
            return false;
    }
}

void DoFrame(game_t * game, float dt)
{
    debug_row = 0;

    int mouse_tile_x, mouse_tile_y;
    SDL_GetMouseState(&mouse_tile_x, &mouse_tile_y);
    mouse_tile_x /= TILE_SIZE * DRAW_SCALE;
    mouse_tile_y /= TILE_SIZE * DRAW_SCALE;

    SDL_Event event;
    while ( SDL_PollEvent(&event) ) {

        if ( game->do_input && game->do_input(game, &event) ) {
            continue;
        }

        // Game didn't process this event. Handle some universal events:
        switch ( event.type ) {
            case SDL_QUIT:
                game->is_running = false;
                return;
            case SDL_KEYDOWN:
                switch ( event.key.keysym.sym ) {
                    case SDLK_g:
                        GenerateMap(game);
                        break;
                    case SDLK_BACKQUOTE:
                        show_debug_info = !show_debug_info;
                        break;
                    case SDLK_BACKSLASH:
                        V_ToggleFullscreen(DESKTOP);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    // Reset tile light.
    box_t visible_region = GetVisibleRegion(&game->actors[0]);

    for ( int y = visible_region.min.y; y <= visible_region.max.y; y++ ) {
        for ( int x = visible_region.min.x; x <= visible_region.max.x; x++ ) {
            game->map.tiles[y][x].light_target = 0;
        }
    }

    if ( game->update ) {
        game->update(game, dt);
    }

    // Do post-update stuff, cast actor light, remove removeables.
    for ( int i = game->num_actors - 1; i >= 0; i-- ) {
        actor_t * actor = &game->actors[i];

        if ( actor->remove ) {
            game->actors[i] = game->actors[--game->num_actors];
        } else {
            CastLight(actor, game->map.tiles);

            if ( actor->hit_timer > 0.0f ) {
                actor->hit_timer -= 5.0f * dt;
            }
        }
    }

    // Update tile light.
    visible_region = GetVisibleRegion(&game->actors[0]);

    for ( int y = visible_region.min.y; y <= visible_region.max.y; y++ ) {
        for ( int x = visible_region.min.x; x <= visible_region.max.x; x++ ) {
            tile_t * tile = &game->map.tiles[y][x];

            // Decide what light level to fade this tile to, and at what rate.
            float w; // lerp factor
            int target;
            if ( tile->visible ) {
                // Tile is visible, light it to at least 80, maybe more.
                target = MAX(80, tile->light_target);
                w = 0.2f; // Light it up quickly.
            } else if ( tile->revealed ) {
                // Not visible, but seen it before. It's dim.
                target = 40;
                w = 0.05f; // fade it out slowly.
            } else {
                // Completely unrevealed.
                target = 0;
                w = 1.0f; // (Shouldn't actually matter)
            }

            tile->light = Lerp((float)tile->light, (float)target, w);
        }
    }

    V_ClearRGB(0, 0, 0);
    RenderMap(game);

#if 0
    // TODO: TEMP
    // middle-of-screen indicators
    V_SetRGB(64, 0, 0);
    V_DrawVLine(GAME_WIDTH / 2, 0, GAME_HEIGHT);
    V_DrawHLine(0, GAME_WIDTH, GAME_HEIGHT / 2);
#endif

    int hud_x = V_CharWidth();
    int hud_y = GAME_HEIGHT - V_CharHeight() * 2;
    V_SetGray(255);
    V_PrintString(hud_x, hud_y, "TURNS: %d", game->player_turns);

    DEBUG_PRINT("TILE %d, %d:", mouse_tile_x, mouse_tile_y);
    DEBUG_PRINT("  type: %d", game->map.tiles[mouse_tile_y][mouse_tile_x]);

    V_Refresh();

    game->ticks++;
}

int main(void)
{
    Randomize();

    video_info_t info = {
        .window_width = 1920,
        .window_height = 1080,
        .render_flags = SDL_RENDERER_PRESENTVSYNC,
    };
    V_InitVideo(&info);
    SDL_RenderSetLogicalSize(renderer, GAME_WIDTH, GAME_HEIGHT);
    V_SetFont(FONT_CP437_8X8);
    V_SetTextScale(DRAW_SCALE, DRAW_SCALE);

    S_InitSound();

    game_t * game = calloc(1, sizeof(*game));
    if ( game == NULL ) {
        Error("Could not allocate game");
    }

    GenerateMap(game);
    game->do_input = GameInputIdle;
    game->update = GameUpdateIdle;
    game->is_running = true;
    game->ticks = 0;
    game->player_turns = INITIAL_TURNS;
    PlayerCastSightLines(&game->map, &game->actors[0]);

    u64 old_time = SDL_GetPerformanceCounter();

    while ( game->is_running ) {
        float new_time = SDL_GetPerformanceCounter();
        float dt = (float)(new_time - old_time) / (float)SDL_GetPerformanceFrequency();

        if ( dt < 1.0f / FPS ) {
            SDL_Delay(1);
            continue;
        }

        dt = 1.0f / FPS;

        DoFrame(game, dt);

        old_time = new_time;
    }

    free(game);

    return 0;
}
