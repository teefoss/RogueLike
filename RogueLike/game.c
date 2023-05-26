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
#include "menu.h"
#include "config.h"
#include "game_log.h"

#include "mathlib.h"
#include "sound.h"
#include "video.h"
#include "texture.h"

#include <stdio.h>


static vec2_t GetWindowScale(const RenderInfo * info)
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    vec2_t scale = { w / (float)info->width, h / (float)info->height };

    return scale;
}


/// In the visible rect, set each tile's light level according to its visibility
/// flags.
void SetTileLight(World * world, const RenderInfo * render_info)
{
    const AreaInfo * info = world->info;
    Box vis = GetCameraVisibleRegion(world->map, render_info);

    for ( int y = vis.top; y <= vis.bottom; y++ ) {
        for ( int x = vis.left; x <= vis.right; x++ ) {
            TileCoord coord = { x, y };
            Tile * tile = GetTile(world->map, coord);

            if ( tile == NULL ) {
                Error("error: somehow NULL tile in vis rect!");
            }

            if ( tile->flags.bright ) {
                tile->light = 255;
            } else if ( info->reveal_all ) {
                tile->light = info->visible_light;
                tile->flags.visible = true;
            } else {
                if ( tile->flags.visible ) {
                    tile->light = info->visible_light;
                } else if ( tile->flags.revealed ) {
                    tile->light = info->revealed_light;
                } else {
                    tile->light = info->unrevealed_light;
                }
            }
        }
    }
}


void LoadLevel(Game * game, int level_num, bool persist_player_stats)
{
    World * world = &game->world;

    ActorsStats saved_player_stats = { 0 };
    if ( persist_player_stats ) {
        Actor * player = FindActor(&world->map->actor_list, ACTOR_PLAYER);
        if ( player ) {
            saved_player_stats = player->stats;
        }
    }

    int seed = (int)time(NULL);

    if ( level_num == ENTER_SUBLEVEL ) {
        game->world.map++;
        if ( game->world.area == AREA_FOREST ) { // TODO: refactor
            game->world.area = AREA_FOREST_SHACK;
            game->world.info = &area_info[game->world.area];
        }
    } else if ( level_num == EXIT_SUBLEVEL ) {
        game->world.map--;
        if ( game->world.area == AREA_FOREST_SHACK ) {
            game->world.area = AREA_FOREST;
            game->world.info = &area_info[game->world.area];
        }
    } else {
        game->level = level_num;

        if ( level_num == 1 ) {
            GenerateWorld(game,
                          AREA_FOREST,
                          (int)time(NULL),
                          game->forest_size,
                          game->forest_size);
        } else {
            GenerateWorld(game, AREA_DUNGEON, seed, 31, 31);
        }

        // Some things to reset when entering a new level:
        game->player_info.turns = INITIAL_TURNS; // TODO: maybe not?
        game->player_info.has_gold_key = false;
    }

    printf("num actors: %d\n", game->world.map->actor_list.count);

    // Focus camera on player.
    Actor * player = FindActor(&world->map->actor_list, ACTOR_PLAYER);
    game->render_info.camera = TileCoordToScaledWorldCoord(player->tile, vec2_zero);

    // Carry over player's stats from the previous level.
    if ( persist_player_stats ) {
        player->stats = saved_player_stats;
    }

    // Initial lighting.
    PlayerCastSight(world, &game->render_info);
    SetTileLight(&game->world, &game->render_info);

    FOR_EACH_ACTOR(actor, world->map->actor_list) {
        CastLight(world, actor);
    }
}


void StartFadeIn(FadeState * fade_state, float seconds)
{
    fade_state->type = FADE_IN;
    fade_state->timer = 0.0f;
    fade_state->duration_sec = seconds;
}


void NewGame(Game * game)
{
    LoadLevel(game, 1, false);
    game->player_info.inventory.item_counts[0] = 3;
    game->player_info.inventory.item_counts[1] = 3;

    game->player_info.fuel = 4;
    game->player_info.fuel_steps = FUEL_STEPS;

    ChangeStateAndFadeIn(game, &gs_level_idle, 2.0f);
}

// TODO: move to inventory.c
int InventoryRenderX(const RenderInfo * info)
{
    return info->width - InventoryWidth();
}


