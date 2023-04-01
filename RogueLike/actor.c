//
//  actor.c
//  RogueLike
//
//  Created by Thomas Foster on 11/6/22.
//

#include "game.h"

#include "mathlib.h"
#include "texture.h"
#include "video.h"
#include "sound.h"

enum {
    DRAW_PRIORITY_NONE,
    DRAW_PRIORITY_ITEM,
    DRAW_PRIORITY_MONSTER,
    DRAW_PRIORITY_PLAYER,
};

// Sprite sheet location.
// Animated sprite frames are layed out horizontally.
// sprite_location is the first frame in an animation.
static const struct {
    u8 x;
    u8 y;
} sprite_cell[NUM_ACTOR_TYPES] = {
    [ACTOR_PLAYER]          = { 0, 0 },
    [ACTOR_TORCH]           = { 2, 3 },
    [ACTOR_BLOB]            = { 0, 2 },
    [ACTOR_ITEM_HEALTH]     = { 0, 1 },
    [ACTOR_ITEM_TURN]       = { 1, 1 },
    [ACTOR_GOLD_KEY]        = { 2, 1 },
    [ACTOR_BLOCK]           = { 4, 3 },
    [ACTOR_VASE]            = { 5, 0 },
    [ACTOR_CLOSED_CHEST]    = { 0, 3 },
    [ACTOR_OPEN_CHEST]      = { 1, 3 },
    [ACTOR_BLOCK_UP]        = { 0, 6 },
    [ACTOR_BLOCK_DOWN]      = { 1, 6 },
    [ACTOR_BUTTON_UP]       = { 2, 6 },
    [ACTOR_BUTTON_DOWN]     = { 1, 6 },
};

static SDL_Rect src = { .w = TILE_SIZE, .h = TILE_SIZE };



void C_Player(actor_t * player, actor_t * hit);
void C_Monster(actor_t * monster, actor_t * hit);
void C_Block(actor_t *, actor_t *);
void A_ChaseBasic(actor_t * blob);

#define ITEM_FLAGS { .collectible = true, .no_collision = true }

static actor_t templates[NUM_ACTOR_TYPES] = {
    [ACTOR_PLAYER] = {
        .flags = { .directional = true, .takes_damage = true },
        .num_frames = 2,
        .frame_msec = 500,
        .max_health = 10,
        .light = 255,
        .light_radius = 3,
        .contact = C_Player,
        .damage = 1,
        .draw_priority = DRAW_PRIORITY_PLAYER,
    },
    [ACTOR_TORCH] = {
        .num_frames = 2,
        .frame_msec = 300,
        .light = 255,
        .light_radius = 2,
    },
    [ACTOR_BLOB] = {
        .flags = { .takes_damage = true },
        .num_frames = 2,
        .frame_msec = 300,
        .max_health = 2,
        .action = A_ChaseBasic,
        .contact = C_Monster,
        .light_radius = 1,
        .light = 160,
        .damage = 1,
        .draw_priority = DRAW_PRIORITY_MONSTER,
    },
    [ACTOR_ITEM_HEALTH] = {
        .flags = ITEM_FLAGS,
        .draw_priority = DRAW_PRIORITY_ITEM,
    },
    [ACTOR_ITEM_TURN] = {
        .flags = ITEM_FLAGS,
        .draw_priority = DRAW_PRIORITY_ITEM,
    },
    [ACTOR_GOLD_KEY] = {
        .flags = ITEM_FLAGS,
        .draw_priority = DRAW_PRIORITY_ITEM,
    },
    [ACTOR_BLOCK] = {
        .contacted = C_Block,
    },
    [ACTOR_OPEN_CHEST] = {
        .flags = { .no_collision = true }
    },
    [ACTOR_BLOCK_UP] = {
        .flags = { .no_shadow = true },
    },
    [ACTOR_BLOCK_DOWN] = {
        .flags = { .no_collision = true, .no_shadow = true }
    },
    [ACTOR_BUTTON_UP] = {
        .flags = {
            .no_draw_offset = true,
            .no_shadow = true,
            .no_collision = true,
        },
    },
    [ACTOR_BUTTON_DOWN] = {
        .flags = {
            .no_collision = true,
            .no_shadow = true,
            .no_draw_offset = true,
        },
    },
};


void SpawnActor(game_t * game, actor_type_t type, tile_coord_t coord)
{
    actor_t actor = templates[type];
    actor.game = game;
    actor.type = type;
    actor.tile = coord;
    actor.health = actor.max_health;

    actors_t * actors = &game->map.actors;

    if ( actors->count + 1 <= MAX_ACTORS ) {
        actors->list[actors->count++] = actor;
    } else {
        printf("ran out of room in actor array!\n");
        return;
    }
}


void SpawnActorAtActor(actor_t * actor, actor_type_t type)
{
    SpawnActor(actor->game, type, actor->tile);
}


