//
//  player.c
//  RogueLike
//
//  Created by Thomas Foster on 11/30/22.
//

#include "game.h"

void PlayerCastSightLines(map_t * map)
{
    actor_t * player = GetPlayer(&map->actors);
    int num_lines = 0;

    box_t visible_region = GetVisibleRegion(map, player);

    tile_coord_t tile;
    for ( tile.y = visible_region.min.y; tile.y <= visible_region.max.y; tile.y++ ) {
        for ( tile.x = visible_region.min.x; tile.x <= visible_region.max.x; tile.x++ ) {
            GetTile(map, tile)->flags.visible = false; // Reset it.

            // Update tile visibility along the way.
            LineOfSight(map, player->tile, tile, true);
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
