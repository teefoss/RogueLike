//
//  actors.h
//  RogueLike
//
//  Created by Thomas Foster on 4/24/23.
//

#ifndef actors_h
#define actors_h

#include "actor.h"
#include "tile.h"

typedef struct actor Actor;

typedef struct {
    // Active actor list.
    Actor * head;
    Actor * tail;
    int count;

    // Unused actor list. Where actors go when they're removed.
    // Actors and are added back from here when spawning new ones.
    Actor * unused;
} ActorList;

// List operations.

void DestroyActorList(ActorList * list);
void RemoveAllActors(ActorList * list);
void DebugPrintActorList(const ActorList * list);

// Search ops.

Actor * GetActorAtTile(const ActorList * actor_list, TileCoord coord);
Actor * FindActor(const ActorList * actor_list, ActorType type);
const Actor * FindActorConst(const ActorList * actor_list, ActorType type);
int FindActors(const ActorList * actor_list, ActorType type, Actor * out[]);

#endif /* actors_h */
