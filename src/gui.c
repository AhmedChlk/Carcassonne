// gui.c
#include "gui.h"
#include "engine.h"
#include <ncurses.h>
#include <stdlib.h>

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

static int color_to_pair(color_t c) {
    return (c == NO_COLOR ? 0 : (int)c + 1);
}

// remplit le pad avec toutes les tuiles + en‑têtes de colonnes
static void draw_board_pad(WINDOW *pad, board_t *b) {
    // — En‑têtes de colonnes, une cellule de largeur TILE_WIDTH à partir de LABEL_WIDTH
    for (int j = 0; j < b->width; j++) {
        int x0 = LABEL_WIDTH + j * TILE_WIDTH;
        // on imprime j juste à droite dans ces TILE_WIDTH colonnes
        mvwprintw(pad, 0, x0, "%*d", TILE_WIDTH, j);
    }

    // pour chaque tuile (i,j)
    for (int i = 0; i < b->height; i++) {
        int base_y = 1 + i * TILE_HEIGHT;

        // Nord
        for (int j = 0; j < b->width; j++) {
            tile_t *t = &b->grid[i][j];
            char c = print_zone(t->zones[NORTH]);
            int x = LABEL_WIDTH + j * TILE_WIDTH + TILE_WIDTH/2;
            if (t->has_meeple && t->meeple_position == NORTH) {
                wattron(pad, COLOR_PAIR(color_to_pair(t->meeple_color)));
                mvwprintw(pad, base_y, x, "%c", c);
                wattroff(pad, COLOR_PAIR(color_to_pair(t->meeple_color)));
            } else {
                mvwprintw(pad, base_y, x, "%c", c);
            }
        }

        // Centre + label de ligne (au milieu de la cellule)
        for (int j = 0; j < b->width; j++) {
            tile_t *t = &b->grid[i][j];
            int x0 = LABEL_WIDTH + j * TILE_WIDTH;
            for (int k = 0; k < TILE_WIDTH; k++) {
                direction_t dir = (k == 0
                                   ? WEST
                                   : (k == TILE_WIDTH-1
                                      ? EAST
                                      : CENTER));
                char ch = print_zone(t->zones[dir]);
                if (t->has_meeple && t->meeple_position == dir) {
                    wattron(pad, COLOR_PAIR(color_to_pair(t->meeple_color)));
                    mvwprintw(pad, base_y+1, x0 + k, "%c", ch);
                    wattroff(pad, COLOR_PAIR(color_to_pair(t->meeple_color)));
                } else {
                    mvwprintw(pad, base_y+1, x0 + k, "%c", ch);
                }
            }
        }

        // Sud
        for (int j = 0; j < b->width; j++) {
            tile_t *t = &b->grid[i][j];
            char c = print_zone(t->zones[SOUTH]);
            int x = LABEL_WIDTH + j * TILE_WIDTH + TILE_WIDTH/2;
            if (t->has_meeple && t->meeple_position == SOUTH) {
                wattron(pad, COLOR_PAIR(color_to_pair(t->meeple_color)));
                mvwprintw(pad, base_y+2, x, "%c", c);
                wattroff(pad, COLOR_PAIR(color_to_pair(t->meeple_color)));
            } else {
                mvwprintw(pad, base_y+2, x, "%c", c);
            }
        }
    }
}

// étiquette fixe : colonnes en ligne 0, lignes au centre de chaque cellule
static void draw_labels(int row_off, int col_off, int map_h, int scr_w, board_t *b) {
    int first_col = col_off / TILE_WIDTH;
    int first_row = row_off / TILE_HEIGHT;
    int max_cols  = (scr_w - LABEL_WIDTH)  / TILE_WIDTH;
    int max_rows  = (map_h - TILE_HEIGHT/2) / TILE_HEIGHT;  // on prend un demi‑tile en plus

    // Colonnes
    for (int j = 0; j <= max_cols && first_col + j < b->width; j++) {
        int x0 = LABEL_WIDTH + j * TILE_WIDTH;
        mvprintw(0, x0, "%*d", TILE_WIDTH, first_col + j);
    }
    // Lignes → au centre de la tuile : décalage d'1 caractère vertical (TILE_HEIGHT/2 = 1)
    for (int i = 0; i <= max_rows && first_row + i < b->height; i++) {
        int y = 1 + i * TILE_HEIGHT + TILE_HEIGHT/2;
        mvprintw(y, 0, "%3d", first_row + i);
    }
}

