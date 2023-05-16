//
//  animation.c
//  RogueLike
//
//  Created by Thomas Foster on 11/28/22.
//

#include "game.h"
#include "mathlib.h"

/// Move actor from offset start to end
void AnimateActorMove(Actor * actor, float move_timer)
{
    actor->offset_current = Vec2Lerp(&actor->offset_start, &vec2_zero, move_timer);
}

void SetUpMoveAnimation(Actor * actor, TileCoord destination)
{
    actor->offset_start.x = (actor->tile.x - destination.x) * SCALED(TILE_SIZE);
    actor->offset_start.y = (actor->tile.y - destination.y) * SCALED(TILE_SIZE);
    actor->animation = AnimateActorMove;
}

void SetUpBumpAnimation(Actor * actor, Direction direction)
{
    if ( direction != NO_DIRECTION ) {
        actor->offset_start.x = ((float)XDelta(direction) * 0.5f) * SCALED(TILE_SIZE);
        actor->offset_start.y = ((float)YDelta(direction) * 0.5f) * SCALED(TILE_SIZE);
        actor->animation = AnimateActorMove;
    }
}
