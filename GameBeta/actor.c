//
//  actor.c
//  GameBeta
//
//  Created by Thomas Foster on 11/6/22.
//

#include "main.h"

#include "mathlib.h"
#include "texture.h"
#include "video.h"
#include "sound.h"

void C_Player(actor_t * player, actor_t * hit);
void C_Monster(actor_t * monster, actor_t * hit);
void A_Blob(actor_t * blob, game_t * game);

static actor_t templates[NUM_ACTOR_TYPES] = {
    [ACTOR_PLAYER] = {
        .flags = FLAG(ACTOR_DIRECTIONAL) | FLAG(ACTOR_TAKES_DAMAGE),
        .num_frames = 2,
        .frame_msec = 500,
        .max_health = 3,
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
        .flags = FLAG(ACTOR_TAKES_DAMAGE),
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
        .flags = FLAG(ACTOR_COLLECTIBLE) | FLAG(ACTOR_NO_BUMP),
        .sprite_cell = { 0, 1 },
    },
    [ACTOR_ITEM_TURN] = {
        .flags = FLAG(ACTOR_COLLECTIBLE) | FLAG(ACTOR_NO_BUMP),
        .sprite_cell = { 1, 1 },
    },
    [ACTOR_GOLD_KEY] = {
        .flags = FLAG(ACTOR_COLLECTIBLE) | FLAG(ACTOR_NO_BUMP),
        .sprite_cell = { 2, 1 },
    }
};

void SpawnActor(game_t * game, actor_type_t type, int x, int y)
{
    actor_t actor = templates[type];
    actor.game = game;
    actor.type = type;
    actor.x = x;
    actor.y = y;
    actor.health = actor.max_health;

    if ( game->num_actors + 1 <= MAX_ACTORS ) {
        game->actors[game->num_actors++] = actor;
    } else {
        printf("ran out of room in actor array!\n");
    }
}

int DamageActor(actor_t * actor)
{
    actor->hit_timer = 1.0f;
    actor->was_attacked = true;

    if ( --actor->health == 0 ) {
        if ( actor->type != ACTOR_PLAYER ) {
            actor->remove = true;
        }

        switch ( actor->type ) {
            case ACTOR_PLAYER:
                break;
            case ACTOR_BLOB:
                if ( Chance(0.5) ) {
                    SpawnActor(actor->game, ACTOR_ITEM_HEALTH, actor->x, actor->y);
                } else {
                    SpawnActor(actor->game, ACTOR_ITEM_TURN, actor->x, actor->y);
                }
                break;
            default:
                break;
        }
    }

    return actor->health;
}

actor_t * GetActorAtXY(actor_t * actors, int num_actors, int x, int y)
{
    for ( int i = 0; i < num_actors; i++ ) {
        if ( actors[i].x == x && actors[i].y == y ) {
            return &actors[i];
        }
    }

    return NULL;
}

actor_t * GetPlayer(actor_t * actors, int num_actors)
{
    for ( int i = 0; i < num_actors; i++ ) {
        if ( actors[i].type == ACTOR_PLAYER ) {
            return &actors[i];
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
    dst.x = (actor->x * RENDER_TILE_SIZE + actor->offset_current.x) - offset_x;
    dst.y = (actor->y * RENDER_TILE_SIZE + actor->offset_current.y) - offset_y;
    dst.w = RENDER_TILE_SIZE;
    dst.h = RENDER_TILE_SIZE;

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
    if ( actor->flags & FLAG(ACTOR_FLOAT) ) {
        dst.y += (sinf(actor->game->ticks / 7) * 5.0f) - 5;
    } else {
        dst.y -= 1 * TILE_SIZE;
    }
#endif

    if ( (actor->flags & FLAG(ACTOR_DIRECTIONAL)) && actor->facing_left ) {
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

    for ( int y = actor->y - r; y <= actor->y + r; y++ ) {
        for ( int x = actor->x - r; x <= actor->x + r; x++ ) {
            tile_t * t = GetTile(&game->map, x, y);

            if ( t && ManhattenPathsAreClear(&game->map,
                                             actor->x,
                                             actor->y,
                                             x,
                                             y) )
            {
                int distance = DISTANCE(actor->x, actor->y, x, y);

                if ( distance <= r ) {
                    t->light_target = actor->light;
                }
            }
        }
    }
}

void MoveActor(actor_t * actor, int dx, int dy)
{
    actor->x += dx;
    actor->y += dy;

    SetUpMoveAnimation(actor, dx, dy);
}

bool TryMoveActor(actor_t * actor, game_t * game, int dx, int dy)
{
    int try_x = actor->x + dx;
    int try_y = actor->y + dy;
    tile_t * tile = GetTile(&game->map, try_x, try_y);

    if ( actor->type != ACTOR_PLAYER && (tile->flags & FLAG(TILE_PLAYER_ONLY)) ) {
        return false;
    }

    if ( dx ) {
        actor->facing_left = dx < 0;
    }

    // Check if there's an actor at try_x, try_y
    for ( int i = 0; i < game->num_actors; i++ ) {
        actor_t * hit = &game->actors[i];

        if ( hit != actor && hit->x == try_x && hit->y == try_y ) {
            // There's an actor on this spot:

            if ( actor->contact ) {
                actor->contact(actor, hit);
            }

            // Bump into it?
            if ( !(hit->flags & FLAG(ACTOR_NO_BUMP)) ) {
                SetUpBumpAnimation(actor, dx, dy);
                return false;
            }

            break;
        }
    }

    MoveActor(actor, dx, dy);
    return true;
}
