#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "engine.h"


#define ANSI_RED    "\x1b[31m"
#define ANSI_BLUE   "\x1b[34m"
#define ANSI_GREEN  "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_WHITE  "\x1b[37m"
#define ANSI_RESET  "\x1b[0m"

const char *ANSI_COLOR_CODES[] = {
    [RED]      = ANSI_RED,
    [BLUE]     = ANSI_BLUE,
    [GREEN]    = ANSI_GREEN,
    [YELLOW]   = ANSI_YELLOW,
    [WHITE]    = ANSI_WHITE,
    [NO_COLOR] = ANSI_RESET
};

char print_zone(zone_type_t z) {
    switch (z) {
        case SHIELD:  return 's';
        case ROAD:    return 'r';
        case FIELD:   return 'f';
        case CITY:    return 'c';
        case VILLAGE: return 'v';
        case ABBEY:   return 'a';
        case NONE:    return ' ';
        default:      return ' ';
    }
}

void print_board(board_t* b) {
    // Print column headers
    printf("    ");
    for (int j = 0; j < b->width; j++) {
        printf(" %d  ", j);
    }
    printf("\n");

    for (int i = 0; i < b->height; i++) {
        // North row
        printf("    ");
        for (int j = 0; j < b->width; j++) {
            tile_t* t = &b->grid[i][j];
            char c = print_zone(t->zones[NORTH]);
            // always print one leading space
            printf(" ");
            if (t->has_meeple && t->meeple_position == NORTH) {
                const char *col = ANSI_COLOR_CODES[t->meeple_color];
                printf("%s%c%s", col, c, ANSI_RESET);
            } else {
                printf("%c", c);
            }
            // always print two trailing spaces
            printf("  ");
        }
        printf("\n");

        // West-Center-East row
        printf("%2d  ", i);
        for (int j = 0; j < b->width; j++) {
            tile_t* t = &b->grid[i][j];
            direction_t dirs[3] = { WEST, CENTER, EAST };
            for (int d = 0; d < 3; d++) {
                direction_t dir = dirs[d];
                char ch = print_zone(t->zones[dir]);
                if (t->has_meeple && t->meeple_position == dir) {
                    const char *col = ANSI_COLOR_CODES[t->meeple_color];
                    printf("%s%c%s", col, ch, ANSI_RESET);
                } else {
                    printf("%c", ch);
                }
            }
            // one space after the three chars
            printf(" ");
        }
        printf("\n");

        // South row
        printf("    ");
        for (int j = 0; j < b->width; j++) {
            tile_t* t = &b->grid[i][j];
            char c = print_zone(t->zones[SOUTH]);
            printf(" ");
            if (t->has_meeple && t->meeple_position == SOUTH) {
                const char *col = ANSI_COLOR_CODES[t->meeple_color];
                printf("%s%c%s", col, c, ANSI_RESET);
            } else {
                printf("%c", c);
            }
            printf("  ");
        }
        printf("\n");
    }
}
// Helper to clear a tile
static void clear_tile(tile_t* t) {
    for (int i = 0; i < 5; i++) {
        t->zones[i] = NONE;
    }
    t->has_meeple = false;
    t->meeple_position = NO_DIRECTION;
    t->meeple_color = NO_COLOR;
}

// Display meeple placement options for a given position
void print_meeple_options(board_t* b, position_t pos) {
    meeple_options_t opts = get_placement_meeple(b, pos);
    printf("Options de placement pour la tuile (%d,%d):", pos.x, pos.y);
    const char* names[5] = {"NORD", "EST", "SUD", "OUEST", "CENTER"};
    bool any = false;
    for (int z = 0; z < 5; z++) {
        if (opts.available[z]) {
            printf(" %s", names[z]);
            any = true;
        }
    }
    if (!any) printf(" aucune");
    printf("\n");
}

int main(void) {
    board_t* b = initialize_board(3, 3);
    // Clear all tiles
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            clear_tile(&b->grid[i][j]);
        }
    }
    // Setup roads on center tile
    tile_t* center = &b->grid[1][1];
    clear_tile(center);
    center->zones[NORTH] = ROAD;
    center->zones[EAST]  = ROAD;
    center->zones[SOUTH] = CITY;
    center->zones[WEST]  = FIELD;
    center->zones[CENTER]  = ROAD;

    // Place meeple on EAST
    center->has_meeple = true;
    center->meeple_position = EAST;
    center->meeple_color = RED;


    center = &b->grid[1][0];
    center->zones[NORTH] = ROAD;
    center->zones[EAST]  = ROAD;
    center->zones[SOUTH] = ROAD;
    center->zones[WEST]  = ROAD;
    center->zones[CENTER]  = ROAD;
    center->has_meeple = true;
    center->meeple_position = SOUTH;
    center->meeple_color = GREEN;

    center = &b->grid[0][1];
    center->zones[NORTH] = ROAD;
    center->zones[EAST]  = ROAD;
    center->zones[SOUTH] = CITY;
    center->zones[WEST]  = ROAD;
    center->zones[CENTER]  = ROAD;

    printf("=== Plateau de démonstration ===\n");
    print_board(b);
    print_meeple_options(b, (position_t){0,1});

    free_board(b);
    return EXIT_SUCCESS;
}