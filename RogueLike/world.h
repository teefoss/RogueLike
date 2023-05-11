//
//  world.h
//  RogueLike
//
//  Created by Thomas Foster on 4/14/23.
//

#ifndef world_h
#define world_h

#include "actor.h"
#include "map.h"
#include "particle.h"
#include "render.h"
#include "actor_list.h"

typedef enum area {
    AREA_FOREST,
    AREA_DUNGEON,
    NUM_AREAS,
} Area;

typedef struct {
    u8 unrevealed_light;
    u8 revealed_light;
    u8 visible_light;

    u8 debug_map_tile_size;
    bool reveal_all; // No fog of war

    SDL_Color render_clear_color;
} AreaInfo;

typedef struct world {
    Area area;
    const AreaInfo * info;

    Map map;
    ActorList actor_list;
    ParticleArray particles;

    TileCoord mouse_tile;
} World;

extern const AreaInfo area_info[NUM_AREAS];

World InitWorld(void);
void RenderWorld(const World * world, const RenderInfo * render_info, int ticks);

void GenerateWorld(Game * game, Area area, int seed, int width, int height);
void GenerateForest(Game * game, int seed, int width);
void GenerateDungeon(Game * game, int width, int height);

#endif /* world_h */
