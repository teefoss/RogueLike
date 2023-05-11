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
    if ( hit->info->flags.takes_damage ) {
        int damage = player->stats.damage;
        damage += player->game->player_info.strength_buff;

        if ( DamageActor(hit, player) <= 0 ) {
            switch ( hit->type ) {
                case ACTOR_VASE:
                    S_Play("o5 t160 l32 g+ e- c f b-");
                    break;
                case ACTOR_CLOSED_CHEST:
                    S_Play("o3 t100 l32 f c b-");
                    break;
                default:
                    S_Play("t160 l32 o3 e c+ c < a-");
                    break;
            }
        } else {
            S_Play("t160 l32 o3 e c+ c");
        }
    }

    // Collect inventory items
    if ( hit->info->flags.collectible ) {
        if ( AddToInventory(player, hit, hit->info->item) ) {

            char * log = player->game->log;
            size_t log_size = sizeof(player->game->log);

            switch ( hit->info->item ) {
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
                case ITEM_FUEL_SMALL:
                    strncpy(log, "Picked up Small Lamp Fuel!", log_size);
                    S_Play("o3 l16 t160 e- b");
                    break;
                case ITEM_FUEL_BIG:
                    strncpy(log, "Picked up Large Lamp Fuel!", log_size);
                    S_Play("o2 l16 t160 e- b");
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
            RemoveActor(hit);
            S_Play("l32 t100 o1 a > a > a");
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
    if ( hit->info->flags.takes_damage && monster->type != hit->type ) {
        DamageActor(hit, monster);
        if ( monster->info->attack_sound ) {
            S_Play(monster->info->attack_sound);
        } else {
            // TODO: default sound
        }
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
