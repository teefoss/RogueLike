//
//  main.c
//  GameBeta
//
//  Created by Thomas Foster on 11/2/22.
//

/*
 https://www.pinterest.com/pin/725290714965873125/
 */

#include "main.h"
#include "debug.h"

#include "mathlib.h"
#include "sound.h"
#include "video.h"
#include "texture.h"

#include <stdio.h>

bool LevelIdleProcessInput(game_t * game, const SDL_Event * event);
void LevelIdleUpdate(game_t * game, float dt);
void LevelTurnUpdate(game_t * game, float dt);
void LevelTurnOnEnter(game_t * game);

void GamePlayRender(const game_t * game);

void IntermissionRender(const game_t * game);
void IntermissionOnExit(game_t * game);

const game_state_t null = { 0 };

const game_state_t level_idle = {
        .process_input  = LevelIdleProcessInput,
        .update         = LevelIdleUpdate,
        .render         = GamePlayRender,
        .on_enter       = NULL,
        .on_exit        = NULL,
        .duration_ticks = -1,
        .next_state     = NULL,
};

const game_state_t level_turn = {
        .process_input  = NULL,
        .update         = LevelTurnUpdate,
        .render         = GamePlayRender,
        .on_enter       = LevelTurnOnEnter,
        .on_exit        = NULL,
        .duration_ticks = -1,
        .next_state     = &level_idle,
};

const game_state_t intermission = {
        .process_input  = NULL,
        .update         = NULL,
        .render         = IntermissionRender,
        .on_enter       = NULL,
        .on_exit        = IntermissionOnExit,
        .duration_ticks = MS2TICKS(3000, FPS),
        .next_state     = &level_idle,
};


void LoadLevel(game_t * game, int level_num)
{
    GenerateDungeon(game, MAP_WIDTH, MAP_HEIGHT);
    PlayerCastSightLines(game, &game->actors[0]);

    game->player_turns = INITIAL_TURNS;
    game->has_gold_key = false;
    game->level = level_num;
}


#pragma mark - Game State

void ChangeState(game_t * game, const game_state_t * new_state)
{
    if ( game->state->on_exit ) {
        game->state->on_exit(game);
    }

    game->state = new_state;

    if ( game->state->on_enter ) {
        game->state->on_enter(game);
    }

    if ( game->state->duration_ticks != -1 ) {
        game->state_timer = game->state->duration_ticks;
    }
}

void UpdateState(game_t * game, float dt)
{
    if ( game->state->duration_ticks != -1 ) {
        // Run timer for finite length states
        if ( --game->state_timer <= 0 ) {
            ChangeState(game, game->state->next_state);
        }
    }

    if ( game->state->update ) {
        game->state->update(game, dt);
    }
}

#pragma mark - Intermission State

bool DoIntermissionInput(game_t * game, const SDL_Event * event)
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

void IntermissionOnExit(game_t * game)
{
    GenerateDungeon(game, MAP_WIDTH, MAP_HEIGHT);
    game->player_turns = INITIAL_TURNS;
    PlayerCastSightLines(game, &game->actors[0]);
}

#pragma mark - LEVEL IDLE STATE

