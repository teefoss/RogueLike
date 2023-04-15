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
void PlayerCastSight(Game * game)
{
    box_t vis = GetVisibleRegion(game);
    const Actor * player = GetPlayer(game);

    TileCoord coord;
    for ( coord.y = vis.top; coord.y <= vis.bottom; coord.y++ ) {
        for ( coord.x = vis.left; coord.x <= vis.right; coord.x++ ) {
            if ( LineOfSight(&game->map, player->tile, coord) ) {
                SetTileVisible(&game->map, coord);
            }
        }
    }
}


bool CollectItem(Actor * player, Actor * item_actor, Item item)
{
    Inventory * inventory = &player->game->inventory;

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


const Actor * GetPlayerConst(const Game * game)
{
    const Actors * actors = &game->map.actors;

    for ( int i = 0; i < actors->count; i++ ) {
        if ( actors->list[i].type == ACTOR_PLAYER ) {
            return &actors->list[i];
        }
    }

    return NULL;
}


Actor * GetPlayerNonConst(Game * game)
{
    Actors * actors = &game->map.actors;

    for ( int i = 0; i < actors->count; i++ ) {
        if ( actors->list[i].type == ACTOR_PLAYER ) {
            return &actors->list[i];
        }
    }

    return NULL;
}
