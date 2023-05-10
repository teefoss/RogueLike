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
        return (*actor1)->sprite->draw_priority - (*actor2)->sprite->draw_priority;
    }
}


void RenderMoon(int x, int y, int size)
{
    SDL_Rect moon1_rect = {
        .x = x - size / 2,
        .y = y - size / 2,
        .w = size,
        .h = size,
    };

    V_SetGray(248);
    V_FillRect(&moon1_rect);
}


void RenderForestBackground(const Star * stars)
{

    for ( int i = 0; i < NUM_STARS; i++ ) {
//        V_SetGray(232);
        V_SetColor(stars[i].color);
        SDL_Rect r = {
            stars[i].pt.x,
            stars[i].pt.y,
            SCALED(1) / 2, // Half a pixel wide
            SCALED(1) / 2
        };
        V_FillRect(&r);
    }

    int moom_size = SCALED(TILE_SIZE * 3);
    RenderMoon(GAME_WIDTH * 0.66, GAME_HEIGHT * 0.66, moom_size);
    RenderMoon(GAME_WIDTH * 0.33, GAME_HEIGHT * 0.33, moom_size * 0.66);
}


void RenderWorld(const World * world, const RenderInfo * render_info, int ticks)
{
    // TODO: move to area_info

    V_SetColor(world->info->render_clear_color);
    V_Clear();

    const SDL_Rect viewport = GetLevelViewport(render_info);
    SDL_RenderSetViewport(renderer, &viewport);

    if ( world->info == &area_info[AREA_FOREST] ) {
        RenderForestBackground(world->stars);
    }

    vec2_t offset = GetRenderLocation(render_info, render_info->camera);
    Box vis_rect = GetVisibleRegion(&world->map, render_info);

    RenderTiles(world, &vis_rect, offset, false);
    RenderParticles(&world->particles, DRAW_SCALE, offset);

    // Make a list of visible actors.

    // Include a padding since the camera may be mid-tile. For the y,
    // include extra padding in case there are tall actors visible
    static const Actor ** visible_actors = NULL;
    static int capacity = 0;
    int num_visible_actors = 0;

    int w = (vis_rect.right - vis_rect.left) + 1;
    int h = (vis_rect.bottom - vis_rect.top) + 1;
    int area = w * h;

    if ( area > capacity ) {
        printf("area %d is greater than capacity %d, resizing\n", area, capacity);
        size_t new_size = area * sizeof(Actor *);
        if ( capacity == 0 ) {
            visible_actors = malloc(new_size);
        } else {
            visible_actors = realloc(visible_actors, new_size);
        }

        if ( visible_actors == NULL ) {
            Error("could not malloc visible actor array");
        }

        capacity = area;
    }

    FOR_EACH_ACTOR_CONST(actor, world->actor_list) {
        const Tile * tile = GetTile(&world->map, actor->tile);

        if ( tile->flags.visible && TileInBox(actor->tile, vis_rect) ) {
            visible_actors[num_visible_actors++] = actor;
        }
    }

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