void MovePlayer(game_t * game, int dx, int dy)
{
    actor_t * player = &game->actors[0]; // TODO: GetPlayer() instead
    player->was_attacked = false;

    game->log[0] = '\0'; // Clear the log.

    // The tile we are moving to.
    tile_t * tile = GetTile(&game->map, player->x + dx, player->y + dy);

    switch ( (tile_type_t)tile->type ) {

        case TILE_WALL:
            SetUpBumpAnimation(player, dx, dy);
            S_Play("l32o0de-");

            // TODO: Game design: Hitting a wall still causes monsters to update?
            // --turns
            break;

        case TILE_DOOR:
            SetUpBumpAnimation(player, dx, dy);
            S_Play("l32o2c+f+b");
            *tile = CreateTile(TILE_FLOOR); // Open (remove) the door.
            break;

        case TILE_GOLD_DOOR:
            SetUpBumpAnimation(player, dx, dy);
            if ( game->has_gold_key ) {
                S_Play("l32o2 c+g+dae-b-");
                *tile = CreateTile(TILE_FLOOR);
            } else {
                S_Play("l32o2 gc+");
                strncpy(game->log, "You need the Gold Key!", sizeof(game->log));
            }
            break;

        case TILE_START: // Really just a floor.
        case TILE_FLOOR:
            if ( TryMoveActor(player, game, dx, dy) ) {
                UpdateDistanceMap(&game->map, player->x, player->y, true);
            }
            game->player_turns--;
            break;

        case TILE_EXIT:
            MoveActor(player, dx, dy);
            S_Play("l32o3bb-a-fd-<a-d<g");
            break;

        default:
            break;
    }

    if ( dx ) {
        player->facing_left = dx < 0;
    }

    PlayerCastSightLines(game, &game->actors[0]);
    ChangeState(game, &level_turn);

    // Update all actors when player is out of turns.
    if ( game->player_turns < 0 ) {
        game->player_turns = INITIAL_TURNS;

        // Do all actor turns.
        for ( int i = 1; i < game->num_actors; i++ ) {
            actor_t * actor = &game->actors[i];

            if ( actor->action && !actor->was_attacked) {
                actor->action(actor, game);
            }

            actor->was_attacked = false; // reset
        }
    }
}

bool InventoryIsEmtpy(const inventory_t * inventory)
{
    for ( int i = 0; i < NUM_ITEMS; i++ ) {
        if ( inventory->item_counts[i] > 0 ) {
            return false;
        }
    }

    return true;
}

void ChangeInventorySelection(inventory_t * inventory, int direction)
{
    if ( InventoryIsEmtpy(inventory) ) {
        return;
    }

    do {
        inventory->selected_item += direction;

        // Clamp
        if ( inventory->selected_item < 0 ) {
            inventory->selected_item = NUM_ITEMS - 1;
        } else if ( inventory->selected_item == NUM_ITEMS ) {
            inventory->selected_item = 0;
        }

        // Scroll past any items the player doesn't have.
    } while ( inventory->item_counts[inventory->selected_item] == 0 );
}

void UseItem(game_t * game)
{
    inventory_t * in = &game->inventory;
    actor_t * player = &game->actors[0];

    // If the inventory is empty, just leave.
    if ( in->item_counts[in->selected_item] == 0 ) {
        return;
    }

    // Remove from inventory.
    --in->item_counts[in->selected_item];

    switch ( in->selected_item ) {
        case ITEM_HEALTH:
            if ( player->health < player->max_health ) {
                player->health++;
            }
            S_Play("l32 o3 d+ < g+ b e");
            break;
        case ITEM_TURN:
            game->player_turns++;
            S_Play("l32 o3 f+ < b a");
            break;
        default:
            break;
    }

    // Change selection if used the last of this item.
    if ( in->item_counts[in->selected_item] == 0 ) {
        ChangeInventorySelection(in, 1);
    }
}

bool LevelIdleProcessInput(game_t * game, const SDL_Event * event)
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
                case SDLK_RIGHT:
                    ChangeInventorySelection(&game->inventory, 1);
                    return true;
                case SDLK_LEFT:
                    ChangeInventorySelection(&game->inventory, -1);
                    return true;
                case SDLK_RETURN:
                    UseItem(game);
                    return true;
                default:
                    return false;
            }
            break;
        default:
            return false;
    }
}

void LevelIdleUpdate(game_t * game, float dt)
{
    actor_t * player = &game->actors[0];
    if ( GetTile(&game->map, player->x, player->y)->type == TILE_EXIT ) {
        ChangeState(game, &intermission);
        LoadLevel(game, game->level + 1);
    }

    // Update actor standing animations, etc.
    for ( int i = 0; i < game->num_actors; i++ ) {
        actor_t * actor = &game->actors[i];

        if (actor->num_frames > 1 &&
            game->ticks % MS2TICKS(actor->frame_msec, FPS) == 0 )
        {
            actor->frame = (actor->frame + 1) % actor->num_frames;
        }
    }
}

