#ifndef GUI_H
#define GUI_H

#include <ncurses.h>
#include "engine.h"
//hauteur et largeur d'une tuile en caracteres 
#define TILE_WIDTH  3  
#define TILE_HEIGHT 3

// espace réservé aux étiquettes
#define LABEL_WIDTH 4  

// Convertit une couleur en paire ncurses
int color_to_pair(color_t c);

// Dessine toutes les tuiles dans le pad
void draw_board_pad(WINDOW *pad, board_t *b);

// Affiche les numéros de ligne et colonne
void draw_labels(int row_off, int col_off, int map_h, int scr_w, board_t *b);

// Lance l'interface graphique ncurses pour la grille
void run_gui(board_t *b);

#endif
