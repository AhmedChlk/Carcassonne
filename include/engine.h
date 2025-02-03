#ifndef ENGINE_H
#define ENGINE_H
#include "board.h"

    board_t* init_board(int height , int width);
    
    void free_board(board_t* board);
    tiles_pack_t init_tiles_pack(const char* fname);
#endif