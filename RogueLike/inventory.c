//
//  inventory.c
//  RogueLike
//
//  Created by Thomas Foster on 4/15/23.
//

#include "inventory.h"
#include "video.h"
#include "direction.h"

#include <stdarg.h>

#define LINE_SPACING 1.5

#define INVENTORY_MARGIN_TOP HUD_MARGIN
#define INVENTORY_MARGIN_LEFT (HUD_MARGIN + SCALED(2))
#define CELL_MARGIN 2

#define ITEM_COLS 4

#define LINING_GRAY 32

void InventoryPrint(int col, int row, const char * format, ...)
{
    va_list args[2];
    va_start(args[0], format);
    va_copy(args[1], args[0]);

    int len = vsnprintf(NULL, 0, format, args[0]);
    char * buffer = calloc(len + 1, sizeof(*buffer));
    vsnprintf(buffer, len + 1, format, args[1]);
    va_end(args[0]);
    va_end(args[1]);

    int x = INVENTORY_MARGIN_LEFT + col * V_CharWidth();
    int y = INVENTORY_MARGIN_TOP + row * (V_CharHeight() * LINE_SPACING);

    V_PrintString(x, y, buffer);
}


bool InventoryIsEmtpy(const Inventory * inventory)
{
    for ( int i = 0; i < NUM_ITEMS; i++ ) {
        if ( inventory->item_counts[i] > 0 ) {
            return false;
        }
    }

    return true;
}


void ChangeInventorySelection(Inventory * inventory, Direction direction)
{
    switch ( direction ) {
        case NORTH:
            inventory->selected_item -= ITEM_COLS;
            if ( inventory->selected_item < 0 ) {
                while ( inventory->selected_item + ITEM_COLS < NUM_ITEMS ) {
                    inventory->selected_item += ITEM_COLS;
                }
            }
            break;
        case SOUTH:
            inventory->selected_item += ITEM_COLS;
            if ( inventory->selected_item >= NUM_ITEMS ) {
                int row = inventory->selected_item / ITEM_COLS;
                inventory->selected_item -= row * ITEM_COLS;
            }
            break;
        case EAST:
            if ( (inventory->selected_item + 1) % ITEM_COLS == 0 ) { // Left side
                inventory->selected_item -= ITEM_COLS - 1; // Jump to left.
            } else {
                ++inventory->selected_item;
            }
            break;
        case WEST:
            if ( inventory->selected_item % ITEM_COLS == 0 ) { // Right side
                inventory->selected_item += ITEM_COLS - 1;
            } else {
                --inventory->selected_item;
            }
            break;
        default:
            break;
    }
}


static void RenderItemInfo(Item item,
                           int count,
                           int x,
                           int y,
                           bool selected,
                           const RenderInfo * render_info)
{
    SDL_Rect icon_cell = {
        .x = x - SCALED(CELL_MARGIN),
        .y = y - SCALED(CELL_MARGIN),
        .w = SCALED(ICON_SIZE + CELL_MARGIN * 2 - 1),
        .h = SCALED(ICON_SIZE + CELL_MARGIN * 2 - 1)
    };

    // Cell background
    SetColor(GOLINE_BLACK);
    V_FillRect(&icon_cell);

    // Cell border color
    if ( selected ) {
        SetColor(GOLINE_RED);
    } else {
        V_SetGray(LINING_GRAY);
    }

    // Cell border rect
    for ( int i = 0; i < SCALED(1); i++ ) {
        V_DrawRect(&icon_cell);
        icon_cell.x++;
        icon_cell.y++;
        icon_cell.w -= 2;
        icon_cell.h -= 2;
    }

    if ( count > 0 ) {
        RenderIcon(ItemIcon(item), x, y, render_info);

        SetColor(GOLINE_WHITE);
        V_PrintString(x + V_CharWidth() * 2 + SCALED(1), y, "%d", count);
    }
}


int InventoryWidth(void)
{
    int width = 0;
    width += INVENTORY_MARGIN_LEFT;
    width += 5 * V_CharWidth() * ITEM_COLS;

    return width;
};


void RenderInventory(const Inventory * inv, const RenderInfo * info)
{
    SDL_Rect inventory_panel = {
        .x = info->inventory_x,
        .y = 0,
        .w = GAME_WIDTH - info->inventory_x,
        .h = GAME_HEIGHT
    };
    SDL_RenderSetViewport(renderer, &inventory_panel);

    // Background and lefthand border.
    V_SetGray(16);
    V_FillRect(NULL);
    V_SetGray(LINING_GRAY);
    for ( int i = 0; i < SCALED(1); i++ ) {
        V_DrawVLine(i, 0, inventory_panel.h);
    }

    // Title
    SetColor(GOLINE_WHITE);
    InventoryPrint(0, 0, "Inventory");

    // Print currently selected item.
    if ( inv->item_counts[inv->selected_item] > 0 ) {
        SetColor(GOLINE_RED);
        InventoryPrint(0, 1, ItemName(inv->selected_item));
    }

    // Render grid of all items.
    for ( int i = 0; i < NUM_ITEMS; i++ ) {

        bool selected = inv->selected_item == i;
        int line_height = V_CharHeight() * 2;
        int start_y = INVENTORY_MARGIN_TOP + (V_CharHeight() * LINE_SPACING) * 2.5; // row 2.5
        int col = (i % ITEM_COLS) * 5;
        int row = (i / ITEM_COLS);

        RenderItemInfo(i,
                       inv->item_counts[i],
                       INVENTORY_MARGIN_LEFT + col * V_CharWidth(),
                       start_y + row * line_height,
                       selected,
                       info);
    }

    SDL_RenderSetViewport(renderer, NULL);
}
