//
//  game_log.c
//  RogueLike
//
//  Created by Thomas Foster on 5/20/23.
//

#include "game_log.h"

#include "mathlib.h"
#include "render.h"
#include "video.h"
#include "game_state.h"

#define LOG_ROWS        10
#define STATUS_ROWS     8
#define LOG_LEN         100

static char _log[LOG_ROWS][LOG_LEN];

// Holds the log after being reset, so it can be slide off the screen.
static char _clear[LOG_ROWS][LOG_LEN];

int next_row = 0;


void Log(const char * string)
{
    if ( next_row == STATUS_ROWS ) {
        return; // Ran out of rows
    }

    strncpy(_log[next_row++], string, LOG_LEN);
}

void ResetLog(void)
{
    memcpy(_clear, _log, sizeof(_log));
    memset(_log, 0, sizeof(_log));
    next_row = 0;
}

void RenderLog(const Game * game, const RenderInfo * info)
{
    const int char_w = V_CharWidth();
    const int margin = HUD_MARGIN;
    const int line_height = V_CharHeight() * 1.5f;

    // Print status rows.

    int num_clear_rows = 0;
    for ( int row = 0; row < STATUS_ROWS; row++ ) {
        if ( strlen(_clear[row]) > 0 ) {
            num_clear_rows++;
        }
    }

    for ( int row = 0; row < STATUS_ROWS; row++ ) {

        size_t log_len = strlen(_log[row]);
        size_t clear_len = strlen(_clear[row]);

        int log_x = game->render_info.inventory_x - (log_len * char_w + margin);
        int clear_x = game->render_info.inventory_x - (clear_len * char_w + margin);
        int y = margin + (row * line_height);

        if ( GetGameState(game) == &gs_level_turn ) {
            if ( log_len != 0 ) {
                float x_slide = Lerp(game->render_info.inventory_x,
                                     log_x,
                                     game->move_timer);
                V_PrintString(x_slide, y, _log[row]);
            }

            if ( clear_len ) {
                float y_off = Lerp(y,
                                   -(num_clear_rows * line_height + margin) + y,
                                   game->move_timer);
                V_PrintString(clear_x, y_off, _clear[row]);
            }
        } else {
            if ( log_len ) {
                V_PrintString(log_x, y, _log[row]);
            }
        }
    }
}
