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
#include "actors.h"

#define NUM_STARS 5000

typedef struct {
    SDL_Point pt;
    SDL_Color color;
} Star;

typedef struct world {
    Area area;
    Map map;
    Actors actors;

    ParticleArray particles;
    Star stars[NUM_STARS];

    TileCoord mouse_tile;
} World;

World InitWorld(void);
void RenderWorld(const World * world, const RenderInfo * render_info, int ticks);

#endif /* world_h */
