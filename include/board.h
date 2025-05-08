#ifndef BOARD_H
#define BOARD_H

#include "tile.h"

// === STRUCTURE DU PLATEAU DE JEU ===
typedef struct {
    int width;          
    int height;          
    tile_t** grid;       
} board_t;

#endif // BOARD_H