/// Do lighting, particles n stuff for level idle and turn.
void UpdateLevel(Game * game, float dt)
{
    // Update camera.

    Actor * player = FindActor(&game->world.map->actor_list, ACTOR_PLAYER);
    vec2_t player_pt; // world scaled
    player_pt.x = player->tile.x * SCALED(TILE_SIZE) + player->offset_current.x;
    player_pt.y = player->tile.y * SCALED(TILE_SIZE) + player->offset_current.y;
    game->render_info.camera = Vec2LerpEpsilon(game->render_info.camera,
                                               player_pt,
                                               0.2f,
                                               1.0f);

    int num_visible_actors = 0;
    Actor ** visible_actors = GetVisibleActors(&game->world, &game->render_info, &num_visible_actors);

    // Update Actors: run timers.
    for ( int i = 0; i < num_visible_actors; i++ ) {
        Actor * actor = visible_actors[i];
        if ( actor->hit_timer > 0.0f ) {
            actor->hit_timer -= 5.0f * dt;
        }
    }

    // Update lighting based on new camera position.

    SetTileLight(&game->world, &game->render_info);


    for ( int i = 0; i < num_visible_actors; i++ ) {
        Actor * actor = visible_actors[i];

        if (    actor->type != ACTOR_PLAYER
            || (actor->type == ACTOR_PLAYER && game->player_info.fuel) )
        {
            CastLight(&game->world, actor);
        }
    }

    // Update inventory panel position
    {
        float target;
        if ( game->inventory_open ) {
            target = InventoryRenderX(&game->render_info);
        } else {
            target = game->render_info.width; // Closed
        }

        game->render_info.inventory_x = LerpEpsilon(game->render_info.inventory_x, target, 0.33f, 1.0f);
    }

    UpdateParticles(&game->world.particles, dt);

    // Update mouse tile
    {
        vec2_t window_scale = GetWindowScale(&game->render_info);
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        // Adjust for window scale
        vec2_t mouse;
        mouse.x = (float)mx / window_scale.x;
        mouse.y = (float)my / window_scale.y;

        // Convert to tile and apply draw offset
        if ( show_debug_map ) {
            int size = game->world.info->debug_map_tile_size;
            // (No draw offset)
            mouse.x /= size;
            mouse.y /= size;
        } else {
            vec2_t render_offset = GetRenderOffset(&game->render_info);
            mouse.x += render_offset.x;
            mouse.y += render_offset.y;
            mouse.x /= SCALED(TILE_SIZE);
            mouse.y /= SCALED(TILE_SIZE);
        }

        game->world.mouse_tile.x = mouse.x;
        game->world.mouse_tile.y = mouse.y;
    }
}


void TryMovePlayer(Actor * player,
                   Map * map,
                   TileCoord coord,
                   PlayerInfo * player_info)
{
    if ( TryMoveActor(player, coord) ) {

        // Only set 'on_teleporer' to false once we've definitely moved off it.
//        Tile * player_tile = GetTile(map, player->tile);
//        if ( player_tile->type != TILE_TELEPORTER ) {
//            player->flags.on_teleporter = false;
//        }

        CalculateDistances(map, player->tile, 0, true);
    }

    --player_info->turns;

    // Burn torch fuel.
    if ( player_info->fuel && --player_info->fuel_steps == 0 ) {
        player_info->fuel_steps = FUEL_STEPS; // Reset step counter.
        --player_info->fuel;
    }
}


