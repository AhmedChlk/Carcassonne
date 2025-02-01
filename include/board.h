#ifndef BOARD_H
#define BOARD_H
    #include "tile.h"
    typedef struct{
        int width,height;
        tile_t** board;
    }board_t;
#endif