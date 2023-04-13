//
//  main.c
//  RogueLike
//
//  Created by Thomas Foster on 11/2/22.
//

/*
 https://www.pinterest.com/pin/725290714965873125/
 */

#include "game.h"
#include "debug.h"
#include "icon.h"
#include "gen.h"

#include "mathlib.h"
#include "sound.h"
#include "video.h"
#include "texture.h"

#include <stdio.h>

bool LevelIdleProcessInput(game_t * game, const SDL_Event * event);
void LevelIdleUpdate(game_t * game, float dt);
void LevelIdleOnEnter(game_t * game);

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
        .on_enter       = LevelIdleOnEnter,
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


void GenerateLevel(game_t * game)
{
    switch ( game->level ) {
        case 1:
            GenerateForest(game);
            break;
        case 2:
            // TODO: The Well
            GenerateDungeon(game, 31, 31);
            break;
        default:
            break;
    }
}


/// In the visible rect, set each tile's light level according to its visibility
/// flags.
void SetTileLight(game_t * game, area_t area)
{
    const area_info_t * info = &area_info[area];
    box_t vis = GetVisibleRegion(game);

    for ( int y = vis.min.y; y <= vis.max.y; y++ ) {
        for ( int x = vis.min.x; x <= vis.max.x; x++ ) {
            tile_coord_t coord = { x, y };
            tile_t * tile = GetTile(&game->map, coord);

            if ( tile == NULL ) {
                Error("error: somehow NULL tile in vis rect!");
            }

            if ( tile->flags.visible || info->reveal_all ) {
                printf("- tile %d, %d visible\n", x, y);
                tile->light = info->visible_light;
                printf("- tile %d, %d light: %d\n", x, y, tile->light);
            } else if ( tile->flags.revealed ) {
                printf("- tile %d, %d revealed\n", x, y);
                tile->light = info->revealed_light;
                printf("- tile %d, %d light: %d\n", x, y, tile->light);
            } else {
                printf("- tile %d, %d unrevealed\n", x, y);
                tile->light = info->unrevealed_light;
                printf("- tile %d, %d light: %d\n", x, y, tile->light);
            }
        }
    }
}


void LoadLevel(game_t * game, int level_num)
{
    game->level = level_num;

    GenerateLevel(game);

    PlayerCastSight(game);
    SetTileLight(game, game->area);

    PrintTilesAreDarkThatShouldntBe(&game->map, "LoadLevel");
//    CastLight(game, &game->map.actors.list[0]);

//    for ( int i = 0; i < game->map.actors.count; i++ ) {
//        CastLight(game, &game->map.actors.list[i]);
//    }

    game->player_turns = INITIAL_TURNS;
    game->has_gold_key = false;
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
    GenerateDungeon(game, 31, 31);
    game->player_turns = INITIAL_TURNS;
    PlayerCastSight(game);
}



#pragma mark - LEVEL IDLE STATE


void LevelIdleOnEnter(game_t * game)
{
    // Check if the player is standing on a teleporter.
    actor_t * player = GetPlayer(game);
    if ( player->flags.on_teleporter ) {
        Teleport(player, player->tile);
        S_Play("o0 t160 l32 c g > d a > e b > f+ > c+ g+ > d+ a+ > f > c ");
        player->flags.on_teleporter = false;
    }
}


