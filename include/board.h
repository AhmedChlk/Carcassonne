#ifndef BOARD_H
#define BOARD_H
    #include "tile.h"
    typedef struct{
        int width,height;
        tile_t** grid;
    }board_t;

    board_t* setup_board(int height , int width);
    void free_board(board_t* board);
#endif