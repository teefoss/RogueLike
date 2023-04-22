//
//  player.c
//  RogueLike
//
//  Created by Thomas Foster on 11/30/22.
//

#include "game.h"


void SetTileVisible(Map * map, TileCoord coord)
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

    const Actor * player = GetPlayer(&world->actors);

    TileCoord coord;
    for ( coord.y = vis.top; coord.y <= vis.bottom; coord.y++ ) {
        for ( coord.x = vis.left; coord.x <= vis.right; coord.x++ ) {
            if ( LineOfSight(map, player->tile, coord) ) {
                SetTileVisible(map, coord);
            }
        }
    }
}


bool CollectItem(Actor * player, Actor * item_actor, Item item)
{
    Inventory * inventory = &player->game->player_info.inventory;

    if ( inventory->item_counts[item] < 8 ) {
        if ( InventoryIsEmtpy(inventory) ) {
            inventory->selected_item = item;
        }

        inventory->item_counts[item]++;
        item_actor->flags.remove = true;
        return true;
    }

    return false;
}

#define GET_PLAYER_FUNC_BODY \
    for ( int i = 0; i < actors->count; i++ )           \
        if ( actors->list[i].type == ACTOR_PLAYER )     \
            return &actors->list[i];                    \
    return NULL

GET_PLAYER_CONST_FUNC_SIG {
    GET_PLAYER_FUNC_BODY;
}

GET_PLAYER_NONCONST_FUNC_SIG {
    GET_PLAYER_FUNC_BODY;
}