void run_gui(board_t *b) {
    initscr(); noecho(); cbreak(); keypad(stdscr, TRUE); curs_set(0);
    if (!has_colors()) {
        endwin();
        fprintf(stderr, "Erreur: le terminal ne supporte pas les couleurs\n");
        return;
    }
    start_color();
    init_pair(1, COLOR_RED,    COLOR_BLACK);
    init_pair(2, COLOR_BLUE,   COLOR_BLACK);
    init_pair(3, COLOR_GREEN,  COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_WHITE,  COLOR_BLACK);

    int scr_h, scr_w;
    getmaxyx(stdscr, scr_h, scr_w);
    int info_h = 5;
    int map_h  = scr_h - info_h;

    int pad_h = 1 + b->height * TILE_HEIGHT;
    int pad_w = LABEL_WIDTH + b->width  * TILE_WIDTH;
    WINDOW *pad  = newpad(pad_h, pad_w);
    WINDOW *info = newwin(info_h, scr_w, map_h, 0);
    if (!pad || !info) {
        endwin();
        fprintf(stderr, "Erreur: impossible d'allouer les fenêtres\n");
        return;
    }
    scrollok(info, TRUE);
    box(info, 0, 0);
    mvwprintw(info, 1, 1, "Navigation: Z=haut  S=bas  Q=gauche  D=droite  Esc=quitter");
    wrefresh(info);

    draw_board_pad(pad, b);

    // Centre la tuile (h/2, w/2) au milieu de l'écran
    int mid_r = b->height/2, mid_c = b->width/2;
    int ctr_sy = map_h/2, ctr_sx = scr_w/2;
    int tile_py = 1 + mid_r*TILE_HEIGHT + TILE_HEIGHT/2;
    int tile_px = LABEL_WIDTH + mid_c*TILE_WIDTH + TILE_WIDTH/2;
    int row_off = (tile_py - ctr_sy) / TILE_HEIGHT * TILE_HEIGHT;
    int col_off = (tile_px - ctr_sx) / TILE_WIDTH  * TILE_WIDTH;
    // Clamp
    if (row_off < 0)           row_off = 0;
    if (col_off < 0)           col_off = 0;
    if (row_off > pad_h-map_h) row_off = pad_h - map_h;
    if (col_off > pad_w-scr_w) col_off = pad_w - scr_w;

    // Affichage initial
    prefresh(pad, row_off, col_off, 0, 0, map_h-1, scr_w-1);
    draw_labels(row_off, col_off, map_h, scr_w, b);
    refresh();

    int ch;
    while ((ch = getch()) != 27) {
        switch (ch) {
            case 'z': row_off = row_off>=TILE_HEIGHT ? row_off-TILE_HEIGHT : 0;                     break;
            case 's': row_off = row_off+TILE_HEIGHT <= pad_h-map_h ? row_off+TILE_HEIGHT : pad_h-map_h; break;
            case 'q': col_off = col_off>=TILE_WIDTH  ? col_off-TILE_WIDTH  : 0;                     break;
            case 'd': col_off = col_off+TILE_WIDTH  <= pad_w-scr_w  ? col_off+TILE_WIDTH  : pad_w-scr_w;  break;
            default:   break;
        }
        prefresh(pad, row_off, col_off, 0, 0, map_h-1, scr_w-1);
        draw_labels(row_off, col_off, map_h, scr_w, b);

        // Affiche la position de la tuile au centre
        int cur_r = (row_off + ctr_sy - 1) / TILE_HEIGHT;
        int cur_c = (col_off + ctr_sx - LABEL_WIDTH) / TILE_WIDTH;
        mvwprintw(info, 2, 1,
                  "Position courante (au centre) : ligne %d, colonne %d   ",
                  cur_r, cur_c);
        wrefresh(info);
    }

    delwin(pad);
    delwin(info);
    endwin();
}
