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
//#define GAME_WIDTH (160 * DRAW_SCALE)  // 20 tile wide
//#define GAME_HEIGHT (100 * DRAW_SCALE) // 12.5 tiles high
#define TILE_SIZE 8
#define RENDER_TILE_SIZE (TILE_SIZE * DRAW_SCALE)

// Must be an odd number!
#define MAP_WIDTH 31
#define MAP_HEIGHT 31

#define DEBUG_TILE_SIZE 16
#define MAX_ROOMS 200
#define MAX_ACTORS 200

#define INITIAL_TURNS 1

#define DIR_BIT(direction) (1 << direction)
#define FLAG(i) (1 << i)

typedef struct {
    SDL_Point min; // upper left
    SDL_Point max; // lower right
} box_t;

#pragma mark - TILE

typedef enum {
    TILE_FLOOR,
    TILE_WALL,
    TILE_DOOR,
    TILE_EXIT,
    TILE_GOLD_DOOR,
    TILE_START,
} tile_type_t;

typedef enum {
    TILE_BLOCKING,
    TILE_PLAYER_ONLY, // Only the player can walk here
} tile_flags_t;

typedef struct {
    u8 type; // a tile_type_t
    u8 variety; // A value that can be used for visual randomization.
    u8 num_variants; // Visual
    u8 flags;

    bool visible; // Player can currently see it.
    bool revealed; // Player has seen it before.

    int light; // Current light level.
    int light_target; // Used to lerp (light -> light_target).

    int distance; // For pathfinding. Updated via UpdateDistanceMap()

    // Sprite sheet location.
    // Tiles with multiple visible varieties are layed out horizontally
    // and its location is the leftmost cell.
    struct {
        int x;
        int y;
    } sprite_cell;

} tile_t;

typedef tile_t tiles_t[MAP_WIDTH][MAP_HEIGHT];

#pragma mark - ITEMS

typedef enum {
    ITEM_HEALTH,    // +1 health
    ITEM_TURN,      // +1 turn

    // Sword(s) wooden, steel, obsidian
    // Shield(s): 25% chance to block
    // - wooden 100% chance to break
    // - steel 75% chance to break
    // - obsidian 50% chance to break
    // Stone of returning: drop and activate to return to stone.
    // Gem that allows you to go back a level (starting pad turns purple)

    NUM_ITEMS,
} item_type_t;

#pragma mark - ACTOR

// When adding a new actor:
// ------------------------
// 1. Add to enum below, somewhere above 'NUM_ACTOR_TYPES'
// 2. Add a template in actor.c
// 3. Optionally, add a contact function in contact.c. Declare it at the
//    top of actor.c and add it to template.
// 4. Optionally, add an action function in action.c. Declare it at the
//    top of actior.c and add it to the template.
typedef enum {
    ACTOR_PLAYER,
    ACTOR_TORCH,
    ACTOR_BLOB,
    ACTOR_ITEM_HEALTH,
    ACTOR_ITEM_TURN,

    NUM_ACTOR_TYPES
} actor_type_t;

typedef enum {
    // Actor can face left or right (sprite gets flipped per facing_left)
    ACTOR_DIRECTIONAL,
    ACTOR_TAKES_DAMAGE,
    ACTOR_BLOCKS_SIGHT,
    ACTOR_NO_BUMP, // Other actors can walk through.
    ACTOR_COLLECTIBLE, // Play can pick it up.
} actor_flags_t;

typedef struct game game_t;
typedef struct actor actor_t;

struct actor {
    game_t * game;
    actor_type_t type;
    actor_flags_t flags;

    int x;
    int y;
    vec2_t offset_start;
    vec2_t offset_current;
    int y_draw_offset;
    bool facing_left;

    int frame;
    int num_frames;
    int frame_msec;

    // Sprite sheet location.
    // Animated sprite frames are layed out horizontally.
    // sprite_location is the first frame in an animation.
    struct {
        int x;
        int y;
    } sprite_cell;

    int max_health;
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

#define NUM_CARDINAL_DIRECTIONS 4

typedef enum {
    NO_DIRECTION = -1,
    NORTH,
    WEST,
    EAST,
    SOUTH,
    NORTH_WEST,
    NORTH_EAST,
    SOUTH_WEST,
    SOUTH_EAST,
    NUM_DIRECTIONS,
} direction_t;

typedef struct game_state {
    bool (* process_input)(game_t *, const SDL_Event *);
    void (* update)(game_t *, float dt);
    void (* render)(const game_t *);

    void (* on_enter)(game_t *);
    void (* on_exit)(game_t *);

    // If state has a finite length, next_state must not be NULL.
    // -1 == indefinite length.
    int duration_ticks;
    const struct game_state * next_state;
} game_state_t;

typedef struct {
    int item_counts[NUM_ITEMS];
    int selected_item;
} inventory_t;

struct game {
    bool is_running;
    int ticks;
    map_t map;
    float move_timer;

    vec2_t camera;
    int num_actors;
    actor_t actors[MAX_ACTORS];

    inventory_t inventory;
    bool has_gold_key;

    char log[100];

    int player_turns;
    bool exiting_level;
    int level;

    game_state_t state;
};

bool InventoryIsEmtpy(const inventory_t * inventory);

#pragma mark - actor.c

/// Propogate actor's light to surrounding tiles by setting their `light_target`
/// value.
void CastLight(game_t * game, const actor_t * actor, tiles_t tiles);
void SpawnActor(game_t * game, actor_type_t type, int x, int y);
void RenderActor(const actor_t * actor, int offset_x, int offset_y);
void MoveActor(actor_t * actor, int dx, int dy);
bool TryMoveActor(actor_t * actor, game_t * game, int dx, int dy);
int DamageActor(actor_t * actor);
actor_t * GetActorAtXY(actor_t * actors, int num_actors, int x, int y);
actor_t * GetPlayer(actor_t * actors, int num_actors);
const actor_t * GetPlayerReadOnly(const actor_t * actors, int num_actors);

#pragma mark - animation.c

void LevelTurnUpdate(game_t * game, float dt);
void AnimateActorMove(actor_t * actor, float move_timer);
void SetUpMoveAnimation(actor_t * actor, int dx, int dy);
void SetUpBumpAnimation(actor_t * actor, int dx, int dy);


#pragma mark - debug.c

extern bool show_map_gen;

void DebugWaitForKeyPress(void);
void CheckForShowMapGenCancel(void);


#pragma mark - gen.c

void GenerateMap(game_t * game);


#pragma mark - map.c

extern const int x_deltas[NUM_DIRECTIONS];
extern const int y_deltas[NUM_DIRECTIONS];

void DebugRenderTiles(tiles_t tiles);
tile_t * GetAdjacentTile(tiles_t tiles, int x, int y, direction_t direction);
box_t GetVisibleRegion(const actor_t * player);
bool IsInBounds(int x, int y);
bool LineOfSight(game_t * game, int x1, int y1, int x2, int y2, bool reveal);
void RenderMap(const game_t * game);
void UpdateDistanceMap(tiles_t tiles, int x, int y);
bool ManhattenPathsAreClear(map_t map, int x0, int y0, int x1, int y1);
vec2_t GetRenderOffset(const actor_t * player);

#pragma mark - player.c

void PlayerCastSightLines(game_t * game, const actor_t * player);
void CollectItem(actor_t * player, actor_t * item_actor, item_type_t item);

#pragma mark - tile.c

tile_t CreateTile(tile_type_t type);

#endif /* main_h */
