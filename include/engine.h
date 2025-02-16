#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"
#include "player.h"
#include "tile.h"

board_t* initialize_board(int height, int width);
void free_board(board_t* board);
tiles_pack_t initialize_tile_pack(const char* filename);
void shuffle_tile_pack(tiles_pack_t* tile_pack);
game_players_t create_players();
tile_t pick_tile(tiles_pack_t* tile_pack);
void rotate_tile(tile_t* tile, degree_t deg);
bool is_cell_empty(board_t* board, position_t pos);
bool is_in_frontier(frontiers_t* frontier, position_t pos);
frontiers_t* allocate_frontier();
void free_frontier(frontiers_t** frontier);
void add_to_frontier(frontiers_t* frontier, position_t pos);
void remove_from_frontier(frontiers_t* frontier, position_t pos);
void update_frontier_after_placement(board_t* board, frontiers_t* frontier, position_t pos);
void place_tile(board_t* board, frontiers_t* frontier, tile_t tile, position_t pos);
void place_meeple(board_t* board, game_players_t* p, direction_t d, position_t pos, color_t c);
void remove_meeple(board_t* board, game_players_t* p, direction_t d, position_t pos, color_t c);






valid_positions_t* allocate_valid_positions();
void free_valid_positions (valid_positions_t** v);
void reset_valid_positions(valid_positions_t* v);
void add_to_valid_positions (valid_positions_t* v,position_t pos);
bool check_matching_zones(board_t* board,tile_t tile,position_t pos);
void get_placement_tile (board_t* b,frontiers_t* frontier,tile_t tile,valid_positions_t* valid_positions);


 
void expand_board(board_t* board);

#endif
