//
//  animation.c
//  GameBeta
//
//  Created by Thomas Foster on 11/28/22.
//

#include "main.h"

/// Move actor from offset start to end
void AnimateActorMove(actor_t * actor, float move_timer)
{
    actor->offsets[1].x = actor->offsets[0].x * (1.0f - move_timer);
    actor->offsets[1].y = actor->offsets[0].y * (1.0f - move_timer);
}

void AnimateActorBump(actor_t * actor, float move_timer)
{
    if ( move_timer < 0.5f ) {
        actor->offsets[1].x = actor->offsets[0].x * move_timer;
        actor->offsets[1].y = actor->offsets[0].y * move_timer;
    } else {
        actor->offsets[1].x = actor->offsets[0].x * (1.0f - move_timer);
        actor->offsets[1].y = actor->offsets[0].y * (1.0f - move_timer);
    }
}

void SetUpBumpAnimation(actor_t * actor, int dx, int dy)
{
    actor->offsets[0].x = dx * RENDER_TILE_SIZE;
    actor->offsets[0].y = dy * RENDER_TILE_SIZE;
    actor->animation = AnimateActorBump;
}