void StartTurn(Game * game, TileCoord destination, Direction direction)
{
    printf("--- START TURN ---\n");

    float start_time = ProgramTime();

    World * world = &game->world;
    Actor * player = FindActor(&world->map->actor_list, ACTOR_PLAYER);
    PlayerInfo * player_info = &game->player_info;

    ResetLog();

    // Do player-tile collisions:

    // The tile we are moving to.
    Tile * tile;
    if ( direction != NO_DIRECTION ) {
        tile = GetAdjacentTile(world->map, player->tile, direction);
        destination = AdjacentTileCoord(player->tile, direction);
    } else {
        tile = GetTile(world->map, destination);
    }

    switch ( (TileType)tile->type ) {

        case TILE_TREE:
        case TILE_DUNGEON_WALL:
        case TILE_NULL:
            SetUpBumpAnimation(player, direction);
            S_Play(SOUND_BUMP);

            --player_info->turns; // TODO: maybe don't
            break;

        case TILE_BUTTON_NOT_PRESSED: {
            Actor * pillars[2] = { NULL };
            FindActors(&world->map->actor_list, ACTOR_PILLAR, pillars);

            // Lower pillars.
            for ( int i = 0; i < 2; i++ ) {
                Tile * pillar_tile = GetTile(world->map, pillars[i]->tile);
                *pillar_tile = CreateTile(TILE_BUTTON_PRESSED);
                RemoveActor(pillars[i]);
            }

            // Press the button.
            S_Play("l32 o1 b- c");
            *tile = CreateTile(TILE_BUTTON_PRESSED);

            TryMovePlayer(player, world->map, destination, player_info);
            break;
        }

        case TILE_DUNGEON_DOOR:
            SetUpBumpAnimation(player, direction);
            S_Play("l32o2c+f+b");
            *tile = CreateTile(TILE_DUNGEON_FLOOR); // Open (remove) the door.

            // Make sure to reveal what's behind the door.
            PlayerCastSight(world, &game->render_info);
            break;

        case TILE_GOLD_DOOR:
            SetUpBumpAnimation(player, direction);
            if ( player_info->has_gold_key ) {
                S_Play("l32o2 c+g+dae-b-");
                *tile = CreateTile(TILE_DUNGEON_FLOOR);
            } else {
                S_Play("l32o2 gc+");
                Log("You need the Gold Key!");
            }

            PlayerCastSight(world, &game->render_info);
            break;

        case TILE_FOREST_EXIT:
            if ( !player_info->has_rope ) {
                SetUpBumpAnimation(player, direction);
                Log("You need a rope to descend!");
                break;
            }
            // Fallthrough:
        case TILE_DUNGEON_EXIT:
            MoveActor(player, destination);
            S_Play("l32o3bb-a-fd-<a-d<g");
            break;
        case TILE_WHITE_OPENING:
            game->player_info.level_state = LEVEL_EXIT_SUB;
            MoveActor(player, destination);
            break;

        case TILE_TELEPORTER:
            player->flags.on_teleporter = true;
            // Fallthough:
        default:
            TryMovePlayer(player, world->map, destination, player_info);
            break;
    }

    ChangeState(game, &gs_level_turn);

    // Update all actors when player is out of turns.
    if ( player_info->turns < 0 ) {
        player_info->turns = INITIAL_TURNS;

        // Do all actor turns.
        FOR_EACH_ACTOR(actor, world->map->actor_list) {
            if ( !actor->flags.was_attacked && actor->info->action ) {
                actor->info->action(actor);
            }

            actor->flags.was_attacked = false; // reset
        }
    }

    printf("%s: %.2f ms\n", __func__, (ProgramTime() - start_time) * 1000.0f);
}


#pragma mark - RENDER


// TODO: param player stats only? This all needs a massive clean up
void RenderHUD(const Game * game, const Actor * player)
{
    const int margin = HUD_MARGIN;
//    const int char_w = V_CharWidth();
    const int char_h = V_CharHeight();
    V_SetGray(255);

    //
    // Top HUD
    //

    // Level

    V_PrintString(margin, margin, "Level %d", game->level);
    if ( game->player_info.has_gold_key ) {
        int x = margin;
        int y = margin * 2 + SCALED(1);
        RenderIcon(ICON_GOLD_KEY, x, y, &game->render_info);
    }

    if ( game->player_info.has_shack_key ) {
        int x = margin + SCALED(ICON_SIZE) + 2;
        int y = margin * 2 + SCALED(1);
        RenderIcon(ICON_OLD_KEY, x, y, &game->render_info);
    }

    // Log
    RenderLog(game, &game->render_info);


    //
    // Lower HUD
    //

    int hud_x = margin;
    int hud_y = game->render_info.height - (V_CharHeight() + margin);

    // Turns

    int turns_x = V_PrintString(hud_x, hud_y, " Turns ");

    for ( int i = 0; i < game->player_info.turns; i++ ) {
        RenderIcon(ICON_TURN, turns_x + i * SCALED(ICON_SIZE), hud_y, &game->render_info);
    }

    // Attack

    hud_y -= char_h;
    int attack_x = V_PrintString(hud_x, hud_y, "Attack ");

    int total_damage = player->stats.damage + game->player_info.strength_buff;
    for ( int i = 0; i < total_damage; i++  ) {
        RenderIcon(ICON_DAMAGE, attack_x + i * SCALED(ICON_SIZE), hud_y, &game->render_info);
    }

    // Health

    hud_y -= char_h;

    int health_x = V_PrintString(hud_x, hud_y, "Health ");

    for ( int i = 0; i < player->info->max_health; i++ ) {
        Icon icon = i + 1 > player->stats.health ? ICON_HEART_EMPTY : ICON_HEART_FULL;
        RenderIcon(icon, health_x + i * SCALED(ICON_SIZE), hud_y, &game->render_info);
    }

    // Fuel

    hud_y -= char_h;

    int fuel_x = V_PrintString(hud_x, hud_y, "  Lamp ");

    const PlayerInfo * info = &game->player_info;

    for ( int i = 0; i < MAX_FUEL; i++ ) {

        Icon icon;

        if ( i + 1 == info->fuel ) {
            // The rightmost icon:
            if ( GetGameState(game) == &gs_level_turn ) {
                icon = ICON_FUEL_BURN;
            } else {
                if ( info->fuel_steps == 1 ) {
                    icon = ICON_FUEL_DYING;
                } else {
                    icon = ICON_FUEL_FULL;
                }
            }
        } else if ( i + 1 < info->fuel ) {
            icon = ICON_FUEL_FULL;
        } else {
            icon = ICON_FUEL_EMPTY;
        }

        RenderIcon(icon, fuel_x + i * SCALED(ICON_SIZE), hud_y, &game->render_info);
    }
}


