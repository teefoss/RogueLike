//
//  world.c
//  RogueLike
//
//  Created by Thomas Foster on 4/14/23.
//

#include "world.h"
#include "game.h"

#include "video.h"

#define FOREST_LIGHT 40

const AreaInfo area_info[NUM_AREAS] = {
    [AREA_FOREST] = {
        .unrevealed_light = FOREST_LIGHT,
        .revealed_light = FOREST_LIGHT,
        .visible_light = FOREST_LIGHT,
        .debug_map_tile_size = 4,
        .reveal_all = true,
        .render_clear_color = { 0, 0, 64 }
    },
    [AREA_DUNGEON] = {
        .unrevealed_light = 0,
        .revealed_light = 10, // was 20
        .visible_light = 80,
        .debug_map_tile_size = 16,
        .reveal_all = false,
        .render_clear_color = { 0, 0, 0 }
    },
};


World InitWorld(void)
{
    World world = { 0 };

    InitParticleArray(&world.particles);

    return world;
}


void GenerateWorld(Game * game, Area area, int seed, int width, int height)
{
    // Free up all previously loaded actors.
    RemoveAllActors(&game->world.actor_list);

    game->world.area = area;
    game->world.info = &area_info[area];

    switch ( area ) {
        case AREA_FOREST:
            GenerateForest(game, seed, width);
            break;
        case AREA_DUNGEON:
            GenerateDungeon(game, width, height);
            break;
        default:
            Error("weird area number!");
            break;
    }
}


static int CompareActors(const void * a, const void * b) {
    const Actor ** actor1 = (const Actor **)a;
    const Actor ** actor2 = (const Actor **)b;

    if ((*actor1)->tile.y != (*actor2)->tile.y) {
        return (*actor1)->tile.y - (*actor2)->tile.y;
    } else {
        return (*actor1)->info->sprite.draw_priority - (*actor2)->info->sprite.draw_priority;
    }
}


void RenderWorld(const World * world, const RenderInfo * render_info, int ticks)
{
    V_SetColor(world->info->render_clear_color);
    V_Clear();

    const SDL_Rect viewport = GetLevelViewport(render_info);
    SDL_RenderSetViewport(renderer, &viewport);

    if ( world->info == &area_info[AREA_FOREST] ) {
//        RenderForestBackground(world->stars);
        V_DrawTexture(render_info->stars, NULL, NULL);
    }

    vec2_t offset = GetRenderLocation(render_info, render_info->camera);
    Box vis_rect = GetVisibleRegion(&world->map, render_info);

    RenderTiles(world, &vis_rect, offset, false);
    RenderParticles(&world->particles, DRAW_SCALE, offset);

    // Make a list of visible actors.


    int num_visible_actors = 0;
    Actor ** visible_actors = GetVisibleActors(world,
                                               render_info,
                                               &num_visible_actors);
    SDL_qsort(visible_actors, num_visible_actors, sizeof(Actor *), CompareActors);

    // Draw actors.

    for ( int i = 0; i < num_visible_actors; i++ ) {
        const Actor * a = visible_actors[i];
        int size = SCALED(TILE_SIZE);
        int x = a->tile.x * size + a->offset_current.x - offset.x;
        int y = a->tile.y * size + a->offset_current.y - offset.y;
        RenderActor(a, x, y, size, false, ticks);
    }

    SDL_RenderSetViewport(renderer, NULL);
}
