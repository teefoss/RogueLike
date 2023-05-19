//
//  world.c
//  RogueLike
//
//  Created by Thomas Foster on 4/14/23.
//

#include "world.h"
#include "game.h"
#include "debug.h"

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
    RemoveAllActors(&game->world.map->actor_list);

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
    Box vis_rect = GetCameraVisibleRegion(world->map, render_info);

    RenderTiles(world, &vis_rect, offset, false, render_info);
    RenderParticles(&world->particles, DRAW_SCALE, offset);

    // Make a list of visible actors.


    int num_visible_actors = 0;
    Actor ** visible_actors = GetVisibleActors(world,
                                               render_info,
                                               &num_visible_actors);
    SDL_qsort(visible_actors, num_visible_actors, sizeof(Actor *), CompareActors);

    // Draw actors.

    float start = ProgramTime();

    for ( int i = 0; i < num_visible_actors; i++ ) {
        const Actor * a = visible_actors[i];
        int size = SCALED(TILE_SIZE);
        int x = a->tile.x * size + a->offset_current.x - offset.x;
        int y = a->tile.y * size + a->offset_current.y - offset.y;
        RenderActor(a, x, y, size, false, ticks);
    }

    actors_msec = ProgramTime() - start;

    SDL_RenderSetViewport(renderer, NULL);
}


/// - parameter region: the rectangular area of the map to draw or NULL to draw
/// entire map.
void RenderTiles(const World * world,
                 const Box * region,
                 vec2_t offset,
                 bool debug,
                 const RenderInfo * render_info)
{
    float start = ProgramTime();

    const Map * map = world->map;

    int tile_size;
    if ( debug ) {
//        tile_size = area_info[world->area].debug_map_tile_size;
        tile_size = render_info->height / map->height;
    } else {
        tile_size = SCALED(TILE_SIZE);
    }

    Box use;
    if ( region == NULL ) {
        use = (Box){ 0, 0, map->width - 1, map->height - 1 };
    } else {
        use = *region;
    }

    TileCoord coord;
    for ( coord.y = use.top; coord.y <= use.bottom; coord.y++ ) {
        for ( coord.x = use.left; coord.x <= use.right; coord.x++ ) {
            const Tile * tile = GetTile((Map *)map, coord);

            int signature = CalculateWallSignature(map, coord, false);

            int pixel_x = coord.x * tile_size - offset.x;
            int pixel_y = coord.y * tile_size - offset.y;

            RenderTile(tile,
                       world->area,
                       signature,
                       pixel_x,
                       pixel_y,
                       tile_size,
                       debug,
                       render_info);

            if ( show_debug_info && TileCoordsEqual(coord, world->mouse_tile) ) {
                SDL_Rect highlight = { pixel_x, pixel_y, tile_size, tile_size };
                V_SetRGB(255, 80, 80);
                V_DrawRect(&highlight);
            }
        }
    }

    tiles_msec = ProgramTime() - start;
}


void ResetTileVisibility(World * world,
                         TileCoord player_tile,
                         const RenderInfo * render_info)
{
    Box vis = GetCameraVisibleRegion(world->map, render_info);

    TileCoord coord;
    for ( coord.y = vis.top; coord.y <= vis.bottom; coord.y++ ) {
        for ( coord.x = vis.left; coord.x <= vis.right; coord.x++ ) {
            Tile * tile = GetTile(world->map, coord);
            if ( !world->info->reveal_all ) {
                tile->flags.visible = false;
            }
        }
    }
}
