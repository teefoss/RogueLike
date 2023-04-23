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
#include "menu.h"

#include "mathlib.h"
#include "sound.h"
#include "video.h"
#include "texture.h"

#include <stdio.h>


/// In the visible rect, set each tile's light level according to its visibility
/// flags.
void SetTileLight(World * world, const RenderInfo * render_info)
{
    const AreaInfo * info = &area_info[world->area];
    Box vis = GetVisibleRegion(&world->map, render_info);

    for ( int y = vis.top; y <= vis.bottom; y++ ) {
        for ( int x = vis.left; x <= vis.right; x++ ) {
            TileCoord coord = { x, y };
            Tile * tile = GetTile(&world->map, coord);

            if ( tile == NULL ) {
                Error("error: somehow NULL tile in vis rect!");
            }

            if ( tile->flags.bright ) {
                tile->light = 255;
            } else if ( info->reveal_all ) {
                tile->light = info->visible_light;
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


void LoadLevel(Game * game, int level_num)
{
    World * world = &game->world;

    game->level = level_num;

    switch ( level_num ) {
        case 1:
            GenerateForest(game, (u32)time(NULL));
            break;
        default:
            // TODO: The Well
            GenerateDungeon(game, 31, 31);
            break;
    }

    // Focus camera on player.
    Actor * player = GetPlayer(&game->world.actors);
    game->render_info.camera = TileCoordToScaledWorldCoord(player->tile, vec2_zero);

    // Initial lighting.
    PlayerCastSight(world, &game->render_info);
    SetTileLight(&game->world, &game->render_info);

    for ( int i = 0; i < world->actors.count; i++ ) {
        CastLight(world, &world->actors.list[i]);
    }

    game->player_info.player_turns = INITIAL_TURNS;
    game->player_info.has_gold_key = false;
}


void NewGame(Game * game)
{
    LoadLevel(game, 1);
    ChangeState(game, &level_idle);
}


int InventoryRenderX(Inventory * inventory)
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


/// Do lighting, particles n stuff for level idle and turn.
void UpdateLevel(Game * game, float dt)
{
    Actor * player = GetPlayer(&game->world.actors);
    Actors * actors = &game->world.actors;
    Map * map = &game->world.map;

    // TODO: make int remove_indices[] and int num_to_remove
    // If num_to_remove > 0, sort array decreasing
    for ( int i = actors->count - 1; i >= 0; i-- ) {
        Actor * actor = &actors->list[i];

        if ( actor->flags.remove ) {
            actors->list[i] = actors->list[--actors->count];
        }
    }

    // Update Actors: run timers.
    for ( int i = 0; i < actors->count; i++ ) {
        Actor * actor = &actors->list[i];

        if ( actor->hit_timer > 0.0f ) {
            actor->hit_timer -= 5.0f * dt;
        }
    }

    // Update camera.

    vec2_t player_pt; // world scaled
    player_pt.x = player->tile.x * SCALED(TILE_SIZE) + player->offset_current.x;
    player_pt.y = player->tile.y * SCALED(TILE_SIZE) + player->offset_current.y;
    game->render_info.camera = Vec2LerpEpsilon(game->render_info.camera,
                                               player_pt,
                                               0.2f,
                                               1.0f);

    // Update lighting based on new camera position.
    Box vis = GetVisibleRegion(map, &game->render_info);
    SetTileLight(&game->world, &game->render_info);

    for ( int i = 0; i < game->world.actors.count; i++ ) {
        Actor * actor = &game->world.actors.list[i];

        if (   actor->tile.x >= vis.left
            && actor->tile.x <= vis.right
            && actor->tile.y >= vis.top
            && actor->tile.y <= vis.bottom )
        {
            CastLight(&game->world, actor);
        }
    }

    // Update inventory panel position
    {
        float target;
        if ( game->inventory_open ) {
            target = InventoryRenderX(&game->player_info.inventory);
        } else {
            target = GAME_WIDTH; // Closed
        }

        game->render_info.inventory_x = LerpEpsilon(game->render_info.inventory_x, target, 0.33f, 1.0f);
    }

    UpdateParticles(&game->world.particles, dt);

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
            mouse.x /= area_info[game->world.area].debug_map_tile_size;
            mouse.y /= area_info[game->world.area].debug_map_tile_size;
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


#pragma mark - Intermission State


bool DoIntermissionInput(Game * game, const SDL_Event * event)
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


void IntermissionOnExit(Game * game)
{
    GenerateDungeon(game, 31, 31);
    game->player_info.player_turns = INITIAL_TURNS;
    PlayerCastSight(&game->world, &game->render_info);
}



#pragma mark - LEVEL IDLE STATE


void LevelIdleOnEnter(Game * game)
{
    Actor * player = GetPlayer(&game->world.actors);
    Tile * player_tile = GetTile(&game->world.map, player->tile);

    // Check if the player has moved onto a tile that requires action:

    switch ( (TileType)player_tile->type ) {
        case TILE_TELEPORTER:
            Teleport(player, player->tile);
            S_Play("o0 t160 l32 c g > d a > e b > f+ > c+ g+ > d+ a+ > f > c ");
            player->flags.on_teleporter = false;
            break;
        case TILE_EXIT:
            ChangeState(game, &intermission);
            LoadLevel(game, game->level + 1);
            break;
        default:
            break;
    }
}


void MovePlayer(Game * game, Direction direction)
{
    World * world = &game->world;
    Map * map = &world->map;
    Actor * player = GetPlayer(&world->actors);
    PlayerInfo * player_info = &game->player_info;

    game->log[0] = '\0'; // Clear the log.

    // Do player-tile collisions:

    // The tile we are moving to.
    Tile * tile = GetAdjacentTile(map, player->tile, direction);

    switch ( (TileType)tile->type ) {

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
            if ( player_info->has_gold_key ) {
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
                CalculateDistances(map, player->tile, 0);
            }
            player_info->player_turns--;
            break;

        case TILE_EXIT:
            MoveActor(player, direction);
            S_Play("l32o3bb-a-fd-<a-d<g");
            break;
        default:
            break;
    }

    UpdateActorFacing(player, XDelta(direction));
    PlayerCastSight(world, &game->render_info);

    ChangeState(game, &level_turn);

    // Update all actors when player is out of turns.
    if ( player_info->player_turns < 0 ) {
        player_info->player_turns = INITIAL_TURNS;

        // Do all actor turns.
        for ( int i = 0; i < world->actors.count; i++ ) {
            Actor * actor = &world->actors.list[i];

            if ( !actor->flags.remove ) {
                if ( !actor->flags.was_attacked && actor->action ) {
                    actor->action(actor);
                }

                actor->flags.was_attacked = false; // reset
            }
        }
    }
}

bool InventoryIsEmtpy(const Inventory * inventory)
{
    for ( int i = 0; i < NUM_ITEMS; i++ ) {
        if ( inventory->item_counts[i] > 0 ) {
            return false;
        }
    }

    return true;
}

void ChangeInventorySelection(Inventory * inventory, int direction)
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

void UseItem(Actor * player, PlayerInfo * info)
{
    Inventory * inventory = &info->inventory;

    // If the inventory is empty, just leave.
    if ( inventory->item_counts[inventory->selected_item] == 0 ) {
        return;
    }

    // Remove from inventory.
    --inventory->item_counts[inventory->selected_item];

    switch ( inventory->selected_item ) {
        case ITEM_HEALTH:
            if ( player->health < player->max_health ) {
                player->health++;
            }
            S_Play("l32 o3 d+ < g+ b e");
            break;
        case ITEM_TURN:
            info->player_turns++;
            S_Play("l32 o3 f+ < b a");
            break;
        default:
            break;
    }

    // Change selection if used the last of this item.
    if ( inventory->item_counts[inventory->selected_item] == 0 ) {
        ChangeInventorySelection(inventory, 1);
    }
}


bool LevelIdleProcessInput(Game * game, const SDL_Event * event)
{
    const u8 * keys = SDL_GetKeyboardState(NULL);

    switch ( event->type ) {
        case SDL_KEYDOWN:

            if ( !keys[SDL_SCANCODE_LSHIFT] && !keys[SDL_SCANCODE_RSHIFT] ) {
                if ( event->key.repeat > 0 ) {
                    return false;
                }
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
                        ChangeInventorySelection(&game->player_info.inventory, +1);
                    }
                    return true;
                case SDLK_DOWN:
                    if ( game->inventory_open ) {
                        ChangeInventorySelection(&game->player_info.inventory, -1);
                    }
                    return true;
                case SDLK_RETURN:
                    if ( game->inventory_open ) {
                        Actor * player = GetPlayer(&game->world.actors);
                        UseItem(player, &game->player_info);
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

void LevelIdleUpdate(Game * game, float dt)
{
    // Update actor standing animations, etc.
    Actors * actors = &game->world.actors;
    for ( int i = 0; i < actors->count; i++ ) {
        Actor * actor = &actors->list[i];

        if (actor->sprite->num_frames > 1 &&
            game->ticks % MS2TICKS(actor->sprite->frame_msec, FPS) == 0 )
        {
            actor->frame = (actor->frame + 1) % actor->sprite->num_frames;
        }
    }

    UpdateLevel(game, dt);
}



#pragma mark - LEVEL TURN STATE

void LevelTurnOnEnter(Game * game)
{
    game->move_timer = 0.0f;
}

/// Run the move timer and do actor movement animations.
void LevelTurnUpdate(Game * game, float dt)
{
    game->move_timer += 5.0f * dt;

    if ( game->move_timer >= 1.0f ) {
        // We're done.
        game->move_timer = 1.0f;
        ChangeState(game, &level_idle);
        // Don't return here, make sure actors complete their animation.
    }

    Actors * actors = &game->world.actors;
    for ( int i = 0; i < actors->count; i++ ) {
        Actor * actor = &actors->list[i];

        if ( actor->animation ) {
            actor->animation(actor, game->move_timer);
            if ( game->move_timer == 1.0f ) {
                actor->animation = NULL; // Remove the animation.
            }
        }
    }

    UpdateLevel(game, dt);
}

#pragma mark - RENDER


vec2_t GetWindowScale(void)
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    vec2_t scale = { w / (float)GAME_WIDTH, h / (float)GAME_HEIGHT };

    return scale;
}


void RenderHUD(const Game * game, const Actor * player)
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
    if ( game->player_info.has_gold_key ) {
        RenderIcon(ICON_GOLD_KEY, margin, margin * 2 + SCALED(1));
    }

    // Log

    int log_len = (int)strlen(game->log);
    if ( log_len ) {
        int log_x = game->render_info.inventory_x - (log_len * char_w + margin);

        if ( GetGameState(game) == &level_turn ) {
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

    for ( int i = 0; i < game->player_info.player_turns; i++ ) {
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
        Icon icon = i + 1 > player->health ? ICON_EMPTY_HEART : ICON_FULL_HEART;
        RenderIcon(icon, health_x + i * SCALED(ICON_SIZE), hud_y);
    }
}


// TODO: params render_info, inventory only
void RenderInventory(const Game * game)
{
    SDL_Rect inventory_panel = {
        .x = game->render_info.inventory_x,
        .y = 0,
        .w = GAME_WIDTH - game->render_info.inventory_x,
        .h = GAME_HEIGHT
    };

    V_SetGray(16);
    V_FillRect(&inventory_panel);
    V_SetGray(32);
    V_DrawVLine(game->render_info.inventory_x, 0, GAME_HEIGHT);

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

    const Inventory * in = &game->player_info.inventory;
    for ( int i = 0; i < NUM_ITEMS; i++ ) {
        if ( in->item_counts[i] ) {
            bool selected = in->selected_item == i;
            RenderItemInfo(i, in->item_counts[i], 0, row++ * char_h, selected);
        }
    }

    SDL_RenderSetViewport(renderer, NULL);
}


#define BOOL_STR(bool_var) bool_var ? "yes" : "no"

void RenderDebugInfo(const Map * map, const Actor * player, TileCoord mouse_tile)
{
    DEBUG_PRINT("Frame time: %.1f (max: %.1f)",
                frame_msec * 1000.0f,
                max_frame_msec * 1000.0f);
    DEBUG_PRINT(" ");
    DEBUG_PRINT("Player health: %d", player->health);
    DEBUG_PRINT(" ");

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
        int size = area_info[world->area].debug_map_tile_size;

        RenderTiles(world, NULL, vec2_zero, true);

        Box vis = GetVisibleRegion(&world->map, &game->render_info);

        SDL_Rect box = {
            .x = vis.left * size,
            .y = vis.top * size,
            .w = ((vis.right - vis.left) + 1) * size,
            .h = ((vis.bottom - vis.top) + 1) * size
        };

        V_SetRGB(255, 80, 80);
        V_DrawRect(&box);

        const Actors * actors = &game->world.actors;
        for ( int i = 0; i < actors->count; i++ ) {
            const Actor * actor = &actors->list[i];
            int x = actor->tile.x * size;
            int y = actor->tile.y * size;
            RenderActor(actor, x, y, size, true, game->ticks);
        }
    } else {
        RenderWorld(world, &game->render_info, game->ticks);

        if ( menu_state == MENU_NONE ) {
            if ( !show_debug_info ) {
                RenderHUD(game, GetPlayer(&world->actors));
            }
        }

        if ( game->render_info.inventory_x != GAME_WIDTH ) {
            RenderInventory(game);
        }
    }

    if ( show_debug_info ) {
        RenderDebugInfo(&game->world.map, GetPlayer(&world->actors), world->mouse_tile);
    }

    // Darken world a bit so the menu is clear.
    if ( menu_state != MENU_NONE ) {
        V_SetRGBA(0, 0, 0, 160);
        V_FillRect(NULL);
    }
}


void TitleScreenRender(const Game * game)
{
    RenderWorld(&game->world, &game->render_info, game->ticks);
}


void IntermissionRender(const Game * game)
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


Game * InitGame(void)
{
    Game * game = calloc(1, sizeof(*game));
    if ( game == NULL ) {
        Error("Could not allocate game");
    }

    game->is_running = true;
    game->ticks = 0;
    game->inventory_open = false;
    game->render_info.inventory_x = GAME_WIDTH; // Start closed.
    game->world = InitWorld();
    game->state_stack_top = -1;

    // Generate a forest as the title screen background.
//    u32 seed = (u32)time(NULL);
    u32 seed = 1682214124;
    printf("title screen seed: %d\n", seed);
    GenerateForest(game, seed);

    // Remove all non-tree actors.
    Actors * actors = &game->world.actors;
    for ( int i = actors->count - 1; i >= 0; i-- ) {
        if ( actors->list[i].type != ACTOR_TREE ) {
            actors->list[i] = actors->list[--actors->count];
        }
    }

    // Center camera in world.
    TileCoord center = { game->world.map.width / 2, game->world.map.height / 2 };
    game->render_info.camera = TileCoordToScaledWorldCoord(center, vec2_zero);

    PushState(game, &game_state_title);
    PushState(game, &game_state_menu);
    menu_state = MENU_MAIN;

    return game;
}


// TODO: profile function macro -> ms stored in debug.c global and displayed in debug screen
void DoFrame(Game * game, float dt)
{
    debug_row = 0;

    SDL_Event event;
    while ( SDL_PollEvent(&event) ) {

        // Let the current game state process this event first.
        const GameState * state = game->state_stack[game->state_stack_top];
        if (   state->process_event
            && state->process_event(game, &event) )
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
                    default:
                        break;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                switch ( event.button.button ) {
                    case SDL_BUTTON_LEFT:
                        if ( event.button.clicks == 2 && show_debug_info ) {
                            Actor * player = GetPlayer(&game->world.actors);
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

    UpdateState(game, dt);

    // TODO: move to area_info
    if ( game->world.area == AREA_FOREST ) {
        V_ClearRGB(0, 0, 64);
    } else {
        V_ClearRGB(0, 0, 0);
    }

    for ( int i = 0; i <= game->state_stack_top; i++ ) {
        const GameState * s = game->state_stack[i];
        if ( s->render ) {
            s->render(game);
        }
    }

    V_Refresh();

    game->ticks++;
}