#pragma mark - LEVEL TURN STATE

void LevelTurnOnEnter(game_t * game)
{
    game->move_timer = 0.0f;
}

/// Run the move timer and do actor movement animations.
void LevelTurnUpdate(game_t * game, float dt)
{
    game->move_timer += 5.0f * dt;

    if ( game->move_timer >= 1.0f ) {
        // We're done.
        game->move_timer = 1.0f;
        ChangeState(game, &level_idle);
    }

    for ( int i = 0; i < game->num_actors; i++ ) {
        actor_t * actor = &game->actors[i];

        if ( actor->animation ) {
            actor->animation(actor, game->move_timer);
            if ( game->move_timer == 1.0f ) {
                actor->animation = NULL; // Remove the animation.
            }
        }
    }
}

#pragma mark - RENDER

void RenderHUD(const game_t * game)
{
    SDL_Texture * icons = GetTexture("assets/icons.png");

    const int margin = 16;
    const int char_w = V_CharWidth();
    const int char_h = V_CharHeight();
    V_SetGray(255);

    //
    // Top HUD
    //

    // Level

    V_PrintString(margin, margin, "Level %d", game->level);
    if ( game->has_gold_key ) {
        SDL_Rect src = { 5 * 5, 0, 5, 5 };
        SDL_Rect dst = {
            margin,
            margin * 2 + DRAW_SCALE,
            5 * DRAW_SCALE,
            5 * DRAW_SCALE };
        V_DrawTexture(icons, &src, &dst);
    }

    // Log

    // FIXME: last character not appearing)
    int log_len = (int)strlen(game->log);
    if ( log_len ) {
        int log_x = GAME_WIDTH - (log_len * char_w + margin);

        if ( game->state == &level_turn ) {
            float x = log_x;
            float y = Lerp(-margin, margin, game->move_timer);
            V_PrintString(x, y, game->log);
        } else {
            V_PrintString(log_x, margin, game->log);
        }
    }



    //
    // Lower HUD
    //

    int hud_x = margin;
    int hud_y = GAME_HEIGHT - (V_CharHeight() + margin);

    // Turns

    const int icon_size = 5 * DRAW_SCALE;
    SDL_Rect src = { 10, 0, 5, 5 };
    SDL_Rect dst = { 0, hud_y, icon_size, icon_size };
    dst.x = V_PrintString(hud_x, hud_y, " Turns ");

    for ( int i = 0; i < game->player_turns; i++ ) {
        V_DrawTexture(icons, &src, &dst);
        dst.x += 6 * DRAW_SCALE;
    }

    // Attack

    hud_y -= char_h;
    dst.y = hud_y;
    dst.x = V_PrintString(hud_x, hud_y, "Attack ");
    src.x = 4 * 5;

    for ( int i = 1; i <= game->actors[0].damage; i++  ) {
        V_DrawTexture(icons, &src, &dst);
        dst.x += 6 * DRAW_SCALE;
    }

    // Health

    hud_y -= char_h;
    dst.y = hud_y;
    dst.x = V_PrintString(hud_x, hud_y, "Health ");

    for ( int i = 1; i <= game->actors[0].max_health; i++ ) {
        if ( i > game->actors[0].health ) {
            src.x = 5;
        } else {
            src.x = 0;
        }
        V_DrawTexture(icons, &src, &dst);
        dst.x += 6 * DRAW_SCALE;
    }

    // Inventory

    hud_y += char_h;
    int inventory_x = GAME_WIDTH - (NUM_ITEMS * icon_size + margin);
    for ( int i = 0; i < NUM_ITEMS; i++ ) {
        if ( game->inventory.item_counts[i] == 0 ) {
            continue;
        }

        src.x = i * 5;
        src.y = 5;
        dst.x = inventory_x + (i * (icon_size + (1 * DRAW_SCALE)));
        V_DrawTexture(icons, &src, &dst);
        V_PrintString(dst.x, dst.y + char_h, "%d", game->inventory.item_counts[i]);
        if ( i == game->inventory.selected_item ) {
            src.x = 15;
            src.y = 0;
            dst.y -= icon_size + DRAW_SCALE;
            V_DrawTexture(icons, &src, &dst);
            dst.y += icon_size + DRAW_SCALE;
        }
    }
}




