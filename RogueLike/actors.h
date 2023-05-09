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

Actor * GetActorAtTile(const ActorList * actor_list, TileCoord coord);
Actor * FindActor(const ActorList * actor_list, ActorType type);
const Actor * FindActorConst(const ActorList * actor_list, ActorType type);
int FindActors(const ActorList * actor_list, ActorType type, Actor * out[]);
void DebugPrintActorList(const ActorList * list);

#endif /* actors_h */
