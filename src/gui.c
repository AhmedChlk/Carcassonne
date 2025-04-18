#include "gui.h"
#include "engine.h"
#include <ncurses.h>
#include <stdlib.h>

// Convertit un type de zone en caractère pour l'affichage
char print_zone(zone_type_t z) {
    switch (z) {
        case SHIELD:  return 's';  // bouclier
        case ROAD:    return 'r';  // route
        case FIELD:   return 'f';  // champ
        case CITY:    return 'c';  // ville
        case VILLAGE: return 'v';  // village
        case ABBEY:   return 'a';  // abbaye
        case NONE:    return ' ';  // rien
        default:      return ' ';
    }
}


int color_to_pair(color_t c) {
    return (c == NO_COLOR ? 0 : (int)c + 1);
}

void draw_board_pad(WINDOW *pad, board_t *b) {
    // Têtes de colonnes (numéros en haut)
    for (int j = 0; j < b->width; j++) {
        int x0 = LABEL_WIDTH + j * TILE_WIDTH;
        mvwprintw(pad, 0, x0, "%*d", TILE_WIDTH, j);
    }

    // Parcours de chaque tuile (i = ligne, j = colonne)
    for (int i = 0; i < b->height; i++) {
        int base_y = 1 + i * TILE_HEIGHT;

        // Bord nord de la tuile
        for (int j = 0; j < b->width; j++) {
            tile_t *t = &b->grid[i][j];
            char c = print_zone(t->zones[NORTH]);
            int x = LABEL_WIDTH + j * TILE_WIDTH + TILE_WIDTH/2;
            mvwprintw(pad, base_y, x, "%c", c);
        }

        // Centre de la tuile + numéro de ligne à gauche
        mvwprintw(pad, base_y+1, 0, "%3d", i);
        for (int j = 0; j < b->width; j++) {
            tile_t *t = &b->grid[i][j];
            int x0 = LABEL_WIDTH + j * TILE_WIDTH;
            for (int k = 0; k < TILE_WIDTH; k++) {
                direction_t dir;
                if (k == 0) dir = WEST;
                else if (k == TILE_WIDTH-1) dir = EAST;
                else dir = CENTER;
                char ch = print_zone(t->zones[dir]);
                mvwprintw(pad, base_y+1, x0 + k, "%c", ch);
            }
        }

        // Bord sud de la tuile
        for (int j = 0; j < b->width; j++) {
            tile_t *t = &b->grid[i][j];
            char c = print_zone(t->zones[SOUTH]);
            int x = LABEL_WIDTH + j * TILE_WIDTH + TILE_WIDTH/2;
            mvwprintw(pad, base_y+2, x, "%c", c);
        }
    }
}

void draw_labels(int row_off, int col_off, int map_h, int scr_w, board_t *b) {
    int first_col = col_off / TILE_WIDTH;
    int first_row = row_off / TILE_HEIGHT;
    int max_cols  = (scr_w - LABEL_WIDTH)  / TILE_WIDTH;
    int max_rows  = (map_h - TILE_HEIGHT/2) / TILE_HEIGHT;

    // Colonnes
    for (int j = 0; j <= max_cols && first_col + j < b->width; j++) {
        int x0 = LABEL_WIDTH + j * TILE_WIDTH;
        mvprintw(0, x0, "%*d", TILE_WIDTH, first_col + j);
    }
    // Lignes
    for (int i = 0; i <= max_rows && first_row + i < b->height; i++) {
        int y = 1 + i * TILE_HEIGHT + TILE_HEIGHT/2;
        mvprintw(y, 0, "%3d", first_row + i);
    }
}

// Point d'entrée de l'interface graphique
void run_gui(board_t *b) {
    initscr();    // démarre ncurses
    noecho();     // ne pas afficher les touches
    cbreak();     // mode brut
    keypad(stdscr, TRUE);
    curs_set(0);  // cacher le curseur

    if (!has_colors()) {
        endwin();
        fprintf(stderr, "Erreur: pas de couleur\n");
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
    int info_h = 5;           // hauteur du panneau bas
    int map_h  = scr_h - info_h;

    int pad_h = 1 + b->height * TILE_HEIGHT;
    int pad_w = LABEL_WIDTH + b->width  * TILE_WIDTH;
    WINDOW *pad  = newpad(pad_h, pad_w);         // zone scrollable
    WINDOW *info = newwin(info_h, scr_w, map_h, 0); // panneau de texte
    scrollok(info, TRUE);

    box(info, 0, 0); // bordure du panneau
    mvwprintw(info, 1, 1, "Z/S/Q/D: déplacer, Esc: quitter");
    wrefresh(info);

    // Remplir le pad avec le plateau
    draw_board_pad(pad, b);

    // Centrage initial sur la tuile centrale
    int mid_r = b->height/2;
    int mid_c = b->width/2;
    int ctr_y = map_h/2;
    int ctr_x = scr_w/2;
    int tile_py = 1 + mid_r*TILE_HEIGHT + TILE_HEIGHT/2;
    int tile_px = LABEL_WIDTH + mid_c*TILE_WIDTH + TILE_WIDTH/2;
    int row_off = (tile_py - ctr_y) / TILE_HEIGHT * TILE_HEIGHT;
    int col_off = (tile_px - ctr_x) / TILE_WIDTH  * TILE_WIDTH;
    if (row_off < 0) row_off = 0;
    if (col_off < 0) col_off = 0;
    if (row_off > pad_h-map_h) row_off = pad_h - map_h;
    if (col_off > pad_w-scr_w) col_off = pad_w - scr_w;

    // Affichage initial
    prefresh(pad, row_off, col_off, 0, 0, map_h-1, scr_w-1);
    draw_labels(row_off, col_off, map_h, scr_w, b);
    refresh();

    int ch;
    // Boucle de gestion des touches
    while ((ch = getch()) != 27) {
        if (ch == 'z') row_off = row_off>=TILE_HEIGHT ? row_off-TILE_HEIGHT : 0;
        if (ch == 's') row_off = row_off+TILE_HEIGHT <= pad_h-map_h ? row_off+TILE_HEIGHT : pad_h-map_h;
        if (ch == 'q') col_off = col_off>=TILE_WIDTH  ? col_off-TILE_WIDTH  : 0;
        if (ch == 'd') col_off = col_off+TILE_WIDTH  <= pad_w-scr_w  ? col_off+TILE_WIDTH  : pad_w-scr_w;

        // Redessiner
        prefresh(pad, row_off, col_off, 0, 0, map_h-1, scr_w-1);
        draw_labels(row_off, col_off, map_h, scr_w, b);
        wrefresh(info);
    }

    // Nettoyage
    delwin(pad);
    delwin(info);
    endwin();
}
