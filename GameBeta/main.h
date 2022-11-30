//
//  main.h
//  GameBeta
//
//  Created by Thomas Foster on 11/4/22.
//

#ifndef main_h
#define main_h

#include "vector.h"
#include "inttypes.h"

#include <SDL_rect.h>
#include <SDL_events.h>
#include <stdbool.h>

#define FPS 30.0f
#define DRAW_SCALE 4
#define GAME_WIDTH (256 * DRAW_SCALE)  // 32 tile wide
#define GAME_HEIGHT (144 * DRAW_SCALE) // 18 tiles high
#define TILE_SIZE 8
#define RENDER_TILE_SIZE (TILE_SIZE * DRAW_SCALE)

#define MAP_WIDTH 51
#define MAP_HEIGHT 51
#define DEBUG_TILE_SIZE 16
#define MAX_ROOMS 200
#define MAX_ACTORS 200

#define INITIAL_TURNS 2

#define DIR_BIT(direction) (1 << direction)

typedef struct {
    SDL_Point min; // upper left
    SDL_Point max; // lower right
} box_t;

#pragma mark - TILE

typedef struct {
    u8 type; // a tile_type_t
    u8 variety; // A value that can be used for visual randomization.

    bool visible; // Player can currently see it.
    bool revealed; // Player has seen it before.

    int light; // Current light level.
    int light_target; // Used to lerp (light -> light_target).

    int distance; // For pathfinding. Updated via UpdateDistanceMap()
} tile_t;

typedef tile_t tiles_t[MAP_WIDTH][MAP_HEIGHT];

typedef enum {
    TILE_FLOOR,
    TILE_WALL,
} tile_type_t;

#pragma mark - ACTOR

// When adding a new actor:
// ------------------------
// 1. Add to enum below, somewhere above 'NUM_ACTOR_TYPES'
// 2. Add a template in actor.c
// 3. Optionally, add a contact function in contact.c. Declare it at the
//    top of actor.c and add it to template.
// 4. In actor.c, add sprite sheet location to the switch statement in
//    RenderActor(). Use the location of the first animation frame.
// 5. Optionally, add an action function in action.c. Declare it at the
//    top of actior.c and add it to the template.
typedef enum {
    ACTOR_PLAYER,
    ACTOR_TORCH,
    ACTOR_BLOB,

    NUM_ACTOR_TYPES
} actor_type_t;

typedef struct game game_t;
typedef struct actor actor_t;

struct actor {
    actor_type_t type;

    int x;
    int y;
    vec2_t offsets[2]; // 0 = start, 1 = current
    bool facing_left;

    int frame;
    int num_frames;
    int frame_msec;

    int health;

    // An actor's light propogates to surrounding tiles.
    int light;
    int light_radius;

    bool remove;

    float hit_timer;

    void (* animation)(actor_t *, float move_timer);
    void (* contact)(actor_t * self, actor_t * other);
    void (* action)(actor_t *, game_t *);
};

#pragma mark - GAME

typedef struct {
    tiles_t tiles;

    int num_rooms;
    SDL_Rect rooms[MAX_ROOMS];
} map_t;

typedef enum {
    NO_DIRECTION = -1,
    NORTH,
    SOUTH,
    WEST,
    EAST,
    NORTH_WEST,
    NORTH_EAST,
    SOUTH_WEST,
    SOUTH_EAST,
    NUM_DIRECTIONS,
} direction_t;

struct game {
    bool is_running;
    int ticks;
    map_t map;
    float move_timer;
    int num_actors;
    actor_t actors[MAX_ACTORS];
    int player_turns;

    bool (* do_input)(game_t *, const SDL_Event *);
    void (* update)(game_t *, float dt);
};


#pragma mark - actor.c

/// Propogate actor's light to surrounding tiles by setting their `light_target`
/// value.
void CastLight(const actor_t * actor, tiles_t tiles);
void SpawnActor(game_t * game, actor_type_t type, int x, int y);
void RenderActor(const actor_t * actor, int offset_x, int offset_y);
bool TryMoveActor(actor_t * actor, game_t * game, int dx, int dy);


#pragma mark - animation.c

void GameUpdateActorAnimations(game_t * game, float dt);
void AnimateActorMove(actor_t * actor, float move_timer);
void SetUpBumpAnimation(actor_t * actor, int dx, int dy);


#pragma mark - gen.c

void GenerateMap(game_t * game);


#pragma mark - map.c

extern const int x_dirs[NUM_DIRECTIONS];
extern const int y_dirs[NUM_DIRECTIONS];

void DebugRenderMap(const game_t * game);
tile_t * GetAdjacentTile(tiles_t tiles, int x, int y, direction_t direction);
box_t GetVisibleRegion(const actor_t * player);
bool IsInBounds(int x, int y);
bool LineOfSight(tiles_t tiles, int x1, int y1, int x2, int y2, bool reveal);
void RenderMap(const game_t * game);
void UpdateDistanceMap(tiles_t tiles, int x, int y);


#pragma mark - player.c

void PlayerCastSightLines(map_t * map, const actor_t * player);

#endif /* main_h */
