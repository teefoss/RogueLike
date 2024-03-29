//
//  loot.c
//  RogueLike
//
//  Created by Thomas Foster on 4/26/23.
//

#include "loot.h"
#include "mathlib.h"

// Each list is terminated with dummy item with weight of -1.
const Loot loot_tables[NUM_ACTOR_TYPES][20] = {
    [ACTOR_BLOB] = {
        { ACTOR_NONE,               50 },
        { ACTOR_ITEM_HEALTH,        10 },
        { ACTOR_ITEM_TURN,          10 },
        { ACTOR_ITEM_STRENGTH,      10 },
        { 0, -1 } // Terminator
    },
    [ACTOR_SPIDER] = {
        { ACTOR_NONE,               80 },
        { ACTOR_ITEM_HEALTH,        7 },
        { ACTOR_ITEM_TURN,          7 },
        { ACTOR_ITEM_FUEL_SMALL,    7 },
        { 0, -1 },
    },
    [ACTOR_SUPER_SPIDER] = {
        { ACTOR_NONE,               80 },
        { ACTOR_ITEM_FUEL_BIG,      5 },
        { ACTOR_ITEM_FUEL_SMALL,    10 },
        { 0, -1 },
    },
    [ACTOR_VASE] = {
        { ACTOR_NONE,               75 },
        { ACTOR_ITEM_HEALTH,        15 },
        { ACTOR_ITEM_TURN,          10, },
        { 0, -1 },
    },
    [ACTOR_CLOSED_CHEST] = {
        { ACTOR_ITEM_HEALTH,        50 },
        { ACTOR_ITEM_TURN,          50, },
        { 0, -1 },
    },
    [ACTOR_GHOST] = {
        { ACTOR_ITEM_FUEL_BIG,      50 },
        { ACTOR_ITEM_FUEL_SMALL,    50 },
        { 0, -1 },
    },
};


// Drop loot according to the loot table of parameter, `actor_type`.
// - returns: The `ActorType` of the loot dropped.
ActorType SelectLoot(ActorType actor_type)
{
    int total_weight = 0;

    const Loot * table = &loot_tables[actor_type][0];

    // For any actors that don't drop loot, the uninitalized table's weight
    // will be zero.
    if ( table[0].weight <= 0 ) {
        return ACTOR_NONE;
    }

    for ( int i = 0; table[i].weight != -1; i++ ) {
        total_weight += table[i].weight;
    }

    int r = Random(0, total_weight - 1);
    int running_total = 0;


    for ( int i = 0; table[i].weight != -1; i++ ) {
        running_total += table[i].weight;
        if ( running_total > r ) {
            return table[i].actor_type;
        }
    }

    // Unreachable
    return ACTOR_NONE;
}

