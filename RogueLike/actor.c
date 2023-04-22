//
//  actor.c
//  RogueLike
//
//  Created by Thomas Foster on 11/6/22.
//

#include "game.h"
#include "world.h"

#include "mathlib.h"
#include "texture.h"
#include "video.h"
#include "sound.h"

#include <limits.h>

enum {
    DRAW_PRIORITY_NONE,
    DRAW_PRIORITY_ITEM,
    DRAW_PRIORITY_KEY,
    DRAW_PRIORITY_MONSTER,
    DRAW_PRIORITY_PLAYER,
};


// Animated sprite frames are layed out horizontally.
// `cell` is the first frame in an animation.
ActorSprite sprite_info[NUM_ACTOR_TYPES] = {
    [ACTOR_PLAYER] = {
        .cell = { 0, 0 },
        .num_frames = 2,
        .frame_msec = 500,
        .draw_priority = DRAW_PRIORITY_PLAYER,
    },
    [ACTOR_TORCH] = {
        .cell = { 2, 3 },
        .num_frames = 2,
        .frame_msec = 300,
    },
    [ACTOR_BLOB] = {
        .cell = { 0, 2 },
        .num_frames = 2,
        .frame_msec = 300,
        .draw_priority = DRAW_PRIORITY_MONSTER,
    },
    [ACTOR_SPIDER] = {
        .cell = { 4, 2 },
        .num_frames = 4,
        .frame_msec = 100,
        .draw_priority = DRAW_PRIORITY_MONSTER,
    },
    [ACTOR_ITEM_HEALTH] = {
        .cell = { 0, 1 },
        .draw_priority = DRAW_PRIORITY_ITEM,
    },
    [ACTOR_ITEM_TURN] = {
        .cell = { 1, 1 },
        .draw_priority = DRAW_PRIORITY_ITEM,
    },
    [ACTOR_GOLD_KEY] = {
        .cell = { 2, 1 },
        .draw_priority = DRAW_PRIORITY_KEY,
    },
    [ACTOR_BLOCK] = {
        .cell = { 4, 3 }
    },
    [ACTOR_VASE] = {
        .cell = { 5, 0 }
    },
    [ACTOR_CLOSED_CHEST] = {
        .cell = { 0, 3 }
    },
    [ACTOR_OPEN_CHEST] = {
        .cell = { 1, 3 }
    },
    [ACTOR_BLOCK_UP] = {
        .cell = { 0, 6 }
    },
    [ACTOR_BLOCK_DOWN] = {
        .cell = { 1, 6 }
    },
    [ACTOR_BUTTON_UP] = {
        .cell = { 2, 6 }
    },
    [ACTOR_BUTTON_DOWN] = {
        .cell = { 1, 6 }
    },
    [ACTOR_TREE] = {
        .cell = { 0, 7 }
    },
    [ACTOR_WELL] = {
        .cell = { 3, 6 },
        .height = 2,
        .y_offset = -1,
    }
};


void C_Player(Actor * player, Actor * hit);
void C_Monster(Actor * monster, Actor * hit);
void C_Spider(Actor * spider, Actor * hit);
void C_Block(Actor *, Actor *);

void A_TargetAndChasePlayerIfVisible(Actor *);
void A_ChasePlayerIfVisible(Actor *);
void A_StupidChasePlayerIfVisible(Actor * actor);

#define ITEM_FLAGS { .collectible = true, .no_collision = true }

