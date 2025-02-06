#ifndef ENGINE_H
#define ENGINE_H
#include "board.h"
#include "player.h"

    board_t* init_board(int height , int width);
    
    void free_board(board_t* board);
    tiles_pack_t init_tiles_pack(const char* fname);
    void shuffle_tiles_pack(tiles_pack_t* tp);
    game_players_t create_players();
    tile_t pick_tile (tiles_pack_t* tp);
    void place_tile (board_t* board,tile_t t,int x,int y);
    void rotate_tile(tile_t* t, degree_t deg);
#endif