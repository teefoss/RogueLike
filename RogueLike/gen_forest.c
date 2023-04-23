//
//  forest.c
//  RogueLike
//
//  Created by Thomas Foster on 4/1/23.
//

#include "game.h"
#include "tile.h"
#include "mathlib.h"

#define FOREST_SIZE 128

int tile_regions[FOREST_SIZE][FOREST_SIZE];
int region_areas[FOREST_SIZE * FOREST_SIZE];

static STORAGE(TileCoord, FOREST_SIZE * FOREST_SIZE) ground_coords;
static int num_tiles;
static TileCoord tiles[FOREST_SIZE * FOREST_SIZE];

static void GetTilesInRegion(const Map * map, int region)
{
    num_tiles = 0;
    TileCoord coord;
    for ( coord.y = 0; coord.y < FOREST_SIZE; coord.y++ ) {
        for ( coord.x = 0; coord.x < FOREST_SIZE; coord.x++ ) {
            if ( tile_regions[coord.y][coord.x] == region ) {
                tiles[num_tiles++] = coord;
            }
        }
    }
}


static void RemoveTile(int index)
{
    tiles[index] = tiles[--num_tiles];
}


static int RandomIndex(void)
{
    return Random(0, num_tiles - 1);
}


static void SpawnActorAtRandomLocation(Game * game, ActorType type)
{
    if ( num_tiles != 0 ) {
        int index = RandomIndex();
        SpawnActor(game, type, tiles[index]);
        RemoveTile(index);
    } else {
        printf("%s: not tiles left!\n", __func__);
    }
}


static TileCoord CreateTileAtRandomLocation(Map * map, TileType type)
{
    int index = RandomIndex();
    TileCoord coord = tiles[index];

    Tile * tile = GetTile(map, coord);
    *tile = CreateTile(type);
    tile->flags.revealed = true;
    RemoveTile(index);

    return coord;
}


static void FloodFillGroundTiles_r(Map * map, TileCoord coord, int region)
{
    Tile * tile = GetTile(map, coord);
    if ( tile == NULL ) {
        printf("NULL tile\n");
        return;
    }

    if ( tile->type == TILE_FLOOR && tile_regions[coord.y][coord.x] == -1 ) {
        tile_regions[coord.y][coord.x] = region;
        region_areas[region]++;
        for ( Direction d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
            TileCoord adjacent_coord = AdjacentTileCoord(coord, d);
            FloodFillGroundTiles_r(map, adjacent_coord, region);
        }
    }
}


void GenerateForest(Game * game, int seed)
{
    World * world = &game->world;
    world->area = AREA_FOREST;

    for ( int i = 0; i < NUM_STARS; i++ ) {
        world->stars[i].pt.x = Random(0, GAME_WIDTH - 1);
        world->stars[i].pt.y = Random(0, GAME_HEIGHT - 1);

        SDL_Color * c = &world->stars[i].color;
        int n = Random(0, 1000);
        if ( n == 1000 ) {
            *c = (SDL_Color){ 0, 248, 0, 255 };
        } else if ( n < 500 ) {
            *c = (SDL_Color){ 208, 208, 208, 255 };
        } else {
            *c = (SDL_Color){ 128, 128, 128, 255 };
        }
    }

    SDL_memset(region_areas, 0, sizeof(region_areas));

    Map * map = &world->map;

    map->width = FOREST_SIZE;
    map->height = FOREST_SIZE;
    int map_size = map->width * map->height;
    int radius = map->height / 2;

    if ( map->tiles ) {
        free(map->tiles);
    }
    map->tiles = calloc(map_size, sizeof(*map->tiles));

    for ( int i = 0; i < map_size; i++ ) {
        map->tiles[i] = CreateTile(TILE_WALL);
    }

    world->actors.count = 0;

    RandomizeNoise(seed);
//    RandomizeNoise(0);
    CLEAR(ground_coords);

    for ( int y = 0; y < FOREST_SIZE; y++ ) {
        for ( int x = 0; x < FOREST_SIZE; x++ ) {
            TileCoord coord = { x, y };

            // Distance from this tile to center of map.
            float distance = DISTANCE(x, y, FOREST_SIZE / 2, FOREST_SIZE / 2);

            float noise;
            float water_noise;

            if ( distance <= radius ) {
                float gradient = MAP(distance, 0.0f, (float)radius, 0.0f, 1.0f);
                float freq = 0.04f;
                float amp = 1.0f;
                float pers = 0.6f;
                float lac = 2.0f;
                noise = Noise2(x, y, 1.0f, freq, 6, amp, pers, lac) - gradient;

                water_noise = Noise2(x, y, 8241931.0f, freq, 6, amp, pers, lac)
                - gradient;
            } else {
                noise = -1.0f; // Outside level radius.
                water_noise = -1.0f;
            }

            Tile * tile = GetTile(map, coord);
            if ( noise < -0.35f || noise > 0.05 ) {
                *tile = CreateTile(TILE_WALL);
            } else {
                *tile = CreateTile(TILE_FLOOR);
            }

            if ( water_noise > 0.05f ) {
                *tile = CreateTile(TILE_WATER);
            }

            if ( tile->type == TILE_WALL ) {
                SpawnActor(game, ACTOR_TREE, coord);
            } else if ( tile->type == TILE_FLOOR ) {
                APPEND(ground_coords, coord);
            }

            tile_regions[y][x] = -1; // Reset all tiles' region
            tile->flags.revealed = true;
        }
    }

    // For all ground tiles
    int region = -1;
    for ( int i = 0; i < ground_coords.count; i++ ) {
        TileCoord coord = ground_coords.data[i];
        if ( tile_regions[coord.y][coord.x] == -1 ) { // Not yet visited
            region++;
            FloodFillGroundTiles_r(map, coord, region);
        }
    }

//    printf("total regions: %d\n", region);
    int largest_region = -1;
    int second_largest_region = -1;

    for ( int i = 0; i <= region; i++ ) {
//        printf("region %d area: %d\n", i, region_areas[i]);
        if ( region_areas[i] > region_areas[largest_region] ) {
            largest_region = i;
        } else if ( region_areas[i] > region_areas[second_largest_region] ) {
            second_largest_region = i;
        }
    }

    // Spawn actor, spiders, and teleporter in second largest region.
    GetTilesInRegion(map, second_largest_region);
    SpawnActorAtRandomLocation(game, ACTOR_PLAYER);
    CreateTileAtRandomLocation(&world->map, TILE_TELEPORTER);
    for ( int i = 0; i < region_areas[second_largest_region] / 20; i++ ) {
        SpawnActorAtRandomLocation(game, ACTOR_SPIDER);
    }

    // Spawn second teleporter, spiders, and exit in largest region.
    GetTilesInRegion(map, largest_region);
    CreateTileAtRandomLocation(map, TILE_TELEPORTER);
    TileCoord exit_coord = CreateTileAtRandomLocation(map, TILE_EXIT);
    SpawnActor(game, ACTOR_WELL, exit_coord);
    for ( int i = 0; i < region_areas[largest_region] / 10; i++ ) {
        SpawnActorAtRandomLocation(game, ACTOR_SPIDER);
    }
}
