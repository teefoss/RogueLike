//
//  astar.c
//  RogueLike
//
//  Created by Thomas Foster on 5/18/23.
//

#include "astar.h"

typedef struct {
    TileCoord parent;
    float cost;
    float heuristic;
    bool visited;
    bool blocked;
} Node;

Node * grid;
int grid_size;
int grid_width;
TileCoord * open_list;

static float Heuristic(TileCoord start, TileCoord end)
{
    // Calculate the Manhattan distance between two points
    return abs(end.x - start.x) + abs(end.y - start.y);
}

#define NODE(coord) grid[coord.y * grid_width + coord.x]

Path FindPath(World * world, TileCoord start, TileCoord end, bool diagonal)
{
    Map * map = world->map;
    Path path;
    path.size = 0;

    grid_width = map->width;

    int size_needed = map->width * map->height;
    if ( size_needed > grid_size ) {
        size_t new_size = size_needed * sizeof(*grid);
        if ( grid == NULL ) {
            grid = malloc(new_size);
        } else {
            grid = realloc(grid, new_size);
        }

        if ( open_list == NULL ) {
            open_list = malloc(size_needed * sizeof(*open_list));
        } else {
            open_list = realloc(open_list, sizeof(*open_list));
        }

        ASSERT(grid != NULL);
        ASSERT(open_list != NULL);
        grid_size = size_needed;
    }

    for ( int i = 0; i < grid_size; i++ ) {
        grid[i].parent.x = -1;
        grid[i].parent.y = -1;
        grid[i].cost = 0.0f;
        grid[i].heuristic = 0.0f;
        grid[i].visited = false;
        grid[i].blocked = false;
    }

    FOR_EACH_ACTOR(actor, world->map->actor_list) {
        if ( ActorBlocksAll(actor) ) {
            grid[actor->tile.y * map->width + actor->tile.x].blocked = true;
        }
    }

    NODE(start).cost = 0.0f;
    NODE(start).heuristic = Heuristic(start, end);
    NODE(start).visited = true;

    int open_list_size = 1;
    open_list[0] = start;

    while ( open_list_size > 0 ) {
        // Find the tile with the lowest cost in the open list
        int index = 0;
        float lowest_cost = NODE(open_list[0]).cost + NODE(open_list[0]).heuristic;
        for ( int i = 1; i < open_list_size; i++ ) {
            float cost = NODE(open_list[i]).cost + NODE(open_list[i]).heuristic;
            if ( cost < lowest_cost ) {
                lowest_cost = cost;
                index = i;
            }
        }

        TileCoord current = open_list[index];

        // Remove the current tile from the open list
        open_list[index] = open_list[--open_list_size];

        if ( current.x == end.x && current.y == end.y ) {
            // Goal reached, construct path
            TileCoord current_coord = current;

            while ( current_coord.x != -1 && current_coord.y != -1 ) {
                path.coords[path.size++] = current_coord;
                if ( path.size == PATH_MAX_COORDS) {
                    break;
                }
                current_coord = NODE(current_coord).parent;
            }

            return path;
        }

        TileCoord neighbors[8] = {
            { current.x - 1, current.y },
            { current.x + 1, current.y },
            { current.x, current.y - 1 },
            { current.x, current.y + 1 },
            { current.x - 1, current.y - 1 },
            { current.x + 1, current.y - 1 },
            { current.x - 1, current.y + 1 },
            { current.x + 1, current.y + 1 }
        };

        int num_directions = diagonal ? 8 : 4;
        for ( int i = 0; i < num_directions; i++ ) {
            TileCoord neighbor = neighbors[i];
            Tile * tile = GetTile(map, neighbor);

            if ( tile->flags.blocks_movement ) continue;
            if ( !IsInBounds(map, neighbor.x, neighbor.y) ) continue;
            if ( NODE(neighbor).visited ) continue;
            if ( NODE(neighbor).blocked ) continue;

            // Visit this tile:

            float neighbor_cost = NODE(current).cost + 1.0f;

            NODE(neighbor).parent = current;
            NODE(neighbor).cost = neighbor_cost;
            NODE(neighbor).heuristic = Heuristic(neighbor, end);
            NODE(neighbor).visited = true;

            open_list[open_list_size++] = neighbor;
        }
    }

    // No path found.
    return path;
}
