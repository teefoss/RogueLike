//
//  action.c
//  GameBeta
//
//  Created by Thomas Foster on 11/8/22.
//

#include "main.h"
#include <limits.h>

/// Blobs take one step toward the player, if visible. NSEW only.
void A_Blob(actor_t * blob)
{
    map_t * map = &blob->game->map;
    actor_t * player = GetPlayer(&map->actors);

    if ( LineOfSight(map, blob->tile, player->tile, false) ) {

        // Find the tile with the smallest distance to player.
        direction_t best_direction = NO_DIRECTION;
        int min_distance = INT_MAX;

        for ( direction_t d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
            tile_t * adj = GetAdjacentTile(map, blob->tile, d);

            if ( !adj->flags.blocking && adj->distance < min_distance )
            {
                min_distance = adj->distance;
                best_direction = d;
            }
        }

        TryMoveActor(blob, best_direction);
    }
}
