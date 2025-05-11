#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"
#include "player.h"
#include "tile.h"

/* =========== STRUCTURES PRINCIPALES =========== */

// Structure contenant toutes les variables d'une partie de jeu
typedef struct {
    board_t* board;
    tiles_pack_t tile_pack;
    game_players_t players;
    frontiers_t* frontier;
    valid_positions_t* valid_positions;
} game_t;

// Structure utilisée pour le calcul de score d'une structure
typedef struct {
    int tile_count;
    int shield_count;
    int* meeple_count_per_player;
} structure_score_t;

/* =========== GESTION DU PLATEAU =========== */

board_t* create_board(int height, int width);
void destroy_board(board_t* board);
bool is_cell_empty(board_t* board, position_t pos);

/* =========== GESTION DES TUILES =========== */

tiles_pack_t initialize_tile_pack(const char* filename);
void shuffle_tile_pack(tiles_pack_t* tile_pack);
tile_t pick_tile(tiles_pack_t* tile_pack);
void rotate_tile(tile_t* tile, degree_t deg);

/* Vérifie si une tuile peut être placée à pos selon les zones adjacentes */
bool check_matching_zones(board_t* board, tile_t tile, position_t pos);

/* Remplit valid_positions avec les positions valides pour la rotation donnée */
void get_placement_tile(board_t* board, frontiers_t* frontier, tile_t tile, degree_t rot, valid_positions_t* valid_positions);

/* =========== GESTION DES JOUEURS =========== */

game_players_t create_players(void);

/* =========== FRONTIÈRES DU PLATEAU =========== */

frontiers_t* allocate_frontier(void);
void free_frontier(frontiers_t** frontier);
bool is_in_frontier(frontiers_t* frontier, position_t pos);
void add_to_frontier(frontiers_t* frontier, position_t pos);
void remove_from_frontier(frontiers_t* frontier, position_t pos);
void update_frontier_after_placement(board_t* board, frontiers_t* frontier, position_t pos);
void place_tile(board_t* board, frontiers_t* frontier, tile_t tile, position_t pos); // à implémenter si utilisé

/* =========== GESTION DES MEEPLES =========== */

void place_meeple(board_t* board, game_players_t* p, direction_t d, position_t pos, color_t c);
void remove_meeple(board_t* bd, game_players_t* gp, position_t p);

/* Retourne les directions valides où placer un meeple sur la tuile */
meeple_options_t get_placement_meeple(board_t* board, position_t pos);

/* =========== POSITIONS VALIDES =========== */

valid_positions_t* create_valid_positions(void);
void destroy_valid_positions(valid_positions_t** vp_ptr);
void reset_valid_positions(valid_positions_t* vp);
void add_valid_position(valid_positions_t* vp, position_t pos);
bool is_valid_position(valid_positions_t* vp, position_t pos); // à implémenter si utilisé

/* =========== INITIALISATION DE PARTIE =========== */

/* Initialise une partie avec le plateau et le pack de tuiles */
game_t init_game(int board_height, int board_width, const char* tile_csv);

/* Libère toutes les ressources de la partie */
void free_game(game_t* game);

/* =========== SCORING =========== */

/* Marque les structures complétées à partir de la dernière position jouée */
void score_completed_structures(game_t* G, position_t last_pos);

/* Marque les structures incomplètes en fin de partie */
void score_incomplete_structures(game_t* game);

/* BFS pour explorer une structure et retourner son score */
structure_score_t bfs_structure(board_t* board, game_players_t* players, int start_x, int start_y, zone_type_t target_feature, bool* visited);

/* DFS pour vérifier si une structure contient un meeple */
bool dfs_has_meeple(board_t* board, int x, int y, direction_t zone, zone_type_t target, bool *visited);

#endif // ENGINE_H
