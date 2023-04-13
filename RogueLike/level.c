//
//  level.c
//  RogueLike
//
//  Created by Thomas Foster on 4/3/23.
//

#include "level.h"

#define FOREST_LIGHT 40

const area_info_t area_info[NUM_AREAS] = {
    [AREA_FOREST] = {
        .unrevealed_light = FOREST_LIGHT,
        .revealed_light = FOREST_LIGHT,
        .visible_light = FOREST_LIGHT,
        .debug_map_tile_size = 4,
        .reveal_all = true,
    },
    [AREA_DUNGEON] = {
        .unrevealed_light = 0,
        .revealed_light = 20,
        .visible_light = 80,
        .debug_map_tile_size = 16,
        .reveal_all = false,
    },
};
