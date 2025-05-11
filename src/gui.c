#include "gui.h"
#include "engine.h"
#include <stdio.h>
#include "colors.h"

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



void print_tile(tile_t t) {
    printf("\033[1m\033[0;37m┌───┐\n" ANSI_RESET);
    printf("│ %s%c" ANSI_RESET " │\n",
           zone_color(t.zones[NORTH]), print_zone(t.zones[NORTH]));
    printf("│%s%c" ANSI_RESET "%s%c" ANSI_RESET "%s%c" ANSI_RESET "│\n",
           zone_color(t.zones[WEST]),   print_zone(t.zones[WEST]),
           zone_color(t.zones[CENTER]), print_zone(t.zones[CENTER]),
           zone_color(t.zones[EAST]),   print_zone(t.zones[EAST]));
    printf("│ %s%c" ANSI_RESET " │\n",
           zone_color(t.zones[SOUTH]), print_zone(t.zones[SOUTH]));
    printf("\033[1m\033[0;37m└───┘\n" ANSI_RESET);
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
                   BG_GREEN, keys[i], names[i], ANSI_RESET);
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
            printf("\033[1m\033[0;37m┌───┐" ANSI_RESET);
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
                       ANSI_RESET);
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
                       ANSI_RESET);
            } else {
                printf("%c", print_zone(t.zones[WEST]));
            }
            // CENTRE
            if (t.has_meeple && t.meeple_position == CENTER) {
                printf("%s%c%s",
                       color_code(t.meeple_color),
                       print_zone(t.zones[CENTER]),
                       ANSI_RESET);
            } else {
                printf("%c", print_zone(t.zones[CENTER]));
            }
            // EST
            if (t.has_meeple && t.meeple_position == EAST) {
                printf("%s%c%s",
                       color_code(t.meeple_color),
                       print_zone(t.zones[EAST]),
                       ANSI_RESET);
            } else {
                printf("%c", print_zone(t.zones[EAST]));
            }
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
                       ANSI_RESET);
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

            printf("\033[1m\033[0;37m└───┘" ANSI_RESET);
        }
        printf("\n");
    }
}




void display_welcome(void) {
    const char *art[] = {
        "  ____    _    ____   ____    _    ____ ____   ___  _   _ _   _ _____ ",
        " / ___|  / \\  |  _ \\ / ___|  / \\  / ___/ ___| / _ \\| \\ | | \\ | | ____|",
        "| |     / _ \\ | |_) | |     / _ \\ \\___ \\___ \\| | | |  \\| |  \\| |  _|  ",
        "| |___ / ___ \\|  _ <| |___ / ___ \\ ___) |__) | |_| | |\\  | |\\  | |___ ",
        " \\____/_/   \\_\\_| \\_\\\\____/_/   \\_\\____/____/ \\___/|_| \\_|_| \\_|_____|"
    };
    for (int i = 0; i < (int)(sizeof(art)/sizeof(*art)); i++)
        printf("%s\n", art[i]);
    printf("\n          Bienvenue dans Carcassonne !\n");
    printf("=============================================\n\n");
}

int display_menu(void) {
    int choice;
    do {
        printf("╔════════════════════════════════════╗\n");
        printf("║          Menu principal           ║\n");
        printf("╠════════════════════════════════════╣\n");
        printf("║ 1. Commencer une nouvelle partie  ║\n");
        printf("║ 2. Quitter                        ║\n");
        printf("╚════════════════════════════════════╝\n");
        printf("Entrez votre choix (1–2) : ");
        if (scanf("%d", &choice) != 1) choice = 0;
        while (getchar()!='\n');
    } while (choice < 1 || choice > 2);
    return choice;
}

void display_turn_header(const char *player_name,
                                color_t col,
                                bool tileInHand,
                                degree_t handRot)
{
    const char *ansi = color_code(col);
    printf("%s>>> Tour de %s%s", ansi, player_name, ANSI_RESET);
    if (tileInHand) printf(" | Rot %d°", handRot * 90);
    printf("\n");
}

void display_scores(game_t *game) {
    printf("Scores :");
    for (int i = 0; i < game->players.num_players; i++) {
        player_t *p = &game->players.players[i];
        printf(" %s=%d", p->name, p->score);
    }
    printf("\n\n");
}
