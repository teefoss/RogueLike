//
//  contact.c
//  RogueLike
//
//  Created by Thomas Foster on 11/8/22.
//

#include "game.h"
#include "sound.h"

void C_Player(Actor * player, Actor * hit)
{
    if ( hit->flags.takes_damage ) {
        int damage = player->stats.damage;
        damage += player->game->player_info.strength_buff;

        if ( DamageActor(hit, damage) <= 0 ) {
            S_Play("t160 l32 o3 e c+ c < a-");
        } else {
            S_Play("t160 l32 o3 e c+ c");
        }
    }

    // Collect inventory items
    if ( hit->flags.collectible ) {
        if ( AddToInventory(player, hit, hit->item) ) {

            char * log = player->game->log;
            size_t log_size = sizeof(player->game->log);

            switch ( hit->item ) {
                case ITEM_HEALTH:
                    strncpy(log, "Picked up a Health Potion!", log_size);
                    S_Play("l32 o2 e b g+ > d+");
                    break;
                case ITEM_TURN:
                    strncpy(log, "Picked up a Turn Potion!", log_size);
                    S_Play("l32 o2 a b > f+");
                    break;
                case ITEM_STRENGTH:
                    strncpy(log, "Picked up a Strength Potion!", log_size);
                    S_Play("l32 t100 o1 c e g+ c+ f a d f+ b-");
                    break;
                default:
                    break;
            }
        }
    }

    // Bump into other things
    switch ( hit->type ) {
        case ACTOR_GOLD_KEY:
            strncpy(player->game->log, "Got the golden key!", sizeof(player->game->log));
            player->game->player_info.has_gold_key = true;
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
        case ACTOR_PILLAR:
            S_Play(SOUND_BUMP);
            break;
        default:
            break;
    }
}


void C_Monster(Actor * monster, Actor * hit)
{
    // Monsters can damage other monsters, but not of the same type
    if ( hit->flags.takes_damage && monster->type != hit->type ) {
        DamageActor(hit, monster->stats.damage);
        S_Play("o3 l32 b c < c+");
    }
}


void C_Spider(Actor * spider, Actor * hit)
{
    if ( hit->flags.takes_damage ) {
        DamageActor(hit, spider->stats.damage);
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
