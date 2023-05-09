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
        { ACTOR_NONE,           30 },
        { ACTOR_ITEM_HEALTH,    10 },
        { ACTOR_ITEM_TURN,      10 },
        { ACTOR_ITEM_STRENGTH,  10 },
        { 0, -1 } // Terminator
    },
    [ACTOR_SUPER_SPIDER] = {
        { ACTOR_NONE, 60 },
        { ACTOR_ITEM_FUEL_BIG, 15 },
        { ACTOR_ITEM_FUEL_SMALL, 25 },
        { 0, -1 },
    }
};


// Drop loot according to the loot table of parameter, `actor_type`.
// - returns: The `ActorType` of the loot dropped.
ActorType SelectLoot(ActorType actor_type)
{
    int total_weight = 0;

    const Loot * table = &loot_tables[actor_type][0];

    // Somebody forgot to add a table for this actor. (A zero-initialed
    // index of loot_tables is being accessed.)
    if ( table[0].weight == 0 ) {
        const char * name = ActorName(actor_type);
        printf("%s: actor '%s' has no loot table!\n", __func__, name);
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

