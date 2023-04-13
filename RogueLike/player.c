//
//  player.c
//  RogueLike
//
//  Created by Thomas Foster on 11/30/22.
//

#include "game.h"


void SetTileVisible(map_t * map, tile_coord_t coord)
{
    tile_t * tile = GetTile(map, coord);

    tile->flags.visible = true;
    tile->flags.revealed = true;
    printf(" - set %d, %d to vis and revealed\n", coord.x, coord.y);

    // Also reveals tiles adjacent to floors.
    if ( !tile->flags.blocking ) {
        printf(" - %d, %d is a floor\n", coord.x, coord.y);
        for ( direction_t d = 0; d < NUM_DIRECTIONS; d++ ) {
            tile_t * adj = GetAdjacentTile(map, coord, d);
            if ( adj ) {
                printf(" - setting adjacent tile at direction %d to vis, rev\n", d);
                adj->flags.visible = true;
                adj->flags.revealed = true;
            }
        }
    }
}


/// Reveal and set tiles visible if LOS to player.
void PlayerCastSight(game_t * game)
{
    box_t vis = GetVisibleRegion(game);
    const actor_t * player = GetPlayer(game);

    tile_coord_t coord;
    for ( coord.y = vis.min.y; coord.y <= vis.max.y; coord.y++ ) {
        for ( coord.x = vis.min.x; coord.x <= vis.max.x; coord.x++ ) {
            if ( LineOfSight(&game->map, player->tile, coord) ) {
                printf("player can see (%d, %d)\n", coord.x, coord.y);
                SetTileVisible(&game->map, coord);
            }
        }
    }
}


bool CollectItem(actor_t * player, actor_t * item_actor, item_t item)
{
    inventory_t * inventory = &player->game->inventory;

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


const actor_t * GetPlayerConst(const game_t * game)
{
    const actors_t * actors = &game->map.actors;

    for ( int i = 0; i < actors->count; i++ ) {
        if ( actors->list[i].type == ACTOR_PLAYER ) {
            return &actors->list[i];
        }
    }

    return NULL;
}


actor_t * GetPlayerNonConst(game_t * game)
{
    actors_t * actors = &game->map.actors;

    for ( int i = 0; i < actors->count; i++ ) {
        if ( actors->list[i].type == ACTOR_PLAYER ) {
            return &actors->list[i];
        }
    }

    return NULL;
}
