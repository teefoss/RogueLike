//
//  actors.c
//  RogueLike
//
//  Created by Thomas Foster on 4/24/23.
//

#include "actors.h"

Actor * GetActorAtTile(Actors * actors, TileCoord coord)
{
    for ( int i = 0; i < actors->count; i++ ) {
        if (   actors->list[i].tile.x == coord.x
            && actors->list[i].tile.y == coord.y ) {
            return &actors->list[i];
        }
    }

    return NULL;
}


Actor * FindActor(Actors * actors, ActorType type)
{
    for ( int i = 0; i < actors->count; i++ ) {
        if ( actors->list[i].type == type ) {
            return &actors->list[i];
        }
    }

    return NULL;
}


const Actor * FindActorConst(const Actors * actors, ActorType type)
{
    for ( int i = 0; i < actors->count; i++ ) {
        if ( actors->list[i].type == type ) {
            return &actors->list[i];
        }
    }

    return NULL;
}


int FindActors(Actors * actors, ActorType type, Actor * out[])
{
    int count = 0;
    for ( int i = 0; i < actors->count; i++ ) {
        if ( actors->list[i].type == type ) {
            out[count++] = &actors->list[i];
        }
    }

    return count;
}
