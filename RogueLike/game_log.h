//
//  game_log.h
//  RogueLike
//
//  Created by Thomas Foster on 5/20/23.
//

#ifndef game_log_h
#define game_log_h

#include "game.h"

void Log(const char * string);
void ResetLog(void);
void RenderLog(const Game * game, const RenderInfo * info);

#endif /* game_log_h */
