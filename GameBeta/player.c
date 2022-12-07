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

    box_t visible_region = GetVisibleRegion(player);

    for ( int y = visible_region.min.y; y <= visible_region.max.y; y++ ) {
        for ( int x = visible_region.min.x; x <= visible_region.max.x; x++ ) {
            game->map.tiles[y][x].visible = false; // Reset it.

            // Update tile visibility along the way.
            LineOfSight(game, player->x, player->y, x, y, true);
            num_lines++;
        }
    }

    printf("cast %d sight lines\n", num_lines);
}
