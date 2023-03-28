//
//  animation.c
//  RogueLike
//
//  Created by Thomas Foster on 11/28/22.
//

#include "game.h"
#include "mylib/mathlib.h"

/// Move actor from offset start to end
void AnimateActorMove(actor_t * actor, float move_timer)
{
    actor->offset_current = Vec2Lerp(&actor->offset_start, &vec2_zero, move_timer);
}

void SetUpMoveAnimation(actor_t * actor, direction_t direction)
{
    actor->offset_start.x = -XDelta(direction) * RENDER_TILE_SIZE;
    actor->offset_start.y = -YDelta(direction) * RENDER_TILE_SIZE;
    actor->animation = AnimateActorMove;
}

void SetUpBumpAnimation(actor_t * actor, direction_t direction)
{
    actor->offset_start.x = ((float)XDelta(direction) * 0.5f) * RENDER_TILE_SIZE;
    actor->offset_start.y = ((float)YDelta(direction) * 0.5f) * RENDER_TILE_SIZE;
    actor->animation = AnimateActorMove;
}