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
    const vec2_t target = { 0 };
    VectorLerp(&actor->offset, &target, move_timer);
}

void SetUpBumpAnimation(actor_t * actor, int dx, int dy)
{
    actor->offset.x = ((float)dx * 0.5f) * RENDER_TILE_SIZE;
    actor->offset.y = ((float)dy * 0.5f) * RENDER_TILE_SIZE;
    actor->animation = AnimateActorMove;
}
