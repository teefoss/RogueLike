//
//  level.h
//  RogueLike
//
//  Created by Thomas Foster on 4/3/23.
//

#ifndef level_h
#define level_h

#include "inttypes.h"
#include <stdbool.h>

typedef enum {
    AREA_FOREST,
    AREA_DUNGEON,
    NUM_AREAS,
} area_t;

typedef struct {
    u8 unrevealed_light;
    u8 revealed_light;
    u8 visible_light;
    u8 debug_map_tile_size;
    bool reveal_all; // No fog of war
} area_info_t;

extern const area_info_t area_info[NUM_AREAS];

#endif /* level_h */
