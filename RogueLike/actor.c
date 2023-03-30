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

void C_Player(actor_t * player, actor_t * hit);
void C_Monster(actor_t * monster, actor_t * hit);
void C_Block(actor_t *, actor_t *);
void A_Blob(actor_t * blob);

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
        .sprite_cell = { 0, 0 },
        .damage = 1,
    },
    [ACTOR_TORCH] = {
        .num_frames = 2,
        .frame_msec = 300,
        .light = 255,
        .light_radius = 2,
        .sprite_cell = { 2, 3 },
    },
    [ACTOR_BLOB] = {
        .flags = { .takes_damage = true },
        .num_frames = 2,
        .frame_msec = 300,
        .max_health = 2,
        .action = A_Blob,
        .contact = C_Monster,
        .light_radius = 1,
        .light = 160,
        .sprite_cell = { 0, 2 },
        .damage = 1,
    },
    [ACTOR_ITEM_HEALTH] = {
        .flags = ITEM_FLAGS,
        .sprite_cell = { 0, 1 },
    },
    [ACTOR_ITEM_TURN] = {
        .flags = ITEM_FLAGS,
        .sprite_cell = { 1, 1 },
    },
    [ACTOR_GOLD_KEY] = {
        .flags = ITEM_FLAGS,
        .sprite_cell = { 2, 1 },
    },
    [ACTOR_BLOCK] = {
        .sprite_cell = { 4, 3 },
        .contacted = C_Block,
    },
    [ACTOR_VASE] = {
        .sprite_cell = { 5, 0 },
        .max_health = 1,
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


actor_t * GetPlayer(actors_t * actors)
{
    for ( int i = 0; i < actors->count; i++ ) {
        if ( actors->list[i].type == ACTOR_PLAYER ) {
            return &actors->list[i];
        }
    }

    return NULL;
}

const actor_t * GetPlayerReadOnly(const actor_t * actors, int num_actors)
{
    for ( int i = 0; i < num_actors; i++ ) {
        if ( actors[i].type == ACTOR_PLAYER ) {
            return &actors[i];
        }
    }

    return NULL;
}

void RenderActor(const actor_t * actor, int offset_x, int offset_y)
{
    SDL_Texture * actor_sheet = GetTexture("assets/actors.png");

    SDL_Rect src;
    src.w = TILE_SIZE;
    src.h = TILE_SIZE;

    SDL_Rect dst;
    dst.x = (actor->tile.x * SCALED(TILE_SIZE) + actor->offset_current.x) - offset_x;
    dst.y = (actor->tile.y * SCALED(TILE_SIZE) + actor->offset_current.y) - offset_y;
    dst.y -= DRAW_SCALE * 1;
    dst.w = SCALED(TILE_SIZE);
    dst.h = SCALED(TILE_SIZE);

    src.x = actor->sprite_cell.x * TILE_SIZE;
    src.y = actor->sprite_cell.y * TILE_SIZE;

    // Select animation frame.
    if ( actor->num_frames > 1 ) {
        src.x += TILE_SIZE * actor->frame;
    }

    // Select damage sprite?
    if ( actor->hit_timer > 0.0f ) {
        src.x += TILE_SIZE * actor->num_frames;
    }

#if 1
    // Draw actor's shadow
    SDL_Rect shadow_sprite_location = {
        .x = 4 * TILE_SIZE,
        .y = 0 * TILE_SIZE,
        .w = TILE_SIZE,
        .h = TILE_SIZE
    };
    V_DrawTexture(actor_sheet, &shadow_sprite_location, &dst);

    // Tweak its y position
    if ( actor->flags.floats ) {
        dst.y += (sinf(actor->game->ticks / 7) * 5.0f) - 5;
    } else {
        dst.y -= 1 * TILE_SIZE;
    }
#endif

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
                    t->light_target = actor->light;
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

            break;
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
