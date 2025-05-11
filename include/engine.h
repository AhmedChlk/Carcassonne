#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"
#include "player.h"
#include "tile.h"

// structure contenant toutes les variables d'une partie de jeu 
typedef struct {
    board_t* board;
    tiles_pack_t tile_pack;
    game_players_t players;
    frontiers_t* frontier;
    valid_positions_t* valid_positions;
} game_t;


typedef struct {
    int tile_count;
    int shield_count;
    int* meeple_count_per_player;
} structure_score_t;

// Initialise un plateau vide
board_t* initialize_board(int height, int width);

// Libère la mémoire allouée pour un plateau
void free_board(board_t* board);

// Charge le pack de tuiles depuis un fichier CSV
tiles_pack_t initialize_tile_pack(const char* filename);

// Mélange aléatoirement le pack de tuiles
void shuffle_tile_pack(tiles_pack_t* tile_pack);

// Crée les joueurs en demandant leur nombre et noms
game_players_t create_players(void);

// Pioche la prochaine tuile du pack
tile_t pick_tile(tiles_pack_t* tile_pack);

// Tourne une tuile de deg degrés
void rotate_tile(tile_t* tile, degree_t deg);

// Vérifie si une case est vide
bool is_cell_empty(board_t* board, position_t pos);

// Alloue et initialise une structure de frontière
frontiers_t* allocate_frontier(void);

// Libère la mémoire d'une frontière
void free_frontier(frontiers_t** frontier);

// Vérifie si pos est dans la frontière de placement
bool is_in_frontier(frontiers_t* frontier, position_t pos);

// Ajoute pos à la frontière si absent
void add_to_frontier(frontiers_t* frontier, position_t pos);

// Retire pos de la frontière
void remove_from_frontier(frontiers_t* frontier, position_t pos);

// Met à jour la frontière après placement d'une tuile
void update_frontier_after_placement(board_t* board, frontiers_t* frontier, position_t pos);

// Place une tuile à pos et met à jour la frontière
void place_tile(board_t* board, frontiers_t* frontier, tile_t tile, position_t pos);

// Place un meeple de couleur c sur la tuile à pos, côté d
void place_meeple(board_t* board, game_players_t* p, direction_t d, position_t pos, color_t c);

// Retire un meeple de la tuile de position p
void remove_meeple(board_t* bd, game_players_t* gp, position_t p);

// Alloue la liste des positions valides pour tuile
valid_positions_t* allocate_valid_positions(void);

// Libère la liste de positions valides
void free_valid_positions(valid_positions_t** v);

// Réinitialise la liste de positions valides
void reset_valid_positions(valid_positions_t* v);

// Ajoute pos à la liste de positions valides
void add_to_valid_positions(valid_positions_t* v, position_t pos);

// Vérifie si une tuile peut être placée à pos selon zones
bool check_matching_zones(board_t* board, tile_t tile, position_t pos);

// Remplit valid_positions avec les emplacements possibles pour la rotation donnée
void get_placement_tile(board_t* board,
                        frontiers_t* frontier,
                        tile_t tile,
                        degree_t rot,
                        valid_positions_t* valid_positions);

// Initialise une partie avec plateau et pack de tuiles
game_t init_game(int board_height, int board_width, const char* tile_csv);

// Vérifie via DFS si un réseau ciblé contient un meeple
bool dfs_has_meeple(board_t* board, int x, int y, direction_t zone, zone_type_t target, bool *visited);

// Retourne pour une tuile les zones où on peut placer un meeple
meeple_options_t get_placement_meeple(board_t* board, position_t pos);

void score_completed_structures(game_t* G, position_t last_pos);

void free_game(game_t* game);

void score_incomplete_structures(game_t* game);
structure_score_t bfs_structure(board_t* board,game_players_t* players,int start_x,int start_y,zone_type_t target_feature,bool* visited);
#endif // ENGINE_H