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
#include "coord.h"
#include "direction.h"

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

#define INITIAL_TURNS 0

#define DIR_BIT(direction) (1 << direction)
#define FLAG(i) (1 << i)
#define HAS_FLAG(flags, flag) (flags & FLAG(flag) != 0)



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


typedef s8 tile_id_t;

typedef struct {
    u8 type; // a tile_type_t
    u8 variety; // A value that can be used for visual randomization.
    u8 num_variants; // Visual

    struct __attribute__((packed)) {
        unsigned blocking       : 1;
        unsigned player_only    : 1;
        unsigned room           : 1;
        unsigned visible        : 1;
        unsigned revealed       : 1;
    } flags;

    u8 light; // Current light level.
    u8 light_target; // Used to lerp (light -> light_target).

    s16 distance; // For pathfinding. Updated via UpdateDistanceMap()

    // Sprite sheet location.
    // Tiles with multiple visible varieties are layed out horizontally
    // and its location is the leftmost cell.
    struct { u8 x, y; } sprite_cell;
} tile_t;




#pragma mark - ITEMS

// Leveled items:
// basic: found in vases
// better: chests

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
    // Charge: move continuously until hit something
    // hover and examine monster's health and attack rating

    NUM_ITEMS,
} item_t;





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
    ACTOR_GOLD_KEY,

    NUM_ACTOR_TYPES
} actor_type_t;

typedef enum {
    // Actor can face left or right (sprite gets flipped per facing_left)
    ACTOR_DIRECTIONAL,
    ACTOR_TAKES_DAMAGE,
    ACTOR_BLOCKS_SIGHT,
    ACTOR_NO_BUMP, // Other actors can walk through.
    ACTOR_COLLECTIBLE, // Play can pick it up.
    ACTOR_FLOAT, // Hovers
} actor_flags_t;


typedef struct game game_t;
typedef struct actor actor_t;

struct actor {
    game_t * game;
    actor_type_t type;
    actor_flags_t flags;

//    int x;
//    int y;
    tile_coord_t tile;

    vec2_t offset_start;
    vec2_t offset_current;
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
    int damage;
    bool was_attacked;

    // An actor's light propogates to surrounding tiles.
    int light;
    int light_radius;

    bool remove;

    float hit_timer;

    void (* animation)(actor_t *, float move_timer);
    void (* contact)(actor_t * self, actor_t * other);
    void (* action)(actor_t *);
};




#pragma mark - GAME

typedef struct {
    int count;
    actor_t list[MAX_ACTORS];
} actors_t;

typedef struct {
    int width;
    int height;
    tile_t * tiles;
    tile_id_t * tile_ids;
    int num_rooms;
    SDL_Rect rooms[MAX_ROOMS];
    actors_t actors;
} map_t;

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

    // Player
    inventory_t inventory;
    bool has_gold_key;
    int player_turns;
    int level;

    char log[100];

    int state_timer;
    const game_state_t * state; // TODO: stack
};

bool InventoryIsEmtpy(const inventory_t * inventory);



#pragma mark - actor.c

/// Propogate actor's light to surrounding tiles by setting their `light_target`
/// value.
void CastLight(game_t * game, const actor_t * actor);
void SpawnActor(game_t * game,
                actors_t * actors,
                actor_type_t type,
                tile_coord_t coord);
void RenderActor(const actor_t * actor, int offset_x, int offset_y);
void MoveActor(actor_t * actor, direction_t direction);
bool TryMoveActor(actor_t * actor, direction_t direction);
int DamageActor(actor_t * actor);
actor_t * GetActorAtXY(actor_t * actors, int num_actors, int x, int y);
actor_t * GetPlayer(actors_t * actors);
const actor_t * GetPlayerReadOnly(const actor_t * actors, int num_actors);
void UpdateActorFacing(actor_t * actor, int dx);

#pragma mark - animation.c

void LevelTurnUpdate(game_t * game, float dt);
void AnimateActorMove(actor_t * actor, float move_timer);
void SetUpMoveAnimation(actor_t * actor, direction_t direction);
void SetUpBumpAnimation(actor_t * actor, direction_t direction);


#pragma mark - debug.c

extern bool show_map_gen;

void DebugWaitForKeyPress(void);
void CheckForShowMapGenCancel(void);


#pragma mark - gen.c

void GenerateDungeon(game_t * game, int width, int height);


#pragma mark - map.c

//extern const int x_deltas[NUM_DIRECTIONS];
//extern const int y_deltas[NUM_DIRECTIONS];

void DebugRenderTiles(map_t * map);
tile_t * GetAdjacentTile(map_t * map, tile_coord_t coord, direction_t direction);
tile_t * GetTile(map_t * map, tile_coord_t coord);
SDL_Point GetCoordinate(const map_t * map, int index);
box_t GetVisibleRegion(const map_t * map, const actor_t * player);
bool IsInBounds(const map_t * map, int x, int y);
bool LineOfSight(map_t * map, tile_coord_t t1, tile_coord_t t2, bool reveal);
void RenderMap(const game_t * game);
void UpdateDistanceMap(map_t * map, tile_coord_t coord, bool ignore_doors);
bool ManhattenPathsAreClear(map_t * map, int x0, int y0, int x1, int y1);
vec2_t GetRenderOffset(const actor_t * player);
void FreeDistanceMapQueue(void);
bool TileIsAdjacentTo(const map_t * map,
                      tile_coord_t coord,
                      tile_type_t type,
                      int num_directions);

#pragma mark - player.c

void PlayerCastSightLines(map_t * map);
void CollectItem(actor_t * player, actor_t * item_actor, item_t item);

#pragma mark - tile.c

tile_t CreateTile(tile_type_t type);

#endif /* main_h */