#define BOOL_STR(bool_var) bool_var ? "yes" : "no"

// TODO: param player stats only?
void RenderDebugInfo(const World * world,
                     const Actor * player,
                     TileCoord mouse_tile)
{
    const Map * map = world->map;

    DEBUG_PRINT("Frame time: %.1f (max: %.1f)",
                frame_msec * 1000.0f,
                max_frame_msec * 1000.0f);
    DEBUG_PRINT("- Update time: %.1f", update_msec * 1000.0f);
    DEBUG_PRINT("- Render time: %.1f", render_msec * 1000.0f);
    DEBUG_PRINT("- - Tiles: %.1f", tiles_msec * 1000.0f);
    DEBUG_PRINT("- - Actors: %.1f", actors_msec * 1000.0f);
    DEBUG_PRINT(" ");
    DEBUG_PRINT("Player health: %d", player->stats.health);
//    DEBUG_PRINT("Actors %d", world->actors.count);

    Tile * hover = GetTile((Map *)map, mouse_tile);
    if ( hover ) {
        DEBUG_PRINT("Mouse tile: %d, %d (%s)",
                    mouse_tile.x,
                    mouse_tile.y,
                    TileName(hover->type));
        DEBUG_PRINT(" light: %d", hover->light);
        DEBUG_PRINT(" revealed: %s", BOOL_STR(hover->flags.revealed));
        DEBUG_PRINT(" visible: %s", BOOL_STR(hover->flags.visible));
        DEBUG_PRINT(" blocking: %s", BOOL_STR(hover->flags.blocks_movement));

        bool los = LineOfSight((Map *)map, player->tile, mouse_tile);
        DEBUG_PRINT(" LOS: %s", los ? "yes" : "no");
    } else {

    }
}


void GamePlayRender(const Game * game)
{
    const World * world = &game->world;

    if ( show_debug_map ) {
//        int size = area_info[world->area].debug_map_tile_size;
        int size = game->render_info.height / world->map->height;

        RenderTiles(world, NULL, vec2_zero, true, &game->render_info);

        Box vis = GetCameraVisibleRegion(world->map, &game->render_info);

        SDL_Rect box = {
            .x = vis.left * size,
            .y = vis.top * size,
            .w = ((vis.right - vis.left) + 1) * size,
            .h = ((vis.bottom - vis.top) + 1) * size
        };

        V_SetRGB(255, 80, 80);
        V_DrawRect(&box);

        FOR_EACH_ACTOR_CONST(actor, world->map->actor_list) {
            int x = actor->tile.x * size;
            int y = actor->tile.y * size;
            RenderActor(actor, x, y, size, true, game->ticks);
        }
    } else {
        RenderWorld(world, &game->render_info, game->ticks);

        if ( menu_state == MENU_NONE ) {
            if ( !show_debug_info ) {
                const Actor * player = FindActorConst(&world->map->actor_list,
                                                      ACTOR_PLAYER);
                RenderHUD(game, player);
            }
        }

        if ( game->render_info.inventory_x != game->render_info.width ) {
            RenderInventory(&game->player_info.inventory, &game->render_info);
        }
    }

    if ( show_debug_info ) {
        const Actor * player = FindActorConst(&world->map->actor_list, ACTOR_PLAYER);
        RenderDebugInfo(&game->world, player, world->mouse_tile);
    }

    // Darken world a bit so the menu is clear.
    if ( menu_state != MENU_NONE ) {
        V_SetRGBA(0, 0, 0, 160);
        V_FillRect(NULL);
    }
}


