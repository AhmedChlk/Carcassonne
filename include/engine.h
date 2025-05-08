#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"
#include "player.h"
#include "tile.h"

// === STRUCTURE PRINCIPALE DU JEU ===
typedef struct {
    board_t* board;
    tiles_pack_t tile_pack;
    game_players_t players;
    frontiers_t* frontier;
    valid_positions_t* valid_positions;
} game_t;

// === INITIALISATION & LIBÉRATION ===
game_t init_game(int board_height, int board_width, const char* tile_csv);
void free_game(game_t* game);
board_t* initialize_board(int height, int width);
void free_board(board_t* board);
game_players_t create_players(void);
tiles_pack_t initialize_tile_pack(const char* filename);
frontiers_t* allocate_frontier(void);
void free_frontier(frontiers_t** frontier);
valid_positions_t* allocate_valid_positions(void);
void free_valid_positions(valid_positions_t** v);

// === UTILITAIRES ===
bool check_matching_zones(board_t* bd, tile_t t, position_t p);
bool is_cell_empty(board_t* bd, position_t pos);
bool feature_matches(zone_type_t a, zone_type_t b);
bool has_tile(board_t* bd, int x, int y);

// === MANIPULATION DES TUILES ===
void shuffle_tile_pack(tiles_pack_t* tile_pack);
tile_t pick_tile(tiles_pack_t* tile_pack);
void rotate_tile(tile_t* tile, degree_t deg);
void place_tile(board_t* board, frontiers_t* frontier, tile_t tile, position_t pos);
void get_placement_tile(board_t* board,frontiers_t* frontier,tile_t tile,degree_t rot,valid_positions_t* valid_positions);

// === MEEPLES ===
void place_meeple(board_t* board, game_players_t* p, direction_t d, position_t pos, color_t c);
void remove_meeple(board_t* bd, game_players_t* gp, position_t p);
bool dfs_has_meeple(board_t* board, int x, int y, direction_t zone, zone_type_t target, bool *visited);
meeple_options_t get_placement_meeple(board_t* board, position_t pos);

// === GESTION DES FRONTIÈRES ===
bool is_in_frontier(frontiers_t* frontier, position_t pos);
void add_to_frontier(frontiers_t* frontier, position_t pos);
void remove_from_frontier(frontiers_t* frontier, position_t pos);
void update_frontier_after_placement(board_t* board, frontiers_t* frontier, position_t pos);


// === POSITIONS VALIDES POUR LES TUILES ===
void reset_valid_positions(valid_positions_t* v);
void add_to_valid_positions(valid_positions_t* v, position_t pos);

// === CALCUL DES SCORES ===
void score_completed_structures(game_t* G, position_t last_pos);
/**
 * \brief Scoring final : compte et attribue les points pour toutes les
 *        structures incomplètes restantes (routes, villes, boucliers, abbayes).
 */
 void final_scoring(game_t *game);
#endif // ENGINE_H
