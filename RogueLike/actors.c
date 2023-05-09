//
//  actors.c
//  RogueLike
//
//  Created by Thomas Foster on 4/24/23.
//

#include "actors.h"

Actor * GetActorAtTile(const ActorList * actor_list, TileCoord coord)
{
    for ( Actor * actor = actor_list->head; actor; actor = actor->next ) {
        if (   actor->tile.x == coord.x
            && actor->tile.y == coord.y ) {
            return actor;
        }
    }

    return NULL;
}


Actor * FindActor(const ActorList * actor_list, ActorType type)
{
    for ( Actor * actor = actor_list->head; actor; actor = actor->next ) {
        if ( actor->type == type ) {
            return actor;
        }
    }

    return NULL;
}


const Actor * FindActorConst(const ActorList * actor_list, ActorType type)
{
    for ( const Actor * actor = actor_list->head; actor; actor = actor->next ) {
        if ( actor->type == type ) {
            return actor;
        }
    }

    return NULL;
}


int FindActors(const ActorList * actor_list, ActorType type, Actor * out[])
{
    int count = 0;
    for ( Actor * actor = actor_list->head; actor; actor = actor->next ) {
        if ( actor->type == type ) {
            out[count++] = actor;
        }
    }

    return count;
}


/// Add a previously allocated actor to linked list.
void AppendActor(ActorList * list, Actor * actor)
{
    if ( list->tail != NULL ) {
        list->tail->next = actor;
    }

    actor->prev = list->tail; // NULL if adding to empty list.
    actor->next = NULL;
    list->tail = actor;

    if ( list->head == NULL ) {
        // This is the only actor in the list, and is both head and tail.
        list->head = actor;
    }

    list->count++;
}


void RemoveActor(ActorList * list, Actor * actor)
{
    if ( actor == NULL ) {
        return;
    }

    if ( actor == list->head ) {
        list->head = actor->next;
    }

    if ( actor == list->tail ) {
        list->tail = actor->prev;
    }

    if ( actor->prev ) {
        actor->prev->next = actor->next;
    }

    if ( actor->next ) {
        actor->next->prev = actor->prev;
    }

    free(actor);
    --list->count;
}


void RemoveAllActors(ActorList * list)
{
    Actor * actor = list->head;

    while ( actor ) {
        Actor * temp = actor;
        actor = actor->next;
        free(temp);
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}


void DebugPrintActorList(const ActorList * list)
{
    printf("ACTOR LIST\n");
    printf("count: %d\n", list->count);

    Actor * actor = list->head;
    int i = 0;
    while ( actor ) {
        printf("%d: actor %2d (%3d, %3d)\n",
               i, actor->type, actor->tile.x, actor->tile.y);
        i++;
        actor = actor->next;
    }
}