static Actor templates[NUM_ACTOR_TYPES] = {
    [ACTOR_PLAYER] = {
        .flags = { .directional = true, .takes_damage = true },
        .max_health = 10,
        .light = 255,
        .light_radius = 3,
        .contact = C_Player,
        .damage = 1,
    },
    [ACTOR_TORCH] = {
        .light = 255,
        .light_radius = 2,
    },
    [ACTOR_BLOB] = {
        .flags = { .takes_damage = true },
        .max_health = 2,
        .action = A_TargetAndChasePlayerIfVisible,
        .contact = C_Monster,
        .light_radius = 1,
        .light = 160,
        .damage = 1,
    },
    [ACTOR_SPIDER] = {
        .flags = { .takes_damage = true },
        .max_health = 1,
        .action = A_StupidChasePlayerIfVisible,
        .contact = C_Spider,
        .damage = 1,
    },
    [ACTOR_ITEM_HEALTH] = {
        .flags = ITEM_FLAGS,
    },
    [ACTOR_ITEM_TURN] = {
        .flags = ITEM_FLAGS,
    },
    [ACTOR_GOLD_KEY] = {
        .flags = ITEM_FLAGS,
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
    [ACTOR_WELL] = {
        .flags = { .no_shadow = true, .no_draw_offset = true },
    }
};


void SpawnActor(Game * game, ActorType type, TileCoord coord)
{
    Actor actor = templates[type];
    actor.game = game;
    actor.type = type;
    actor.sprite = &sprite_info[type];
    actor.tile = coord;
    actor.health = actor.max_health;

    if ( game->world.actors.count + 1 <= MAX_ACTORS ) {
        game->world.actors.list[game->world.actors.count++] = actor;
    } else {
        printf("ran out of room in actor array!\n");
        return;
    }
}


void SpawnActorAtActor(Actor * actor, ActorType type)
{
    SpawnActor(actor->game, type, actor->tile);
}


void SpawnParticlesAtActor(ParticleArray * array, Actor * actor, SDL_Color color)
{
    SDL_Point position = {
        .x = actor->tile.x * TILE_SIZE,
        .y = actor->tile.y * TILE_SIZE,
    };

    for ( int i = 0; i < 30; i++ ) {
        Particle p;
        p.position.x = Random(position.x, position.x + TILE_SIZE);
        p.position.y = Random(position.y, position.y + TILE_SIZE);
        p.velocity = RandomVelocity(10.0f, 20.0f);
        p.color = color;
        p.lifespan = Random(MS2TICKS(200, FPS), MS2TICKS(400, FPS));
        InsertParticle(&actor->game->world.particles, p);
    }
}


void KillActor(Actor * actor)
{
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
            SDL_Color green = { 0, 255, 0, 255 }; // TODO: Palette
            SpawnParticlesAtActor(&actor->game->world.particles, actor, green);

            break;
        }
        case ACTOR_SPIDER: {
            if ( Chance(0.2) ) {
                SpawnActorAtActor(actor, ACTOR_ITEM_TURN);
            }
            SDL_Color black = { 0, 0, 0, 255 }; // TODO: Palette
            SpawnParticlesAtActor(&actor->game->world.particles, actor, black);
            break;
        }
        default:
            break;
    }
}


int DamageActor(Actor * actor)
{
    actor->hit_timer = 1.0f;
    actor->flags.was_attacked = true;

    if ( --actor->health == 0 ) {
        KillActor(actor);
    }

    return actor->health;
}


Actor * GetActorAtTile(Actors * actors, TileCoord coord)
{
    for ( int i = 0; i < actors->count; i++ ) {
        if (   actors->list[i].tile.x == coord.x
            && actors->list[i].tile.y == coord.y ) {
            return &actors->list[i];
        }
    }

    return NULL;
}


