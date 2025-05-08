#include "gui.h"
#include "engine.h"
#include "ansi_colors.h"
#include <stdio.h>

// === POINTEUR GLOBAL POUR LES POSITIONS VALIDES ===
static valid_positions_t* g_valid = NULL;
static int g_remaining_tiles = 0;

// === AFFICHAGE DES TUILES ET ZONES ===
static const char* zone_color(zone_type_t z) {
    switch (z) {
        case CITY:    return ANSI_RED;
        case ROAD:    return ANSI_YELLOW;
        case FIELD:   return ANSI_GREEN;
        case SHIELD:  return ANSI_MAGENTA;
        case VILLAGE: return ANSI_CYAN;
        case ABBEY:   return ANSI_WHITE;
        default:      return ANSI_RESET;
    }
}

char print_zone(zone_type_t z) {
    switch (z) {
        case SHIELD:  return 'S';
        case ROAD:    return 'R';
        case FIELD:   return 'F';
        case CITY:    return 'C';
        case VILLAGE: return 'V';
        case ABBEY:   return 'A';
        default:      return ' ';
    }
}

void print_tile(tile_t t) {
    printf(ANSI_BOLD ANSI_WHITE "┌───┐\n" ANSI_RESET);
    printf("│ %s%c" ANSI_RESET " │\n",
           zone_color(t.zones[NORTH]), print_zone(t.zones[NORTH]));
    printf("│%s%c" ANSI_RESET "%s%c" ANSI_RESET "%s%c" ANSI_RESET "│\n",
           zone_color(t.zones[WEST]),   print_zone(t.zones[WEST]),
           zone_color(t.zones[CENTER]), print_zone(t.zones[CENTER]),
           zone_color(t.zones[EAST]),   print_zone(t.zones[EAST]));
    printf("│ %s%c" ANSI_RESET " │\n",
           zone_color(t.zones[SOUTH]), print_zone(t.zones[SOUTH]));
    printf(ANSI_BOLD ANSI_WHITE "└───┘\n" ANSI_RESET);
}

void gui_show_current_tile(tile_t tile, degree_t rotation) {
    tile_t tmp = tile;
    for (int r = 0; r < (int)rotation; r++)
        rotate_tile(&tmp, NINETY);
    printf("Tuile courante (rotation %d°) :\n", rotation * 90);
    print_tile(tmp);
}

void gui_display_positions(valid_positions_t* v) {
    printf(ANSI_CYAN "╭─ Positions valides ─────────────────╮" ANSI_RESET "\n");
    for (int i = 0; i < v->actual; i++) {
        printf(" [%2d] (%2d,%2d)   ",
               i,
               v->valid_positions[i].x,
               v->valid_positions[i].y);
        if ((i + 1) % 2 == 0) printf("\n");
    }
    if (v->actual % 2) printf("\n");
    printf(ANSI_CYAN "╰──────────────────────────────────────╯" ANSI_RESET "\n\n");
}

void gui_set_valid_positions(valid_positions_t* v) {
    g_valid = v;
}

void gui_display_meeple_options(meeple_options_t opts, color_t player_color) {
    (void)player_color;
    printf(ANSI_MAGENTA "╭─ Options meeple ────────────────────╮" ANSI_RESET "\n");
    const char* names[5] = { "Nord", "Est", "Sud", "Ouest", "Centre" };
    const char  keys[5]  = { 'N',     'E',     'S',     'W',      'C' };
    for (int i = 0; i < 5; i++) {
        if (opts.available[i]) {
            printf("%s [%c] %-6s %s   ",
                   BG_GREEN, keys[i], names[i], ANSI_RESET);
        } else {
            printf(" [%c] %-6s   ", keys[i], names[i]);
        }
        if ((i + 1) % 2 == 0) printf("\n");
    }
    if (5 % 2) printf("\n");
    printf(ANSI_MAGENTA "╰──────────────────────────────────────╯" ANSI_RESET "\n\n");
}

void gui_set_remaining_tiles(int remaining) {
    g_remaining_tiles = remaining;
}

