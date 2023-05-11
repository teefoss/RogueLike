//
//  menu.c
//  RogueLike
//
//  Created by Thomas Foster on 4/22/23.
//

#include "menu.h"
#include "video.h"
#include "game_state.h"
#include "game.h"

int menu_state;

static void MenuPrint(const char * string, int row, int col)
{
    int margin = 64;

    int x = margin + col * V_CharWidth();
    int y = margin + row * (V_CharHeight() * 1.5f);

    V_PrintString(x, y, string);
}


static void ScrollUp(int * cursor, int num_items)
{
    *cursor -= 1;

    if ( *cursor < 0 ) {
        *cursor = num_items - 1;
    }
}


static void ScrollDown(int * cursor, int num_items)
{
    *cursor += 1;

    if ( *cursor >= num_items ) {
        *cursor = 0;
    }
}


#pragma mark - MAIN MENU

enum {
    MAIN_NEW_GAME,
    MAIN_QUIT,
    MAIN_NUM_ITEMS
};

int main_cursor;

static void MainRender(const Game * game)
{
    SetColor(GOLINE_BROWN);
    MenuPrint("Untitled Rogue-like", 0, 0);

    SetColor(GOLINE_WHITE);
    MenuPrint("New Game", 2, 0);
    MenuPrint("Quit to Desktop", 3, 0);

    MenuPrint("-", main_cursor + 2, -2);
}


static void MainMenuAction(Game * game)
{
    switch ( main_cursor ) {
        case MAIN_NEW_GAME:
            MenuToggle(game); // Close menu.
            NewGame(game);
            break;
        case MAIN_QUIT:
            game->is_running = false;
            break;
    }
}


static bool MainProcessEvent(Game * game, const SDL_Event * event)
{
    switch ( event->type ) {
        case SDL_KEYDOWN:
            switch ( event->key.keysym.sym ) {
                case SDLK_DOWN:
                case SDLK_s:
                    ScrollDown(&main_cursor, MAIN_NUM_ITEMS);
                    return true;
                case SDLK_UP:
                case SDLK_w:
                    ScrollUp(&main_cursor, MAIN_NUM_ITEMS);
                    return true;
                case SDLK_RETURN:
                    MainMenuAction(game);
                    return true;
                default:
                    return false;
            }
            break;
        default:
            return false;
    }
}


#pragma mark -


void MenuToggle(Game * game)
{
    if ( menu_state == MENU_NONE ) {
        PushState(game, &gs_menu);
        menu_state = MENU_MAIN;
    } else {
        PopState(game);
        menu_state = MENU_NONE;
    }
}


bool MenuProcessEvent(Game * game, const SDL_Event * event)
{
    switch ( menu_state ) {
        case MENU_MAIN:
            return MainProcessEvent(game, event);
        default:
            return false;
    }
}


void MenuRender(const Game * game)
{
    switch ( menu_state ) {
        case MENU_MAIN:
            MainRender(game);
            break;
        default:
            break;
    }
}
