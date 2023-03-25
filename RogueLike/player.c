//
//  player.c
//  GameBeta
//
//  Created by Thomas Foster on 11/30/22.
//

#include "main.h"

void PlayerCastSightLines(game_t * game, const actor_t * player)
{
    int num_lines = 0;

    box_t visible_region = GetVisibleRegion(&game->map, player);

    for ( int y = visible_region.min.y; y <= visible_region.max.y; y++ ) {
        for ( int x = visible_region.min.x; x <= visible_region.max.x; x++ ) {
            GetTile(&game->map, x, y)->visible = false; // Reset it.

            // Update tile visibility along the way.
            LineOfSight(game, player->x, player->y, x, y, true);
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
        item_actor->remove = true;
    }
}
