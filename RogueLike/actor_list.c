//
//  actors.c
//  RogueLike
//
//  Created by Thomas Foster on 4/24/23.
//

#include "actor_list.h"

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


void DestroyActorList(ActorList * list)
{
    Actor * actor;
    Actor * temp;

    actor = list->head;
    while ( actor ) {
        temp = actor;
        actor = actor->next;
        free(temp);
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;

    actor = list->free_list;
    while ( actor ) {
        temp = actor;
        actor = actor->prev;
        free(temp);
    }
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


/// Move all active actors to the unused list.
void RemoveAllActors(ActorList * list)
{
    Actor * actor = list->head;

    while ( actor ) {
        actor->prev = list->free_list;
        list->free_list = actor;
        actor = actor->next;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}