void MovePlayer(game_t * game, direction_t direction)
{
    actor_t * player = GetPlayer(game);
//    player->flags.was_attacked = false;

    game->log[0] = '\0'; // Clear the log.

    // Do player-tile collisions:

    // The tile we are moving to.
    tile_t * tile = GetAdjacentTile(&game->map, player->tile, direction);

    switch ( (tile_type_t)tile->type ) {

        case TILE_WALL:
            SetUpBumpAnimation(player, direction);
            S_Play("l32o0de-");

            // TODO: Game design: Hitting a wall still causes monsters to update?
            // --turns
            break;

        case TILE_DOOR:
            SetUpBumpAnimation(player, direction);
            S_Play("l32o2c+f+b");
            *tile = CreateTile(TILE_FLOOR); // Open (remove) the door.
            break;

        case TILE_GOLD_DOOR:
            SetUpBumpAnimation(player, direction);
            if ( game->has_gold_key ) {
                S_Play("l32o2 c+g+dae-b-");
                *tile = CreateTile(TILE_FLOOR);
            } else {
                S_Play("l32o2 gc+");
                strncpy(game->log, "You need the Gold Key!", sizeof(game->log));
            }
            break;

        case TILE_TELEPORTER:
            player->flags.on_teleporter = true;
        case TILE_START: // Really just a floor.
        case TILE_FLOOR:
            if ( TryMoveActor(player, direction) ) {
                CalculateDistances(&game->map, player->tile, 0);
            }
            game->player_turns--;
            break;

        case TILE_EXIT:
            MoveActor(player, direction);
            S_Play("l32o3bb-a-fd-<a-d<g");
            break;
        default:
            break;
    }

    UpdateActorFacing(player, XDelta(direction));

    PlayerCastSight(game);
    SetTileLight(game, game->area);
    CastLight(game, player);

    ChangeState(game, &level_turn);

    // Update all actors when player is out of turns.
    if ( game->player_turns < 0 ) {
        game->player_turns = INITIAL_TURNS;

        // Do all actor turns.
        actors_t * actors = &game->map.actors;
        for ( int i = 0; i < actors->count; i++ ) {
            actor_t * actor = &actors->list[i];

            if ( !actor->flags.remove ) {
                if ( !actor->flags.was_attacked && actor->action ) {
                    actor->action(actor);
                }

                actor->flags.was_attacked = false; // reset
                CastLight(game, actor);
            }
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
    actor_t * player = GetPlayer(game);

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
                    MovePlayer(game, NORTH);
                    return true;
                case SDLK_s:
                    MovePlayer(game, SOUTH);
                    return true;
                case SDLK_a:
                    MovePlayer(game, WEST);
                    return true;
                case SDLK_d:
                    MovePlayer(game, EAST);
                    return true;
                case SDLK_UP:
                    if ( game->inventory_open ) {
                        ChangeInventorySelection(&game->inventory, +1);
                    }
                    return true;
                case SDLK_DOWN:
                    if ( game->inventory_open ) {
                        ChangeInventorySelection(&game->inventory, -1);
                    }
                    return true;
                case SDLK_RETURN:
                    if ( game->inventory_open ) {
                        UseItem(game);
                    }
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
    actor_t * player = GetPlayer(game);
    if ( GetTile(&game->map, player->tile)->type == TILE_EXIT ) {
        ChangeState(game, &intermission);
        LoadLevel(game, game->level + 1);
    }

    // Update actor standing animations, etc.
    actors_t * actors = &game->map.actors;
    for ( int i = 0; i < actors->count; i++ ) {
        actor_t * actor = &actors->list[i];

        if (actor->sprite->num_frames > 1 &&
            game->ticks % MS2TICKS(actor->sprite->frame_msec, FPS) == 0 )
        {
            actor->frame = (actor->frame + 1) % actor->sprite->num_frames;
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

    for ( int i = 0; i < game->map.actors.count; i++ ) {
        actor_t * actor = &game->map.actors.list[i];

        if ( actor->animation ) {
            actor->animation(actor, game->move_timer);
            if ( game->move_timer == 1.0f ) {
                actor->animation = NULL; // Remove the animation.
            }
        }
    }
}

#pragma mark - RENDER


vec2_t GetWindowScale(void)
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    vec2_t scale = { w / (float)GAME_WIDTH, h / (float)GAME_HEIGHT };

    return scale;
}


int InventoryRenderX(inventory_t * inventory)
{
    int max_width = 0;

    if ( InventoryIsEmtpy(inventory) ) {
        return GAME_WIDTH - (V_CharWidth() * strlen("Inventory") + HUD_MARGIN * 2);
    }

    for ( int i = 0; i < NUM_ITEMS; i++ ) {
        if ( inventory->item_counts[i] == 0 ) {
            continue;
        }

        max_width = MAX(max_width, HUD_MARGIN * 2 + ItemInfoWidth(i));
    }

    return GAME_WIDTH - max_width;
}


/// Level area rect. Width and height are scaled.
SDL_Rect GetLevelViewport(const game_t * game)
{
    int inventory_width = GAME_WIDTH - game->inventory_x;
    SDL_Rect viewport = { 0, 0, GAME_WIDTH - inventory_width, GAME_HEIGHT };

    return viewport;
}


void RenderHUD(const game_t * game, const actor_t * player)
{
    const int margin = HUD_MARGIN;
    const int char_w = V_CharWidth();
    const int char_h = V_CharHeight();
    V_SetGray(255);

    //
    // Top HUD
    //

    // Level

    V_PrintString(margin, margin, "Level %d", game->level);
    if ( game->has_gold_key ) {
        RenderIcon(ICON_GOLD_KEY, margin, margin * 2 + SCALED(1));
    }

    // Log

    int log_len = (int)strlen(game->log);
    if ( log_len ) {
        int log_x = game->inventory_x - (log_len * char_w + margin);

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

    int turns_x = V_PrintString(hud_x, hud_y, " Turns ");

    for ( int i = 0; i < game->player_turns; i++ ) {
        RenderIcon(ICON_TURN, turns_x + i * SCALED(ICON_SIZE), hud_y);
    }

    // Attack

    hud_y -= char_h;
    int attack_x = V_PrintString(hud_x, hud_y, "Attack ");

    for ( int i = 0; i < player->damage; i++  ) {
        RenderIcon(ICON_DAMAGE, attack_x + i * SCALED(ICON_SIZE), hud_y);
    }

    // Health

    hud_y -= char_h;

    int health_x = V_PrintString(hud_x, hud_y, "Health ");

    for ( int i = 0; i < player->max_health; i++ ) {
        icon_t icon = i > player->health ? ICON_EMPTY_HEART : ICON_FULL_HEART;
        RenderIcon(icon, health_x + i * SCALED(ICON_SIZE), hud_y);
    }
}


void RenderForestBackground(const game_t * game)
{
    for ( int i = 0; i < NUM_STARS; i++ ) {
        V_SetGray(232);
        SDL_Rect r = {
            game->stars[i].x,
            game->stars[i].y,
            SCALED(1) / 2,
            SCALED(1) / 2
        };
        V_FillRect(&r);
    }

    // TODO: refactor
    float phi = 0.61803398875f;
    int moon1_x = (float)GAME_WIDTH * phi;
    int moon1_y = (float)GAME_HEIGHT * phi;
    int moon1_size = SCALED(TILE_SIZE * 3);

    SDL_Rect moon1_rect = {
        .x = moon1_x - moon1_size / 2,
        .y = moon1_y - moon1_size / 2,
        .w = moon1_size,
        .h = moon1_size,
    };
    V_FillRect(&moon1_rect);

    int moon2_x = (float)GAME_WIDTH * (1 - phi);
    int moon2_y = (float)GAME_HEIGHT * (1 - phi);
    int moon2_size = (int)((float)moon1_size * phi);

    SDL_Rect moon2_rect = {
        .x = moon2_x - moon2_size / 2,
        .y = moon2_y - moon2_size / 2,
        .w = moon2_size,
        .h = moon2_size,
    };
    V_FillRect(&moon2_rect);
}


void RenderInventory(const game_t * game)
{
    SDL_Rect inventory_panel = {
        .x = game->inventory_x,
        .y = 0,
        .w = GAME_WIDTH - game->inventory_x,
        .h = GAME_HEIGHT
    };

    V_SetGray(16);
    V_FillRect(&inventory_panel);
    V_SetGray(32);
    V_DrawVLine(game->inventory_x, 0, GAME_HEIGHT);

    inventory_panel.x += HUD_MARGIN;
    inventory_panel.y += HUD_MARGIN;
    inventory_panel.w -= HUD_MARGIN;
    inventory_panel.h -= HUD_MARGIN;
    SDL_RenderSetViewport(renderer, &inventory_panel);

    V_SetRGB(255, 255, 255);
    int row = 0;
    int char_h = V_CharHeight();

    V_PrintString(0, row++ * char_h, "Inventory");
    row++;

    const inventory_t * in = &game->inventory;
    for ( int i = 0; i < NUM_ITEMS; i++ ) {
        if ( in->item_counts[i] ) {
            bool selected = in->selected_item == i;
            RenderItemInfo(i, in->item_counts[i], 0, row++ * char_h, selected);
        }
    }

    SDL_RenderSetViewport(renderer, NULL);
}


void GamePlayRender(const game_t * game)
{
    if ( show_debug_map ) {
        int size = area_info[game->area].debug_map_tile_size;

        RenderTilesInRegion(game, NULL, size, vec2_zero, true);

        box_t vis = GetVisibleRegion(game);
        SDL_Rect box = {
            .x = vis.min.x * size,
            .y = vis.min.y * size,
            .w = ((vis.max.x - vis.min.x) + 1) * size,
            .h = ((vis.max.y - vis.min.y) + 1) * size
        };

        V_SetRGB(255, 80, 80);
        V_DrawRect(&box);

        const actors_t * actors = &game->map.actors;
        for ( int i = 0; i < actors->count; i++ ) {
            const actor_t * actor = &actors->list[i];
            int x = actor->tile.x * size;
            int y = actor->tile.y * size;
            RenderActor(actor, x, y, size, true);
        }
    } else {
        if ( game->area == AREA_FOREST ) {
            RenderForestBackground(game);
        }

        RenderMap(game);

        if ( !show_debug_info ) {
            RenderHUD(game, GetPlayer(game));
        }

        if ( game->inventory_x != GAME_WIDTH ) {
            RenderInventory(game);
        }
    }

    if ( show_debug_info ) {
        DEBUG_PRINT("Frame time: %.1f (max: %.1f)",
                    frame_msec * 1000.0f,
                    max_frame_msec * 1000.0f);
        DEBUG_PRINT(" ");

        tile_t * hover = GetTile((map_t *)&game->map, game->mouse_tile);
        if ( hover ) {

            DEBUG_PRINT("Mouse tile: %d, %d (%s)",
                        game->mouse_tile.x,
                        game->mouse_tile.y,
                        TileName(hover->type));
            DEBUG_PRINT(" light: %d", hover->light);
            DEBUG_PRINT(" revealed: %s", hover->flags.revealed ? "yes" : "no");
            DEBUG_PRINT(" visible: %s", hover->flags.visible ? "yes" : "no");
            DEBUG_PRINT(" blocking: %s", hover->flags.blocking ? "yes" : "no");

            const actor_t * player = GetPlayer(game);
            bool los = LineOfSight((map_t *)&game->map,
                                   player->tile,
                                   game->mouse_tile);
            DEBUG_PRINT(" LOS: %s", los ? "yes" : "no");
        } else {

        }
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


#pragma mark -


game_t * InitGame(void)
{
    game_t * game = calloc(1, sizeof(*game));
    if ( game == NULL ) {
        Error("Could not allocate game");
    }

    game->state = &null;
    game->is_running = true;
    game->ticks = 0;
    game->inventory_open = false;
    game->inventory_x = GAME_WIDTH;
    InitParticleArray(&game->particles);
    LoadLevel(game, 2);
    ChangeState(game, &level_idle);

    return game;
}


void UpdateGame(game_t * game, float dt)
{
    actor_t * player = GetPlayer(game);
    actors_t * actors = &game->map.actors;

    UpdateState(game, dt);

    // TODO: make int remove_indices[] and int num_to_remove
    // If num_to_remove > 0, sort array decreasing
    for ( int i = actors->count - 1; i >= 0; i-- ) {
        actor_t * actor = &actors->list[i];

        if ( actor->flags.remove ) {
            actors->list[i] = actors->list[--actors->count];
        }
    }

    // Update Actors: run timers.
    for ( int i = 0; i < actors->count; i++ ) {
        actor_t * actor = &actors->list[i];

        if ( actor->hit_timer > 0.0f ) {
            actor->hit_timer -= 5.0f * dt;
        }
    }

    // Update camera.

    vec2_t offset = GetRenderOffset(player);
    game->camera = Vec2LerpEpsilon(game->camera, offset, 0.2f, 1.0f);

    // Update inventory panel position
    {
        float target;
        if ( game->inventory_open ) {
            target = InventoryRenderX(&game->inventory);
        } else {
            target = GAME_WIDTH; // Hidden
        }

        game->inventory_x = Lerp(game->inventory_x, target, 0.33f);
        if ( fabsf(target - game->inventory_x) < 1.0f ) {
            game->inventory_x = target;
        }
    }

    UpdateParticles(&game->particles, dt);

    // Update mouse tile
    {
        vec2_t window_scale = GetWindowScale();
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        // Adjust for window scale
        vec2_t mouse;
        mouse.x = (float)mx / window_scale.x;
        mouse.y = (float)my / window_scale.y;

        // Convert to tile and apply draw offset
        if ( show_debug_map ) {
            // (No draw offset)
            mouse.x /= area_info[game->area].debug_map_tile_size;
            mouse.y /= area_info[game->area].debug_map_tile_size;
        } else {
            mouse.x += offset.x;
            mouse.y += offset.y;
            mouse.x /= SCALED(TILE_SIZE);
            mouse.y /= SCALED(TILE_SIZE);
        }

        game->mouse_tile.x = mouse.x;
        game->mouse_tile.y = mouse.y;
    }
}

// TODO: profile function macro -> ms stored in debug.c global and displayed in debug screen
void DoFrame(game_t * game, float dt)
{
    debug_row = 0;

    SDL_Event event;
    while ( SDL_PollEvent(&event) ) {

        // Let the current game state process this event first.
        if (   game->state->process_input
            && game->state->process_input(game, &event) )
        {
            continue;
        }

        // State didn't process this event. Handle some universal events:
        switch ( event.type ) {
            case SDL_QUIT:
                game->is_running = false;
                return;
            case SDL_KEYDOWN:
                switch ( event.key.keysym.sym ) {
                    case SDLK_BACKSLASH:
                        V_ToggleFullscreen(DESKTOP);
                        break;
                    case SDLK_TAB:
                        game->inventory_open = !game->inventory_open;
                        break;
                    case SDLK_F1:
                        show_debug_info = !show_debug_info;
                        max_frame_msec = 0.0f;
                        break;
                    case SDLK_F2:
                        show_debug_map = !show_debug_map;
                        break;
                    case SDLK_F3:
                        show_distances = !show_distances;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

//    PrintTilesAreDarkThatShouldntBe(&game->map, "before UpdateGame");
    UpdateGame(game, dt);
//    PrintTilesAreDarkThatShouldntBe(&game->map, "after UpdateGame");

    if ( game->area == AREA_FOREST ) {
        V_ClearRGB(0, 0, 64);
    } else {
        V_ClearRGB(0, 0, 0);
    }

    game->state->render(game);
    V_Refresh();

    game->ticks++;
}
