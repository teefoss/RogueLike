//
//  tile.c
//  GameBeta
//
//  Created by Thomas Foster on 12/12/22.
//

#include "main.h"
#include "mathlib.h"

static tile_t tile_templates[] = {
    [TILE_FLOOR] = {
        .sprite_cell = { 0, 1 },
        .num_variants = 8,
    },
    [TILE_WALL] = {
        .flags = { .blocking = true },
        .sprite_cell = { 0, 0 },
    },
    [TILE_DOOR] = {
        .flags = { .blocking = true },
        .sprite_cell = { 3, 3 },
    },
    [TILE_EXIT] = {
        .flags = { .player_only = true },
        .sprite_cell = { 0, 3 },
    },
    [TILE_GOLD_DOOR] = {
        .flags = { .blocking = true },
        .sprite_cell = { 2, 3 },
    },
    [TILE_START] = {
        .sprite_cell = { 4, 3 },
    }
};

tile_t CreateTile(tile_type_t type)
{
    tile_t tile = tile_templates[type];
    tile.type = type;
    tile.variety = Random(0, 255);

    return tile;
}
