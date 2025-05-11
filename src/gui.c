#include "gui.h"
#include "engine.h"
#include <stdio.h>

#define ZC_RESET   "\033[0m"
#define BG_GREEN   "\033[42m"
#define REV_VIDEO  "\033[7m"

static valid_positions_t* g_valid = NULL;

void gui_set_valid_positions(valid_positions_t* v) {
    g_valid = v;
}

void gui_show_current_tile(tile_t tile, degree_t rotation) {
    tile_t tmp = tile;
    for (int r = 0; r < (int)rotation; r++)
        rotate_tile(&tmp, NINETY);
    printf("Tuile courante (rotation %d°) :\n", rotation * 90);
    print_tile(tmp);
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

static const char* zone_color(zone_type_t z) {
    switch (z) {
        case CITY:    return "\033[0;31m";
        case ROAD:    return "\033[0;33m";
        case FIELD:   return "\033[0;32m";
        case SHIELD:  return "\033[0;35m";
        case VILLAGE: return "\033[0;36m";
        case ABBEY:   return "\033[0;37m";
        default:      return ZC_RESET;
    }
}

void print_tile(tile_t t) {
    printf("\033[1m\033[0;37m┌───┐\n" ZC_RESET);
    printf("│ %s%c" ZC_RESET " │\n",
           zone_color(t.zones[NORTH]), print_zone(t.zones[NORTH]));
    printf("│%s%c" ZC_RESET "%s%c" ZC_RESET "%s%c" ZC_RESET "│\n",
           zone_color(t.zones[WEST]),   print_zone(t.zones[WEST]),
           zone_color(t.zones[CENTER]), print_zone(t.zones[CENTER]),
           zone_color(t.zones[EAST]),   print_zone(t.zones[EAST]));
    printf("│ %s%c" ZC_RESET " │\n",
           zone_color(t.zones[SOUTH]), print_zone(t.zones[SOUTH]));
    printf("\033[1m\033[0;37m└───┘\n" ZC_RESET);
}

void gui_display_positions(valid_positions_t* v) {
    printf("\n\033[0;36m╭─ Positions valides ─────────────────╮\033[0m\n");
    for (int i = 0; i < v->size; i++) {
        printf(" [%2d] (%2d,%2d)   ",
               i,
               v->positions[i].x,
               v->positions[i].y);
        if ((i + 1) % 2 == 0) printf("\n");
    }
    if (v->size % 2) printf("\n");
    printf("\033[0;36m╰──────────────────────────────────────╯\033[0m\n\n");
}

void gui_display_meeple_options(meeple_options_t opts, color_t player_color) {
    (void)player_color;
    printf("\033[0;35m╭─ Options meeple ────────────────────╮\033[0m\n");
    const char* names[5] = { "Nord", "Est", "Sud", "Ouest", "Centre" };
    const char  keys[5]  = { 'N',     'E',     'S',     'W',      'C' };
    for (int i = 0; i < 5; i++) {
        if (opts.available[i]) {
            printf("%s [%c] %-6s %s   ",
                   BG_GREEN, keys[i], names[i], ZC_RESET);
        } else {
            printf(" [%c] %-6s   ", keys[i], names[i]);
        }
        if ((i + 1) % 2 == 0) printf("\n");
    }
    if (5 % 2) printf("\n");
    printf("\033[0;35m╰──────────────────────────────────────╯\033[0m\n\n");
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
            tile_t t = board->grid[by][bx];
            bool hl_valid = false, hl_cursor = false;
            if (g_valid) {
                for (int i = 0; i < g_valid->size; i++) {
                    position_t vp = g_valid->positions[i];
                    if (vp.x == by && vp.y == bx) { hl_valid = true; break; }
                }
            }
            if (cursor.x == by && cursor.y == bx) hl_cursor = true;
            if (hl_valid)  printf(BG_GREEN);
            if (hl_cursor) printf(REV_VIDEO);
            printf("\033[1m\033[0;37m┌───┐" ZC_RESET);
        }
        printf("\n");

        // Ligne Nord
        for (int tx = 0; tx < viewW; ++tx) {
            int bx = offsetX + tx;
            tile_t t = board->grid[by][bx];
            bool hl_valid = false, hl_cursor = false;
            if (g_valid) {
                for (int i = 0; i < g_valid->size; i++) {
                    position_t vp = g_valid->positions[i];
                    if (vp.x == by && vp.y == bx) { hl_valid = true; break; }
                }
            }
            if (cursor.x == by && cursor.y == bx) hl_cursor = true;
            if (hl_valid)  printf(BG_GREEN);
            if (hl_cursor) printf(REV_VIDEO);

            printf("│ ");
            // si meeple présent au NORD, colorier ce caractère
            if (t.has_meeple && t.meeple_position == NORTH) {
                printf("%s%c%s",
                       color_code(t.meeple_color),
                       print_zone(t.zones[NORTH]),
                       ZC_RESET);
            } else {
                printf("%c", print_zone(t.zones[NORTH]));
            }
            printf(" │");
            printf(ZC_RESET);
        }
        printf("\n");

        // Ligne Ouest-Centre-Est
        for (int tx = 0; tx < viewW; ++tx) {
            int bx = offsetX + tx;
            tile_t t = board->grid[by][bx];
            bool hl_valid = false, hl_cursor = false;
            if (g_valid) {
                for (int i = 0; i < g_valid->size; i++) {
                    position_t vp = g_valid->positions[i];
                    if (vp.x == by && vp.y == bx) { hl_valid = true; break; }
                }
            }
            if (cursor.x == by && cursor.y == bx) hl_cursor = true;
            if (hl_valid)  printf(BG_GREEN);
            if (hl_cursor) printf(REV_VIDEO);

            printf("│");
            // OUEST
            if (t.has_meeple && t.meeple_position == WEST) {
                printf("%s%c%s",
                       color_code(t.meeple_color),
                       print_zone(t.zones[WEST]),
                       ZC_RESET);
            } else {
                printf("%c", print_zone(t.zones[WEST]));
            }
            // CENTRE
            if (t.has_meeple && t.meeple_position == CENTER) {
                printf("%s%c%s",
                       color_code(t.meeple_color),
                       print_zone(t.zones[CENTER]),
                       ZC_RESET);
            } else {
                printf("%c", print_zone(t.zones[CENTER]));
            }
            // EST
            if (t.has_meeple && t.meeple_position == EAST) {
                printf("%s%c%s",
                       color_code(t.meeple_color),
                       print_zone(t.zones[EAST]),
                       ZC_RESET);
            } else {
                printf("%c", print_zone(t.zones[EAST]));
            }
            printf("│");
            printf(ZC_RESET);
        }
        printf("\n");

        // Ligne Sud
        for (int tx = 0; tx < viewW; ++tx) {
            int bx = offsetX + tx;
            tile_t t = board->grid[by][bx];
            bool hl_valid = false, hl_cursor = false;
            if (g_valid) {
                for (int i = 0; i < g_valid->size; i++) {
                    position_t vp = g_valid->positions[i];
                    if (vp.x == by && vp.y == bx) { hl_valid = true; break; }
                }
            }
            if (cursor.x == by && cursor.y == bx) hl_cursor = true;
            if (hl_valid)  printf(BG_GREEN);
            if (hl_cursor) printf(REV_VIDEO);

            printf("│ ");
            // si meeple présent au SUD, colorier ce caractère
            if (t.has_meeple && t.meeple_position == SOUTH) {
                printf("%s%c%s",
                       color_code(t.meeple_color),
                       print_zone(t.zones[SOUTH]),
                       ZC_RESET);
            } else {
                printf("%c", print_zone(t.zones[SOUTH]));
            }
            printf(" │");
            printf(ZC_RESET);
        }
        printf("\n");

        // Bordure basse
        for (int tx = 0; tx < viewW; ++tx) {
            int bx = offsetX + tx;
            bool hl_valid = false, hl_cursor = false;
            int by = offsetY + ty; /* correction d’index dans cette boucle */
            if (g_valid) {
                for (int i = 0; i < g_valid->size; i++) {
                    position_t vp = g_valid->positions[i];
                    if (vp.x == by && vp.y == bx) { hl_valid = true; break; }
                }
            }
            if (cursor.x == by && cursor.y == bx) hl_cursor = true;
            if (hl_valid)  printf(BG_GREEN);
            if (hl_cursor) printf(REV_VIDEO);

            printf("\033[1m\033[0;37m└───┘" ZC_RESET);
        }
        printf("\n");
    }
}