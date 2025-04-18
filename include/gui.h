// gui.h
#ifndef GUI_H
#define GUI_H

#include "engine.h"

// Dimensions d'une tuile à l'écran (3×3 caractères)
#define TILE_WIDTH  3
#define TILE_HEIGHT 3

// Largeur réservée pour les labels de ligne
#define LABEL_WIDTH 4

void run_gui(board_t *b);

#endif // GUI_H
