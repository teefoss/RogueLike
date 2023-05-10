//
//  player.c
//  RogueLike
//
//  Created by Thomas Foster on 11/30/22.
//

#include "game.h"


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
    Map * map = &world->map;
    Box vis = GetVisibleRegion(map, render_info);

    const Actor * player = FindActor(&world->actor_list, ACTOR_PLAYER);

    TileCoord coord;
    for ( coord.y = vis.top; coord.y <= vis.bottom; coord.y++ ) {
        for ( coord.x = vis.left; coord.x <= vis.right; coord.x++ ) {
            if ( LineOfSight(map, player->tile, coord) ) {
                RevealTile(map, coord);
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
