//
//  actor.h
//  RogueLike
//
//  Created by Thomas Foster on 3/29/23.
//

#ifndef actor_h
#define actor_h

#define MAX_ACTORS (256 * 256)

typedef enum {
    ACTOR_PLAYER,
    ACTOR_TORCH,
    ACTOR_BLOB,
    ACTOR_SPIDER,
    ACTOR_ITEM_HEALTH,
    ACTOR_ITEM_TURN,
    ACTOR_GOLD_KEY,
    ACTOR_BLOCK,
    ACTOR_VASE,
    ACTOR_CLOSED_CHEST,
    ACTOR_OPEN_CHEST,
    ACTOR_SWORD,
    ACTOR_BLOCK_UP,
    ACTOR_BLOCK_DOWN,
    ACTOR_BUTTON_UP,
    ACTOR_BUTTON_DOWN,
    ACTOR_TREE,
    ACTOR_WELL,

    NUM_ACTOR_TYPES
} actor_type_t;


typedef struct actor_sprite {
    struct { u8 x, y; } cell;
    u8 height;
    s8 y_offset;
    u8 num_frames;
    u16 frame_msec;
    u8 draw_priority;
} actor_sprite_t;


typedef struct game game_t;
typedef struct actor actor_t;

struct actor {
    game_t * game;
    u8 type;

    struct {
        u8 directional      : 1; // Look at 'facing_left' for sprite flip.
        u8 takes_damage     : 1; // Can be hurt by other actors.
        u8 was_attacked     : 1;
        u8 has_target       : 1;
        u8 blocks_sight     : 1;
        u8 no_collision     : 1;
        u8 collectible      : 1; // Actors can walk through (no bump anim).
        u8 floats           : 1; // Hovers in the air.
        u8 facing_left      : 1;
        u8 no_shadow        : 1;
        u8 no_draw_offset   : 1;
        u8 on_teleporter    : 1;
        u8 remove           : 1; // Deleted
    } flags;

    tile_coord_t tile;

    vec2_t offset_start;
    vec2_t offset_current;

    u8 frame;
    actor_sprite_t * sprite;

//    u8 num_frames; // TODO: in sprite_info
//    u16 frame_msec; // TODO: in sprite_info
//    u8 draw_priority; // TODO: in actor.c static const array

    u8 max_health;
    s8 health;
    u8 damage;
    tile_coord_t target_tile;

    // An actor's light propogates to surrounding tiles.
    u8 light;
    u8 light_radius;

    float hit_timer;

    void (* animation)(actor_t *, float move_timer);
    void (* contact)(actor_t * self, actor_t * other);
    void (* contacted)(actor_t * self, actor_t * other); // When hit by something else.
    void (* action)(actor_t *);
};


/// Propogate actor's light to surrounding tiles by setting their `light_target`
/// value.
void CastLight(game_t * game, const actor_t * actor);
void SpawnActor(game_t * game, actor_type_t type, tile_coord_t coord);
void RenderActor(const actor_t * actor, int x, int y, int size, bool debug);
void MoveActor(actor_t * actor, direction_t direction);
bool TryMoveActor(actor_t * actor, direction_t direction);
int DamageActor(actor_t * actor);
actor_t * GetActorAtTile(actor_t * actors, int num_actors, tile_coord_t coord);
const actor_t * GetPlayerReadOnly(const actor_t * actors, int num_actors);
void UpdateActorFacing(actor_t * actor, int dx);
void Teleport(actor_t * actor, tile_coord_t from);

#endif /* actor_h */
