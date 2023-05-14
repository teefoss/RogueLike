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
    const Actor * player = FindActor(&world->actor_list, ACTOR_PLAYER);
    Box vis = GetPlayerVisibleRegion(&world->map, player->tile);

    TileCoord coord;
    for ( coord.y = vis.top; coord.y <= vis.bottom; coord.y++ ) {
        for ( coord.x = vis.left; coord.x <= vis.right; coord.x++ ) {
            if ( LineOfSight(&world->map, player->tile, coord) ) {
                RevealTile(&world->map, coord);
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
        return;
    }

    // Remove from inventory.
    --inventory->item_counts[inventory->selected_item];

    switch ( inventory->selected_item ) {
        case ITEM_HEALTH:
            if ( stats->health < actor_info->max_health ) {
                stats->health++;
            }
            S_Play("l32 o3 d+ < g+ b e");
            break;
        case ITEM_TURN:
            player_info->turns++;
            S_Play("l32 o3 f+ < b a");
            break;
        case ITEM_STRENGTH:
            player_info->strength_buff = 1;
            S_Play("l32 t100 o1 e a f b- f+ b");
            break;
        case ITEM_FUEL_SMALL:
            player_info->fuel = MIN(player_info->fuel + 1, MAX_FUEL);
            player_info->fuel_steps = FUEL_STEPS;
            S_Play("o3 l16 t160 b e-");
            break;
        case ITEM_FUEL_BIG:
            player_info->fuel = MIN(player_info->fuel + 3, MAX_FUEL);
            player_info->fuel_steps = FUEL_STEPS;
            S_Play("o2 l16 t160 b e-");
            break;
        default:
            break;
    }
}
