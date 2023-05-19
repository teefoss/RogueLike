//
//  player.c
//  RogueLike
//
//  Created by Thomas Foster on 11/30/22.
//

#include "game.h"
#include "sound.h"


void RevealTile(Map * map, TileCoord coord)
{
    Tile * tile = GetTile(map, coord);

    tile->flags.visible = true;
    tile->flags.revealed = true;

    // Also reveals tiles adjacent to floors.
    if ( !tile->flags.blocks_movement ) {
        for ( Direction d = 0; d < NUM_DIRECTIONS; d++ ) {
            Tile * adj = GetAdjacentTile(map, coord, d);
            if ( adj ) {
                adj->flags.visible = true;
                adj->flags.revealed = true;
            }
        }
    }
}


/// Reveal and set tiles visible if LOS to player.
void PlayerCastSight(World * world, const RenderInfo * render_info)
{
    const Actor * player = FindActor(&world->map->actor_list, ACTOR_PLAYER);
    Box vis = GetPlayerVisibleRegion(world->map, player->tile);

    TileCoord coord;
    for ( coord.y = vis.top; coord.y <= vis.bottom; coord.y++ ) {
        for ( coord.x = vis.left; coord.x <= vis.right; coord.x++ ) {
            if ( LineOfSight(world->map, player->tile, coord) ) {
                RevealTile(world->map, coord);
            }
        }
    }
}


bool AddToInventory(Actor * player, Actor * item_actor, Item item)
{
    Inventory * inventory = &player->game->player_info.inventory;

    if ( inventory->item_counts[item] < 8 ) {

        inventory->item_counts[item]++;
        RemoveActor(item_actor);
        return true;
    }

    return false;
}


void UseItem(Actor * player)
{
    const ActorInfo * actor_info = player->info;
    ActorsStats * stats = &player->stats;
    PlayerInfo * player_info = &player->game->player_info;
    Inventory * inventory = &player_info->inventory;

    // If the inventory is empty, just leave.
    if ( inventory->item_counts[inventory->selected_item] == 0 ) {
        S_Play("o1 t100 l32 d");
        return;
    }

    const char * cant_use_sound = "l32 o4 e c+";

    // TODO: clean this up
    switch ( inventory->selected_item ) {
        case ITEM_HEALTH:
            if ( stats->health < actor_info->max_health ) {
                stats->health++;
                --inventory->item_counts[inventory->selected_item];
                S_Play("l32 o3 d+ < g+ b e");
            } else {
                S_Play(cant_use_sound);
            }
            break;
        case ITEM_TURN:
            player_info->turns++;
            S_Play("l32 o3 f+ < b a");
            --inventory->item_counts[inventory->selected_item];
            break;
        case ITEM_STRENGTH:
            if ( player_info->strength_buff == 0 ) {
                player_info->strength_buff = 1;
                S_Play("l32 t100 o1 e a f b- f+ b");
                --inventory->item_counts[inventory->selected_item];
            } else {
                S_Play(cant_use_sound);
            }
            break;
        case ITEM_FUEL_SMALL:
            if ( player_info->fuel < MAX_FUEL) {
                player_info->fuel++;
                player_info->fuel_steps = FUEL_STEPS;
                S_Play("o3 l16 t160 b e-");
                --inventory->item_counts[inventory->selected_item];
            } else {
                S_Play(cant_use_sound);
            }
            break;
        case ITEM_FUEL_BIG:
            if ( player_info->fuel < MAX_FUEL) {
                player_info->fuel = MIN(player_info->fuel + 2, MAX_FUEL);
                player_info->fuel_steps = FUEL_STEPS;
                S_Play("o2 l16 t160 b e-");
                --inventory->item_counts[inventory->selected_item];
            } else {
                S_Play(cant_use_sound);
            }
            break;
        default:
            break;
    }
}
