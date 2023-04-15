//
//  contact.c
//  RogueLike
//
//  Created by Thomas Foster on 11/8/22.
//

#include "game.h"
#include "mylib/sound.h"

void C_Player(Actor * player, Actor * hit)
{
    if ( hit->flags.takes_damage ) {
        if ( DamageActor(hit) == 0 ) {
            S_Play("t160 l32 o3 e c+ c < a-");
        } else {
            S_Play("t160 l32 o3 e c+ c");
        }
    }

    switch ( hit->type ) {
        case ACTOR_ITEM_HEALTH:
            if ( CollectItem(player, hit, ITEM_HEALTH) ) {
                strncpy(player->game->log,
                        "Picked up a Health Potion!",
                        sizeof(player->game->log));
                S_Play("l32 o2 e b g+ > d+");
            }
            break;
        case ACTOR_ITEM_TURN:
            if ( CollectItem(player, hit, ITEM_TURN) ) {
                strncpy(player->game->log,
                        "Picked up a Turn Potion!",
                        sizeof(player->game->log));
                S_Play("l32 o2 a b > f+");
            }
            break;
        case ACTOR_GOLD_KEY:
            strncpy(player->game->log, "Got the golden key!", sizeof(player->game->log));
            player->game->has_gold_key = true;
            hit->flags.remove = true;
            S_Play("l32 t100 o1 a > a > a");
            break;
        case ACTOR_VASE:
            S_Play("o5 t90 l64 b d g+ e- c f b-");
            SpawnActor(player->game, ACTOR_ITEM_HEALTH, hit->tile);
            hit->flags.remove = true;
            break;
        case ACTOR_CLOSED_CHEST:
            SpawnActor(player->game, ACTOR_ITEM_HEALTH, hit->tile);
            SpawnActor(player->game, ACTOR_OPEN_CHEST, hit->tile);
            hit->flags.remove = true;
            S_Play("o3 t100 l32 f c b-");
            break;
        case ACTOR_BUTTON_UP:
            for ( int i = 0; i < player->game->map.actors.count; i++ ) {
                Actor * a = &player->game->map.actors.list[i];
                if ( a->type == ACTOR_BLOCK_UP ) {
                    a->flags.remove = true;
                    SpawnActor(player->game, ACTOR_BLOCK_DOWN, a->tile);
                }
            }
            hit->flags.remove = true;
            SpawnActor(player->game, ACTOR_BUTTON_DOWN, hit->tile);
            S_Play("o1 t90 l32 a c+");
            break;
        default:
            break;
    }
}


void C_Monster(Actor * monster, Actor * hit)
{
    // Monsters can damage other monsters, but not of the same type
    if ( hit->flags.takes_damage && monster->type != hit->type ) {
        DamageActor(hit);
        S_Play("o3 l32 b c < c+");
    }
}


void C_Spider(Actor * spider, Actor * hit)
{
    if ( hit->flags.takes_damage ) {
        DamageActor(hit);
        S_Play("o4 l32 f b c+");
    }
}

void C_Block(Actor * block, Actor * pusher)
{
    int dx = block->tile.x - pusher->tile.x;
    int dy = block->tile.y - pusher->tile.y;

    if ( TryMoveActor(block, GetDirection(dx, dy)) ) {
        S_Play("l32 o1 b-f-");
    }
}