int DamageActor(actor_t * actor)
{
    actor->hit_timer = 1.0f;
//    actor->flags.was_attacked = true;

    if ( --actor->health == 0 ) {
        if ( actor->type != ACTOR_PLAYER ) {
            actor->flags.remove = true;
        } else {
            // TODO: player death
        }

        switch ( actor->type ) {
            case ACTOR_PLAYER:
                break;
            case ACTOR_BLOB: {
                if ( Chance(0.5) ) {
                    SpawnActorAtActor(actor, ACTOR_ITEM_HEALTH);
                } else {
                    SpawnActorAtActor(actor, ACTOR_ITEM_TURN);
                }
                break;
            }
            default:
                break;
        }
    }

    return actor->health;
}


actor_t * GetActorAtTile(actor_t * actors, int num_actors, tile_coord_t coord)
{
    for ( int i = 0; i < num_actors; i++ ) {
        if (   actors[i].tile.x == coord.x
            && actors[i].tile.y == coord.y ) {
            return &actors[i];
        }
    }

    return NULL;
}


void RenderActor(const actor_t * actor, int x, int y, int size, bool debug)
{
    SDL_Texture * actor_sheet = GetTexture("assets/actors.png");

    tile_t * tile = GetTile(&actor->game->map, actor->tile);
    if ( !debug ) {
        SDL_SetTextureColorMod(actor_sheet, tile->light, tile->light, tile->light);
    } else {
        SDL_SetTextureColorMod(actor_sheet, 255, 255, 255);
    }

    src.x = (sprite_cell[actor->type].x + actor->frame) * TILE_SIZE;
    src.y = sprite_cell[actor->type].y * TILE_SIZE;

    if ( actor->hit_timer > 0.0f ) {
        src.x += TILE_SIZE * actor->num_frames;
    }

    SDL_Rect dst = { x, y, size, size };

    // Draw actor's shadow
    if ( !actor->flags.no_shadow ) {
        SDL_Rect shadow_sprite_location = {
            .x = 4 * TILE_SIZE,
            .y = 0 * TILE_SIZE,
            .w = TILE_SIZE,
            .h = TILE_SIZE
        };
        V_DrawTexture(actor_sheet, &shadow_sprite_location, &dst);
    }

    // y position tweaks
    if ( actor->flags.floats ) {
        dst.y += (sinf(actor->game->ticks / 7) * SCALED(1)) - SCALED(6);
    } else {
        dst.y -= SCALED(3);
    }

    if ( actor->flags.directional && actor->flags.facing_left ) {
        V_DrawTextureFlip(actor_sheet, &src, &dst, SDL_FLIP_HORIZONTAL);
    } else {
        V_DrawTexture(actor_sheet, &src, &dst);
    }
}


void CastLight(game_t * game, const actor_t * actor)
{
    int r = actor->light_radius;

    if ( r == 0 ) {
        return;
    }

    for ( int y = actor->tile.y - r; y <= actor->tile.y + r; y++ ) {
        for ( int x = actor->tile.x - r; x <= actor->tile.x + r; x++ ) {
            tile_t * t = GetTile(&game->map, (tile_coord_t){ x, y });

            if ( t && ManhattenPathsAreClear(&game->map,
                                             actor->tile.x,
                                             actor->tile.y,
                                             x,
                                             y) )
            {
                int distance = DISTANCE(actor->tile.x, actor->tile.y, x, y);

                if ( distance <= r ) {
                    if ( t->light_target < actor->light ) {
                        t->light_target = actor->light;
                    }
                }
            }
        }
    }
}

void MoveActor(actor_t * actor, direction_t direction)
{
    actor->tile.x += XDelta(direction);
    actor->tile.y += YDelta(direction);

    SetUpMoveAnimation(actor, direction);
}


bool TryMoveActor(actor_t * actor, direction_t direction)
{
    tile_t * tile = GetAdjacentTile(&actor->game->map, actor->tile, direction);
    tile_coord_t try_coord = AdjacentTileCoord(actor->tile, direction);

    if ( tile->flags.blocking ) {
        return false;
    }

    // Tile is player-only.
    if ( actor->type != ACTOR_PLAYER && tile->flags.player_only ) {
        return false;
    }

    UpdateActorFacing(actor, XDelta(direction));

    actors_t * actors = &actor->game->map.actors;

    // Check if there's an actor at try_x, try_y
    for ( int i = 0; i < actors->count; i++ ) {
        actor_t * hit = &actors->list[i];

        if ( hit != actor
            && hit->tile.x == try_coord.x
            && hit->tile.y == try_coord.y )
        {
            // There's an actor on this spot:

            if ( actor->contact ) {
                actor->contact(actor, hit);
            }

            if ( hit->contacted ) {
                hit->contacted(hit, actor);
            }

            // Bump into it?
            if ( !hit->flags.no_collision ) {
                SetUpBumpAnimation(actor, direction);
                return false;
            }
        }
    }

    MoveActor(actor, direction);
    return true;
}

void UpdateActorFacing(actor_t * actor, int dx)
{
    if ( dx ) {
        actor->flags.facing_left = dx < 0;
    }
}
