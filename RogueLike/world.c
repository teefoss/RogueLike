//
//  world.c
//  RogueLike
//
//  Created by Thomas Foster on 4/14/23.
//

#include "world.h"
#include "game.h"

#include "video.h"

World InitWorld(void)
{
    World world;
    InitParticleArray(&world.particles);

    return world;
}


static int CompareActors(const void * a, const void * b) {
    const Actor * actor1 = (const Actor *)a;
    const Actor * actor2 = (const Actor *)b;

    if (actor1->tile.y != actor2->tile.y) {
        return actor1->tile.y - actor2->tile.y;
    } else {
        return actor1->sprite->draw_priority - actor2->sprite->draw_priority;
    }
}


void RenderWorld(const World * world, const RenderInfo * render_info, int ticks)
{
    const SDL_Rect viewport = GetLevelViewport(render_info);
    SDL_RenderSetViewport(renderer, &viewport);

    vec2_t offset = GetRenderLocation(render_info, render_info->camera);
    Box vis_rect = GetVisibleRegion(&world->map, render_info);

    RenderTiles(world, &vis_rect, offset, false);
    RenderParticles(&world->particles, DRAW_SCALE, offset);

    // Make a list of visible actors.

    Actor visible_actors[MAX_ACTORS];
    int num_visible_actors = 0;

    const Actors * actors = &world->actors;
    for ( int i = 0; i < actors->count; i++ ) {
        const Actor * actor = &actors->list[i];
        const Tile * tile = GetTile((Map *)&world->map, actor->tile);

        if ( (tile->flags.visible || area_info[world->area].reveal_all )
            && actor->tile.x >= vis_rect.left
            && actor->tile.x <= vis_rect.right
            && actor->tile.y >= vis_rect.top
            && actor->tile.y <= vis_rect.bottom )
        {
            visible_actors[num_visible_actors++] = *actor;
        }
    }

    SDL_qsort(visible_actors, num_visible_actors, sizeof(Actor), CompareActors);

    // Draw actors.

    for ( int i = 0; i < num_visible_actors; i++ ) {
        const Actor * a = &visible_actors[i];
        int size = SCALED(TILE_SIZE);
        int x = a->tile.x * size + a->offset_current.x - offset.x;
        int y = a->tile.y * size + a->offset_current.y - offset.y;
        RenderActor(a, x, y, size, false, ticks);
    }

    SDL_RenderSetViewport(renderer, NULL);
}
