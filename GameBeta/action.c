//
//  action.c
//  GameBeta
//
//  Created by Thomas Foster on 11/8/22.
//

#include "main.h"
#include <limits.h>

/// Blobs take one step toward the player, if visible. NSEW only.
void A_Blob(actor_t * blob, game_t * game)
{
    actor_t * player = &game->actors[0];

    if ( LineOfSight(game, blob->x, blob->y, player->x, player->y, false) ) {
        // Find the tile with the smallest distance to player.
        direction_t best = NO_DIRECTION;
        int min_distance = INT_MAX;
        for ( int d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
            tile_t * adjacent = GetAdjacentTile(game->map.tiles,
                                                blob->x,
                                                blob->y,
                                                d);

            if ( adjacent->distance < min_distance ) {
                min_distance = adjacent->distance;
                best = d;
            }
        }

        TryMoveActor(blob, game, x_deltas[best], y_deltas[best]);
    }
}
