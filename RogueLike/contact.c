//
//  contact.c
//  RogueLike
//
//  Created by Thomas Foster on 11/8/22.
//

#include "game.h"
#include "sound.h"
#include "game_log.h"

void C_Player(Actor * player, Actor * hit)
{
    if ( hit->info->flags.takes_damage ) {
        int damage = player->stats.damage;
        damage += player->game->player_info.strength_buff;

        if ( DamageActor(hit, player, damage) <= 0 ) {
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

            switch ( hit->info->item ) {
                case ITEM_HEALTH:
                    Log("Picked up a Health Potion!");
                    S_Play("l32 o2 e b g+ > d+");
                    break;
                case ITEM_TURN:
                    Log("Picked up a Turn Potion!");
                    S_Play("l32 o2 a b > f+");
                    break;
                case ITEM_STRENGTH:
                    Log("Picked up a Strength Potion!");
                    S_Play("l32 t100 o1 c e g+ c+ f a d f+ b-");
                    break;
                case ITEM_FUEL_SMALL:
                    Log("Picked up Small Lamp Fuel!");
                    S_Play("o3 l16 t160 e- b");
                    break;
                case ITEM_FUEL_BIG:
                    Log("Picked up Large Lamp Fuel!");
                    S_Play("o2 l16 t160 e- b");
                    break;
                default:
                    break;
            }
        }
    }

    // Bump into other things
    switch ( hit->type ) {
        case ACTOR_ROPE:
            Log("Picked up the rope");
            player->game->player_info.has_rope = true;
            RemoveActor(hit);
            S_Play("o2 l32 f a < b-");
            break;
        case ACTOR_GOLD_KEY:
            Log("Got the golden key!");
            player->game->player_info.has_gold_key = true;
            RemoveActor(hit);
            S_Play("l32 t100 o1 a > a > a");
            break;
        case ACTOR_OLD_KEY:
            Log("Picked up an old key");
            player->game->player_info.has_shack_key = true;
            RemoveActor(hit);
            S_Play("l32 t100 o1 a > a > a"); // TODO: define key sound
            break;
        case ACTOR_PILLAR:
            S_Play(SOUND_BUMP);
            break;
        case ACTOR_SHACK_CLOSED:
            if ( player->game->player_info.has_shack_key ) {
                S_Play("l32 o2 f b-");
                TileCoord coord = hit->tile;
                SpawnActor(player->game, ACTOR_SHACK_OPEN, coord);
                RemoveActor(hit);
                player->game->player_info.has_shack_key = false;
            } else {
                Log("It's locked!");
                S_Play(SOUND_BUMP); // TODO: locked sound
            }
            break;
        case ACTOR_SHACK_OPEN:
            player->game->player_info.level_state = LEVEL_ENTER_SUB;
            break;
        default:
            break;
    }
}


void C_Monster(Actor * monster, Actor * hit)
{
    // Monsters cannot damage ghosts.
    if ( hit->type == ACTOR_GHOST ) {
        return;
    }

    // Monsters can damage other monsters, but not of the same type
    if ( hit->info->flags.takes_damage && monster->type != hit->type ) {
        DamageActor(hit, monster, monster->stats.damage);
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
    TileCoord coord;
    coord.x = block->tile.x + dx;
    coord.y = block->tile.y + dy;

    if ( TryMoveActor(block, coord) ) {
        S_Play("l32 o1 b-f-");
    }
}