void RenderActor(const Actor * actor, int x, int y, int size, bool debug, int game_ticks)
{
    SDL_Texture * actor_sheet = GetTexture("assets/actors.png");

    Tile * tile = GetTile(&actor->game->world.map, actor->tile);
    if ( !debug ) {
        SDL_SetTextureColorMod(actor_sheet, tile->light, tile->light, tile->light);
    } else {
        SDL_SetTextureColorMod(actor_sheet, 255, 255, 255);
    }

    SDL_Rect src;
    src.x = (actor->sprite->cell.x + actor->frame) * TILE_SIZE;
    src.y = actor->sprite->cell.y * TILE_SIZE;
    src.w = TILE_SIZE;

    // "Damaged" sprite?
    if ( actor->hit_timer > 0.0f ) {
        src.x += TILE_SIZE * actor->sprite->num_frames;
    }

    SDL_Rect dst;
    dst.x = x;
    dst.y = y + actor->sprite->y_offset * size;
    dst.w = size;

    // Adjust height.
    if ( actor->sprite->height ) {
        src.h = actor->sprite->height * TILE_SIZE;
        dst.h = actor->sprite->height * size;
    } else {
        src.h = TILE_SIZE;
        dst.h = size;
    }

    // Draw actor's shadow
    // TODO: adjust shadow for tall sprites
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
    // TODO: adjust for tall sprites
    if ( actor->flags.floats ) {
        dst.y += (sinf(game_ticks / 7) * SCALED(1)) - SCALED(6);
    } else {
        if ( !debug ) {
            dst.y -= SCALED(3);
        }
    }

    if ( actor->flags.directional && actor->flags.facing_left ) {
        V_DrawTextureFlip(actor_sheet, &src, &dst, SDL_FLIP_HORIZONTAL);
    } else {
        V_DrawTexture(actor_sheet, &src, &dst);
    }
}


void CastLight(World * world, const Actor * actor)
{
    int r = actor->light_radius;

    if ( r == 0 ) {
        return;
    }

    TileCoord coord;
    for ( coord.y = actor->tile.y - r; coord.y <= actor->tile.y + r; coord.y++ ) {
        for ( coord.x = actor->tile.x - r; coord.x <= actor->tile.x + r; coord.x++ ) {
            Tile * t = GetTile(&world->map, coord);

//            if ( t && LineOfSight(&game->map, actor->tile, coord, false))
            if ( t && ManhattenPathsAreClear(&world->map,
                                             actor->tile.x,
                                             actor->tile.y,
                                             coord.x,
                                             coord.y) )
            {
                if ( TileDistance(actor->tile, coord) <= r ) {
                    if ( t->flags.revealed && actor->light > t->light ) {
                        t->light = actor->light;
                    }
                }
            }
        }
    }
}

void MoveActor(Actor * actor, Direction direction)
{
    actor->tile.x += XDelta(direction);
    actor->tile.y += YDelta(direction);

    SetUpMoveAnimation(actor, direction);
}


bool TryMoveActor(Actor * actor, Direction direction)
{
    Tile * tile = GetAdjacentTile(&actor->game->world.map, actor->tile, direction);
    TileCoord try_coord = AdjacentTileCoord(actor->tile, direction);

    if ( tile->flags.blocks_movement ) {
        return false;
    }

    // Tile is player-only.
    if ( actor->type != ACTOR_PLAYER && tile->flags.player_only ) {
        return false;
    }

    UpdateActorFacing(actor, XDelta(direction));

    Actors * actors = &actor->game->world.actors;

    // Check if there's an actor at try_x, try_y
    for ( int i = 0; i < actors->count; i++ ) {
        Actor * hit = &actors->list[i];

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

void UpdateActorFacing(Actor * actor, int dx)
{
    if ( dx ) {
        actor->flags.facing_left = dx < 0;
    }
}


// TODO: telefrag?
void Teleport(Actor * actor, TileCoord from)
{
    Map * map = &actor->game->world.map;

    int min_distance = INT_MAX;
    TileCoord to = actor->tile;

    // Find the nearest teleport.
    for ( int i = 0; i < map->width * map->height; i++ ) {
        if ( map->tiles[i].type == TILE_TELEPORTER ) {
            TileCoord coord = GetCoordinate(map, i);

            int distance = DISTANCE(coord.x, coord.y, from.x, from.y);
            if ( distance != 0 && distance < min_distance ) {
                min_distance = distance;
                to = coord;
            }
        }
    }

    actor->tile = to;
}