/// Render a transparent rectangle over everything if fading in/out.
void RenderFade(FadeState * fade_state)
{
    u8 alpha = 0;

    if ( fade_state->type == FADE_IN ) {
        alpha = (1.0f - fade_state->timer) * 255.0f;
    } else if ( fade_state->type == FADE_OUT ) {
        alpha = fade_state->timer * 255.0f;
    }

    V_SetRGBA(0, 0, 0, alpha);
    V_FillRect(NULL);
}


#pragma mark -


Game * InitGame(int width, int height)
{
    Game * game = calloc(1, sizeof(*game));
    if ( game == NULL ) {
        Error("Could not allocate game");
    }

    game->is_running = true;
    game->ticks = 0;
    game->inventory_open = false;

    game->world = InitWorld();
    game->world.map = &game->world.maps[0];
    
    game->state_stack_top = -1;
    game->render_info = InitRenderInfo(width, height);

    // TODO: move to debug.c
    game->forest_size = 256;
    game->forest_seed = 0;
    game->forest_freq = 0.06f;
    game->forest_amp = 1.0f;
    game->forest_pers = 0.6f;
    game->forest_lec = 2.0f; // lac
    game->forest_low = -0.35f;
    game->forest_high = 0.05;

    // Make sure there's a state on the stack to begin.
    PushState(game, &gs_title_screen);
    ChangeState(game, &gs_title_screen);

    return game;
}


// TODO: profile function macro -> ms stored in debug.c global and displayed in debug screen
void DoFrame(Game * game, float dt)
{
    debug_row = 0;
//    SDL_Keymod mods = SDL_GetModState();
//    bool shift = mods & KMOD_SHIFT;

    SDL_Event event;
    while ( SDL_PollEvent(&event) ) {

        // Let the current game state process this event first, if not doing
        // a fade in/out transition.
        if (   game->fade_state.type == FADE_NONE
            || game->fade_state.type == FADE_IN )
        {
            const GameState * state = game->state_stack[game->state_stack_top];

            if ( state->process_event && state->process_event(game, &event) ) {
                continue;
            }
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
                        cfg_fullscreen ^= 1;
                        break;
                    case SDLK_ESCAPE:
                        MenuToggle(game);
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
                    case SDLK_LEFTBRACKET:
                        LoadLevel(game, game->level - 1, false);
                        break;
                    case SDLK_RIGHTBRACKET:
                        LoadLevel(game, game->level + 1, false);
                        break;
#if 0
                    case SDLK_MINUS:
                        game->forest_size -= 8;
                        LoadLevel(game, game->level, false);
                        break;
                    case SDLK_EQUALS:
                        game->forest_size += 8;
                        LoadLevel(game, game->level, false);
                        break;
#endif
                    case SDLK_COMMA:
                        --game->forest_seed;
                        LoadLevel(game, game->level, false);
                        break;
                    case SDLK_PERIOD:
                        ++game->forest_seed;
                        LoadLevel(game, game->level, false);
                        break;
#if 0
                    case SDLK_u:
                        game->forest_freq += shift ? -0.01f : 0.01f;
                        LoadLevel(game, game->level, false);
                        break;
                    case SDLK_i:
                        game->forest_amp += shift ? -0.1f : 0.1f;
                        LoadLevel(game, game->level, false);
                        break;
                    case SDLK_o:
                        game->forest_pers += shift ? -0.1f : 0.1f;
                        LoadLevel(game, game->level, false);
                        break;
                    case SDLK_p:
                        game->forest_lec += shift ? -0.1f : 0.1f;
                        LoadLevel(game, game->level, false);
                        break;
#endif 
                    case SDLK_k:
                        FOR_EACH_ACTOR(actor, game->world.map->actor_list) {
                            if ( actor->type != ACTOR_PLAYER ) {
                                RemoveActor(actor);
                            }
                        }
                        break;
                    default:
                        break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                switch ( event.button.button ) {
                    case SDL_BUTTON_LEFT:
                        if ( event.button.clicks == 2 && show_debug_info ) {
                            Actor * player = FindActor(&game->world.map->actor_list,
                                                       ACTOR_PLAYER);
                            player->tile = game->world.mouse_tile;
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    PROFILE(UpdateState(game, dt), update_msec);

    float render_start = ProgramTime();

    V_ClearRGB(0, 0, 0);

    for ( int i = 0; i <= game->state_stack_top; i++ ) {
        const GameState * s = game->state_stack[i];
        if ( s->render ) {
            s->render(game);
        }
    }

    if ( game->fade_state.type != FADE_NONE ) {
        RenderFade(&game->fade_state);
    }

    V_Refresh();

    render_msec = ProgramTime() - render_start;

    game->ticks++;
}
