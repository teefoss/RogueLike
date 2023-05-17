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

#include <stdarg.h>

#define CURSOR_CHAR '*'

int menu_state;

int next_menus[NUM_MENUS] = {
    [MENU_NONE] = MENU_MAIN,
    [MENU_MAIN] = MENU_NONE,
    [MENU_VIDEO] = MENU_MAIN,
};


enum {
    ACTION_CONFIRM,
    ACTION_LEFT,
    ACTION_RIGHT,
};


static void MenuPrint(int col, int row, const char * format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[100] = { 0 };
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    int margin = 64;

    int x = margin + col * V_CharWidth();
    int y = margin + row * (V_CharHeight() * 1.5f);

    V_PrintString(x, y, buffer);
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


int main_cursor;
int video_cursor;


#pragma mark - MAIN MENU

enum {
    MAIN_NEW_GAME,
    MAIN_VIDEO,
    MAIN_QUIT,
    MAIN_NUM_ITEMS
};


static void MainRender(const Game * game)
{
    SetColor(GOLINE_BROWN);
    MenuPrint(0, 0, GAME_NAME);

    SetColor(GOLINE_WHITE);
    MenuPrint(0, 2, "New Game");
    MenuPrint(0, 3, "Video Options");
    MenuPrint(0, 4, "Quit to Desktop");

    MenuPrint(-2, main_cursor + 2, "%c", CURSOR_CHAR);
}


static void MainMenuAction(Game * game)
{
    switch ( main_cursor ) {
        case MAIN_NEW_GAME:
            MenuToggle(game); // Close menu.
            NewGame(game);
            break;
        case MAIN_VIDEO:
            menu_state = MENU_VIDEO;
            video_cursor = 0;
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


#pragma mark - VIDEO MENU

enum {
    VIDEO_FULLSCREEN,
    VIDEO_WINDOW_SCALE,
    VIDEO_BACK,
    VIDEO_NUM_ITEMS
};


static void VideoRender(const Game * game)
{
    SetColor(GOLINE_DARK_GREEN);
    MenuPrint(0, 0, "Video Options");

    SetColor(GOLINE_WHITE);

    bool is_fullscreen = SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP;
    MenuPrint(0, 2, "Fullscreen  %s", is_fullscreen ? "YES" : "NO" );
    MenuPrint(0, 3, "Window Scale  %c %.1f %c", game->render_info.window_scale, 0x1b, 0x1a);
    MenuPrint(0, 4, "Back");

    MenuPrint(-2, video_cursor + 2, "%c", CURSOR_CHAR);
}


static void VideoMenuAction(Game * game, int action)
{
    switch ( video_cursor ) {
        case VIDEO_FULLSCREEN: {
            if ( action == ACTION_CONFIRM ) {
                V_ToggleFullscreen(DESKTOP);
            }
            break;
        }
        case VIDEO_WINDOW_SCALE:
            if ( action == ACTION_LEFT ) {
                game->render_info.window_scale -= 0.5f;
            } else if ( action == ACTION_RIGHT ) {
                game->render_info.window_scale += 0.5f;
            } else {
                return;
            }
            CLAMP(game->render_info.window_scale, 1.0f, 5.0f);
            SDL_SetWindowSize(window,
                              game->render_info.width * game->render_info.window_scale,
                              game->render_info.height * game->render_info.window_scale);
            SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            break;
        case VIDEO_BACK:
            if ( action == ACTION_CONFIRM ) {
                menu_state = MENU_MAIN;
                main_cursor = 0;
            }
            break;
    }
}


static bool VideoProcessEvent(Game * game, const SDL_Event * event)
{
    switch ( event->type ) {
        case SDL_KEYDOWN:
            switch ( event->key.keysym.sym ) {
                case SDLK_DOWN:
                case SDLK_s:
                    ScrollDown(&video_cursor, VIDEO_NUM_ITEMS);
                    return true;
                case SDLK_UP:
                case SDLK_w:
                    ScrollUp(&video_cursor, VIDEO_NUM_ITEMS);
                    return true;
                case SDLK_RETURN:
                    VideoMenuAction(game, ACTION_CONFIRM);
                    return true;
                case SDLK_LEFT:
                    VideoMenuAction(game, ACTION_LEFT);
                    return true;
                case SDLK_RIGHT:
                    VideoMenuAction(game, ACTION_RIGHT);
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
        main_cursor = 0;
    }

    menu_state = next_menus[menu_state];

    if ( menu_state == MENU_NONE ) {
        PopState(game);
    }
}


bool MenuProcessEvent(Game * game, const SDL_Event * event)
{
    switch ( menu_state ) {
        case MENU_MAIN:
            return MainProcessEvent(game, event);
        case MENU_VIDEO:
            return VideoProcessEvent(game, event);
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
        case MENU_VIDEO:
            VideoRender(game);
            break;
        default:
            break;
    }
}
