//
//  menu.h
//  RogueLike
//
//  Created by Thomas Foster on 4/22/23.
//

#ifndef menu_h
#define menu_h

#include "game.h"

enum {
    MENU_NONE,
    MENU_MAIN,
    MENU_VIDEO,
    NUM_MENUS,
};

void MenuToggle(Game * game);
bool MenuProcessEvent(Game * game, const SDL_Event * event);
void MenuRender(const Game * game);

extern int menu_state;

#endif /* menu_h */
