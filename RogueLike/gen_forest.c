//
//  forest.c
//  RogueLike
//
//  Created by Thomas Foster on 4/1/23.
//

#include "game.h"
#include "mathlib.h"

#define FOREST_SIZE 128

int tile_regions[FOREST_SIZE][FOREST_SIZE];
int region_areas[FOREST_SIZE * FOREST_SIZE];

static int num_tiles;
static tile_coord_t tiles[FOREST_SIZE * FOREST_SIZE];

static void GetTilesInRegion(const map_t * map, int region)
{
    num_tiles = 0;
    tile_coord_t coord;
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


static void SpawnActorAtRandomLocation(game_t * game, actor_type_t type)
{
    int index = RandomIndex();
    SpawnActor(game, type, tiles[index]);
    RemoveTile(index);
}


static tile_coord_t CreateTileAtRandomLocation(map_t * map, tile_type_t type)
{
    int index = RandomIndex();
    tile_coord_t coord = tiles[index];
    *GetTile(map, coord) = CreateTile(type);
    RemoveTile(index);

    return coord;
}


static void FloodFillGroundTiles_r(map_t * map, tile_coord_t coord, int region)
{
    tile_t * tile = GetTile(map, coord);
    if ( tile == NULL ) {
        printf("NULL tile\n");
        return;
    }
    if ( tile->type == TILE_FLOOR && tile_regions[coord.y][coord.x] == -1 ) {
        tile_regions[coord.y][coord.x] = region;
        region_areas[region]++;
        for ( direction_t d = 0; d < NUM_CARDINAL_DIRECTIONS; d++ ) {
            tile_coord_t adjacent_coord = AdjacentTileCoord(coord, d);
            FloodFillGroundTiles_r(map, adjacent_coord, region);
        }
    }
}


void GenerateForest(game_t * game)
{
    game->area = AREA_FOREST;

    for ( int i = 0; i < NUM_STARS; i++ ) {
        game->stars[i].x = Random(0, GAME_WIDTH - 1);
        game->stars[i].y = Random(0, GAME_HEIGHT - 1);
    }

    map_t * map = &game->map;

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

    map->actors.count = 0;

    RandomizeNoise((u32)time(NULL));
//    RandomizeNoise(0);
    STORAGE(tile_coord_t, FOREST_SIZE * FOREST_SIZE) ground_coords = { 0 };

    for ( int y = 0; y < FOREST_SIZE; y++ ) {
        for ( int x = 0; x < FOREST_SIZE; x++ ) {
            tile_coord_t coord = { x, y };

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

            tile_t * tile = GetTile(map, coord);
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

            tile_regions[y][x] = -1;
            tile->flags.revealed = true;
        }
    }

    // For all ground tiles
    int region = -1;
    for ( int i = 0; i < ground_coords.count; i++ ) {
        tile_coord_t coord = ground_coords.data[i];
        if ( tile_regions[coord.y][coord.x] == -1 ) { // Not yet visited
            region++;
            FloodFillGroundTiles_r(map, coord, region);
        }
    }

    printf("total regions: %d\n", region);
    int largest_region = -1;
    int second_largest_region = -1;

    for ( int i = 0; i <= region; i++ ) {
        printf("region %d area: %d\n", i, region_areas[i]);
        if ( region_areas[i] > region_areas[largest_region] ) {
            largest_region = i;
        } else if ( region_areas[i] > region_areas[second_largest_region] ) {
            second_largest_region = i;
        }
    }

    // Spawn actor, spiders, and teleporter in second largest region.
    GetTilesInRegion(map, second_largest_region);
    SpawnActorAtRandomLocation(game, ACTOR_PLAYER);
    CreateTileAtRandomLocation(&game->map, TILE_TELEPORTER);
    for ( int i = 0; i < region_areas[second_largest_region] / 20; i++ ) {
        SpawnActorAtRandomLocation(game, ACTOR_SPIDER);
    }

    // Spawn second teleporter, spiders, and exit in largest region.
    GetTilesInRegion(map, largest_region);
    CreateTileAtRandomLocation(&game->map, TILE_TELEPORTER);
    tile_coord_t exit_coord = CreateTileAtRandomLocation(&game->map, TILE_EXIT);
    SpawnActor(game, ACTOR_WELL, exit_coord);
    for ( int i = 0; i < region_areas[largest_region] / 10; i++ ) {
        SpawnActorAtRandomLocation(game, ACTOR_SPIDER);
    }
}