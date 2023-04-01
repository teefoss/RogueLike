//
//  player.c
//  RogueLike
//
//  Created by Thomas Foster on 11/30/22.
//

#include "game.h"

void PlayerCastSightLines(game_t * game)
{
    int num_lines = 0;

    box_t vis = GetVisibleRegion(game);
    const actor_t * player = GetPlayer(game);

    tile_coord_t tile;
    for ( tile.y = vis.min.y; tile.y <= vis.max.y; tile.y++ ) {
        for ( tile.x = vis.min.x; tile.x <= vis.max.x; tile.x++ ) {
            GetTile(&game->map, tile)->flags.visible = false; // Reset it.

            // Update tile visibility along the way.
            LineOfSight(&game->map, player->tile, tile, true);
            num_lines++;
        }
    }

    printf("cast %d sight lines\n", num_lines);
}


void CollectItem(actor_t * player, actor_t * item_actor, item_t item)
{
    inventory_t * inventory = &player->game->inventory;

    if ( inventory->item_counts[item] < 8 ) {
        if ( InventoryIsEmtpy(inventory) ) {
            inventory->selected_item = item;
        }

        inventory->item_counts[item]++;
        item_actor->flags.remove = true;
    }
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
