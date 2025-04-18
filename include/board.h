#ifndef BOARD_H
#define BOARD_H

#include "tile.h"

// Structure du plateau de jeu
typedef struct {
    int width, height;    // Dimensions
    tile_t** grid;        // Grille 2D de pointeurs vers les tuiles
} board_t;

#endif
