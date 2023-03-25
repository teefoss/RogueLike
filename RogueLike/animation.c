//
//  animation.c
//  GameBeta
//
//  Created by Thomas Foster on 11/28/22.
//

#include "main.h"
#include "mylib/mathlib.h"
/// Move actor from offset start to end
void AnimateActorMove(actor_t * actor, float move_timer)
{
    actor->offset_current = Vec2Lerp(&actor->offset_start, &vec2_zero, move_timer);
}

void SetUpMoveAnimation(actor_t * actor, int dx, int dy)
{
    actor->offset_start.x = -dx * RENDER_TILE_SIZE;
    actor->offset_start.y = -dy * RENDER_TILE_SIZE;
    actor->animation = AnimateActorMove;
}

void SetUpBumpAnimation(actor_t * actor, int dx, int dy)
{
    actor->offset_start.x = ((float)dx * 0.5f) * RENDER_TILE_SIZE;
    actor->offset_start.y = ((float)dy * 0.5f) * RENDER_TILE_SIZE;
    actor->animation = AnimateActorMove;
}
