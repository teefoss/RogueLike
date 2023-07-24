//
//  gs_level_idle.c
//  RogueLike
//
//  Created by Thomas Foster on 5/11/23.
//

#include "game.h"
#include "sound.h"
#include "game_log.h"

static bool InventoryProcessEvent(Game * game, const SDL_Event * event)
{
    Inventory * inv = &game->player_info.inventory;

    switch ( event->type ) {
        case SDL_KEYDOWN:
            switch ( event->key.keysym.sym ) {
                case SDLK_w:
                case SDLK_UP:
                    ChangeInventorySelection(inv, NORTH);
                    return true;
                case SDLK_s:
                case SDLK_DOWN:
                    ChangeInventorySelection(inv, SOUTH);
                    return true;
                case SDLK_a:
                case SDLK_LEFT:
                    ChangeInventorySelection(inv, WEST);
                    return true;
                case SDLK_d:
                case SDLK_RIGHT:
                    ChangeInventorySelection(inv, EAST);
                    return true;
                case SDLK_RETURN: {
                    Actor * player = FindActor(&game->world.map->actor_list,
                                               ACTOR_PLAYER);
                    UseItem(player);
                    return true;
                }
                default:
                    return false;
            }
            break;
        default:
            return false;
    }
}


static bool LevelProcessEvent(Game * game, const SDL_Event * event)
{
//    const float elevation_change = 0.05f;
    TileCoord dummy_coord = { 0, 0 };

    switch ( event->type ) {
        case SDL_KEYDOWN:
            switch ( event->key.keysym.sym ) {

                case SDLK_w:
                    StartTurn(game, dummy_coord, NORTH);
                    return true;

                case SDLK_s:
                    StartTurn(game, dummy_coord, SOUTH);
                    return true;

                case SDLK_a:
                    StartTurn(game, dummy_coord, WEST);
                    return true;

                case SDLK_d:
                    StartTurn(game, dummy_coord, EAST);
                    return true;

                case SDLK_1:
                    game->player_info.fuel++;
                    return true;

                case SDLK_2: {
                    Actor * player = FindActor(&game->world.map->actor_list, ACTOR_PLAYER);
                    player->stats.health++;
                    return true;
                }

                case SDLK_3:
                    game->player_info.has_shack_key = true;
                    return true;

                case SDLK_l:
                    Log("Test String!");
                    Log("Test String 2!");
                    Log("Test String Three!");
                    return true;
#if 0
                case SDLK_UP:
                        game->forest_high += elevation_change;
                        LoadLevel(game, game->level, false);
                    return true;

                case SDLK_DOWN:
                        game->forest_high -= elevation_change;
                        LoadLevel(game, game->level, false);
                    return true;

                case SDLK_LEFT:
                        game->forest_low -= elevation_change;
                        LoadLevel(game, game->level, false);
                    return true;

                case SDLK_RIGHT:
                        game->forest_low += elevation_change;
                        LoadLevel(game, game->level, false);
                    return true;
#endif
                default:
                    return false;
            }
            break;
        default:
            return false;
    }
}


bool LevelIdle_ProcessEvent(Game * game, const SDL_Event * event)
{
    if ( event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_TAB ) {
        game->inventory_open = !game->inventory_open;
    }

    if ( game->inventory_open ) {
        return InventoryProcessEvent(game, event);
    } else {
        return LevelProcessEvent(game, event);
    }
}


void LevelIdle_Update(Game * game, float dt)
{
    // Update actor standing animations, etc.

    FOR_EACH_ACTOR(actor, game->world.map->actor_list) {
        const ActorSprite * sprite = &actor->info->sprite;
        if (sprite->num_frames > 1 &&
            game->ticks % MS2TICKS(sprite->frame_msec, FPS) == 0 )
        {
            actor->frame = (actor->frame + 1) % sprite->num_frames;
        }
    }

    UpdateLevel(game, dt);
}


void LevelIdle_OnEnter(Game * game)
{
    Actor * player = FindActor(&game->world.map->actor_list, ACTOR_PLAYER);
    Tile * player_tile = GetTile(game->world.map, player->tile);

    // Check if the player has moved onto a tile that requires action:

    switch ( (TileType)player_tile->type ) {
        case TILE_TELEPORTER:
            if ( player->flags.on_teleporter ) {
                Teleport(player);
                player->flags.on_teleporter = false;
                S_Play("o0 t160 l32 c g > d a > e b > f+ > c+ g+ > d+ a+ > f > c ");
            }
            break;
        case TILE_FOREST_EXIT: // TODO: maybe exit is a flag
        case TILE_DUNGEON_EXIT:
            ++game->level;
            FadeOutAndChangeState(game, &gs_intermission, 0.5f);
            break;
        default:
            break;
    }

    if ( game->player_info.level_state == LEVEL_ENTER_SUB
        || game->player_info.level_state == LEVEL_EXIT_SUB )
    {
        FadeOutAndChangeState(game, &gs_sublevel_enter, 0.5f);
    }
}


const GameState gs_level_idle = {
    .process_event      = LevelIdle_ProcessEvent,
    .update             = LevelIdle_Update,
    .render             = GamePlayRender,
    .on_enter           = LevelIdle_OnEnter,
    .on_exit            = NULL,
    .next_state         = NULL,
};