void print_board(board_t *board,
                 int offsetX, int offsetY,
                 int viewW,   int viewH,
                 position_t cursor)
{
    printf("Vue : X=[%d…%d], Y=[%d…%d]\n",
           offsetX, offsetX + viewW - 1,
           offsetY, offsetY + viewH - 1);

    if (offsetX < 0) offsetX = 0;
    if (offsetY < 0) offsetY = 0;
    if (offsetX + viewW > board->width)  viewW  = board->width  - offsetX;
    if (offsetY + viewH > board->height) viewH  = board->height - offsetY;
    if (viewW <= 0 || viewH <= 0) return;

    for (int ty = 0; ty < viewH; ++ty) {
        int by = offsetY + ty;

        // Bordure haute
        for (int tx = 0; tx < viewW; ++tx) {
            int bx = offsetX + tx;
            bool hl_valid = false, hl_cursor = false;
            if (g_valid) {
                for (int i = 0; i < g_valid->actual; i++) {
                    position_t vp = g_valid->valid_positions[i];
                    if (vp.x == by && vp.y == bx) { hl_valid = true; break; }
                }
            }
            if (cursor.x == by && cursor.y == bx) hl_cursor = true;
            if (hl_valid)  printf(BG_GREEN);
            if (hl_cursor) printf(REV_VIDEO);
            printf(ANSI_BOLD ANSI_WHITE "┌───┐" ANSI_RESET);
        }
        printf("\n");

        // Ligne Nord
        for (int tx = 0; tx < viewW; ++tx) {
            int bx = offsetX + tx;
            tile_t t = board->grid[by][bx];
            bool hl_valid = false, hl_cursor = false;
            if (g_valid) {
                for (int i = 0; i < g_valid->actual; i++) {
                    position_t vp = g_valid->valid_positions[i];
                    if (vp.x == by && vp.y == bx) { hl_valid = true; break; }
                }
            }
            if (cursor.x == by && cursor.y == bx) hl_cursor = true;
            if (hl_valid)  printf(BG_GREEN);
            if (hl_cursor) printf(REV_VIDEO);

            printf("│ ");
            if (t.has_meeple && t.meeple_position == NORTH) {
                printf("%s%c%s", color_code(t.meeple_color), print_zone(t.zones[NORTH]), ANSI_RESET);
            } else {
                printf("%c", print_zone(t.zones[NORTH]));
            }
            printf(" │");
            printf(ANSI_RESET);
        }
        printf("\n");

        // Ligne Ouest-Centre-Est
        for (int tx = 0; tx < viewW; ++tx) {
            int bx = offsetX + tx;
            tile_t t = board->grid[by][bx];
            bool hl_valid = false, hl_cursor = false;
            if (g_valid) {
                for (int i = 0; i < g_valid->actual; i++) {
                    position_t vp = g_valid->valid_positions[i];
                    if (vp.x == by && vp.y == bx) { hl_valid = true; break; }
                }
            }
            if (cursor.x == by && cursor.y == bx) hl_cursor = true;
            if (hl_valid)  printf(BG_GREEN);
            if (hl_cursor) printf(REV_VIDEO);

            printf("│");
            if (t.has_meeple && t.meeple_position == WEST)
                printf("%s%c%s", color_code(t.meeple_color), print_zone(t.zones[WEST]), ANSI_RESET);
            else
                printf("%c", print_zone(t.zones[WEST]));

            if (t.has_meeple && t.meeple_position == CENTER)
                printf("%s%c%s", color_code(t.meeple_color), print_zone(t.zones[CENTER]), ANSI_RESET);
            else
                printf("%c", print_zone(t.zones[CENTER]));

            if (t.has_meeple && t.meeple_position == EAST)
                printf("%s%c%s", color_code(t.meeple_color), print_zone(t.zones[EAST]), ANSI_RESET);
            else
                printf("%c", print_zone(t.zones[EAST]));

            printf("│");
            printf(ANSI_RESET);
        }
        printf("\n");

        // Ligne Sud
        for (int tx = 0; tx < viewW; ++tx) {
            int bx = offsetX + tx;
            tile_t t = board->grid[by][bx];
            bool hl_valid = false, hl_cursor = false;
            if (g_valid) {
                for (int i = 0; i < g_valid->actual; i++) {
                    position_t vp = g_valid->valid_positions[i];
                    if (vp.x == by && vp.y == bx) { hl_valid = true; break; }
                }
            }
            if (cursor.x == by && cursor.y == bx) hl_cursor = true;
            if (hl_valid)  printf(BG_GREEN);
            if (hl_cursor) printf(REV_VIDEO);

            printf("│ ");
            if (t.has_meeple && t.meeple_position == SOUTH) {
                printf("%s%c%s", color_code(t.meeple_color), print_zone(t.zones[SOUTH]), ANSI_RESET);
            } else {
                printf("%c", print_zone(t.zones[SOUTH]));
            }
            printf(" │");
            printf(ANSI_RESET);
        }
        printf("\n");

        // Bordure basse
        for (int tx = 0; tx < viewW; ++tx) {
            int bx = offsetX + tx;
            bool hl_valid = false, hl_cursor = false;
            int by = offsetY + ty;
            if (g_valid) {
                for (int i = 0; i < g_valid->actual; i++) {
                    position_t vp = g_valid->valid_positions[i];
                    if (vp.x == by && vp.y == bx) { hl_valid = true; break; }
                }
            }
            if (cursor.x == by && cursor.y == bx) hl_cursor = true;
            if (hl_valid)  printf(BG_GREEN);
            if (hl_cursor) printf(REV_VIDEO);

            printf(ANSI_BOLD ANSI_WHITE "└───┘" ANSI_RESET);
        }
        printf("\n");
    }

    printf("Tuiles restantes : %d\n\n", g_remaining_tiles);
}