void GamePlayRender(const game_t * game)
{
    RenderMap(game);

    if ( show_debug_info ) {
//        DEBUG_PRINT("TILE %d, %d:", mouse_tile_x, mouse_tile_y);
//        DEBUG_PRINT("  type: %d", game->map.tiles[mouse_tile_y][mouse_tile_x]);
    } else {
        RenderHUD(game);
    }
}




void IntermissionRender(const game_t * game)
{
    const char * level_string = "Level %d";

    V_SetRGBA(0, 0, 0, 0);
    int width = V_PrintString(0, 0, level_string, game->level);

    V_SetRGB(255, 255, 255);
    int x = (GAME_WIDTH - width) / 2;
    int y = (GAME_HEIGHT - V_CharHeight()) / 2;
    V_PrintString(x, y, level_string, game->level);
}




void UpdateGame(game_t * game, float dt)
{
    box_t visible_region = GetVisibleRegion(&game->map, &game->actors[0]);

    // Reset tile light and blocks.
    for ( int y = visible_region.min.y; y <= visible_region.max.y; y++ ) {
        for ( int x = visible_region.min.x; x <= visible_region.max.x; x++ ) {
            GetTile(&game->map, x, y)->light_target = 0;
        }
    }

    UpdateState(game, dt);

    // TODO: refine the order and location of this update code:

    // Remove actors flagged to be removed.
    for ( int i = game->num_actors - 1; i >= 0; i-- ) {
        actor_t * actor = &game->actors[i];

        if ( actor->remove ) {
            game->actors[i] = game->actors[--game->num_actors];
        }
    }

    // Update Actors: cast light, run timers.
    for ( int i = 0; i < game->num_actors; i++ ) {
        actor_t * actor = &game->actors[i];
        CastLight(game, actor);

        if ( actor->hit_timer > 0.0f ) {
            actor->hit_timer -= 5.0f * dt;
        }
    }

    // Update map light.
    // Get the visible region again, in case the player has moved.
    visible_region = GetVisibleRegion(&game->map, &game->actors[0]);

    for ( int y = visible_region.min.y; y <= visible_region.max.y; y++ ) {
        for ( int x = visible_region.min.x; x <= visible_region.max.x; x++ ) {
            tile_t * tile = GetTile(&game->map, x, y);

            // Decide what light level to fade this tile to, and at what rate.
            float w; // lerp factor
            int target;
            if ( tile->visible ) {
                // Tile is visible, light it to at least 80, maybe more.
                target = MAX(80, tile->light_target); // was 80
                w = 0.2f; // Light it up quickly.
            } else if ( tile->revealed ) {
                // Not visible, but seen it before. It's dim.
                target = 20; // previous: 40
                w = 0.05f; // fade it out slowly.
            } else {
                // Completely unrevealed.
                target = 0;
                w = 1.0f; // (Shouldn't actually matter)
            }

            tile->light = Lerp((float)tile->light, (float)target, w);
        }
    }

    // Update camera.
    actor_t * player = GetPlayer(game->actors, game->num_actors);
    vec2_t offset = GetRenderOffset(player);
    game->camera = Vec2LerpEpsilon(game->camera, offset, 0.2f, 1.0f);

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

        if (   game->state->process_input
            && game->state->process_input(game, &event) )
        {
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
                        GenerateDungeon(game, MAP_WIDTH, MAP_HEIGHT);
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

    UpdateGame(game, dt);

    // Render:

    V_ClearRGB(0, 0, 0);
    game->state->render(game);

    V_Refresh();

    game->ticks++;
}





#pragma mark -

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

    game_t * game = calloc(1, sizeof(*game));
    if ( game == NULL ) {
        Error("Could not allocate game");
    }

    game->state = &null;
    game->is_running = true;
    game->ticks = 0;
    LoadLevel(game, 1);
    ChangeState(game, &level_idle);

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
