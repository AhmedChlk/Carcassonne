#ifndef ENGINE_H
#define ENGINE_H
#include "board.h"
#include "player.h"
#include "tile.h"


    board_t* init_board(int height , int width);
    
    void free_board(board_t* board);
    tiles_pack_t init_tiles_pack(const char* fname);
    void shuffle_tiles_pack(tiles_pack_t* tp);
    game_players_t create_players();
    tile_t pick_tile (tiles_pack_t* tp);
    void place_tile (board_t* board,frontieres_t* f,tile_t t,position_t p);
    void rotate_tile(tile_t* t, degree_t deg);
    bool check_empty_case(board_t* b,position_t p);
    bool is_in_frontiere(frontieres_t* f,position_t p);
    frontieres_t* alloc_frontieres();
    void free_frontieres(frontieres_t** f);
    void add_to_frontiere(frontieres_t* f,position_t p);
    void remove_from_frontiere(frontieres_t* f,position_t p);
    void add_frontieres_after_placement(board_t* b,frontieres_t* f,position_t p);
    

#endif