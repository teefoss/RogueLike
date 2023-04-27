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

typedef struct actors {
    int count;
    Actor list[MAX_ACTORS];
} Actors;

Actor * GetActorAtTile(Actors * actors, TileCoord coord);
Actor * FindActor(Actors * actors, ActorType type);
const Actor * FindActorConst(const Actors * actors, ActorType type);
int FindActors(Actors * actors, ActorType type, Actor * out[]);

#endif /* actors_h */
