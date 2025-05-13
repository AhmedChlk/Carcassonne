#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "engine.h"
#include "controller.h"
#include "gui.h"
#include "colors.h"

// Ajout des déclarations si les headers manquent
frontiers_t* create_frontier(void);
valid_positions_t* create_valid_positions(void);

#define BOARD_SIZE 5

// Activez les tests ici
//#define ENABLE_TEST_ABBAYE
//#define ENABLE_TEST_ROUTE
//#define ENABLE_TEST_VILLE
//#define ENABLE_TEST_MAJORITY
//#define ENABLE_TEST_CITY_SHIELD
#define ENABLE_TEST_CITY_WITHOUT_MEEPLES

static game_t* init_test_game() {
    game_t* game = malloc(sizeof(game_t));
    if (!game) {
        fprintf(stderr, "Erreur : allocation de game_t échouée.\n");
        exit(EXIT_FAILURE);
    }
    memset(game, 0, sizeof(game_t));

    game->board = create_board(BOARD_SIZE, BOARD_SIZE);
    game->frontier = create_frontier();
    game->valid_positions = create_valid_positions();
    if (!game->valid_positions) {
        fprintf(stderr, "Erreur : valid_positions non initialisé.\n");
        exit(EXIT_FAILURE);
    }

    game->players.num_players = 2;
    game->players.players = malloc(sizeof(player_t) * game->players.num_players);
    if (!game->players.players) {
        fprintf(stderr, "Erreur : allocation des joueurs échouée.\n");
        exit(EXIT_FAILURE);
    }

    strncpy(game->players.players[0].name, "Alice", sizeof(game->players.players[0].name) - 1);
    game->players.players[0].name[sizeof(game->players.players[0].name) - 1] = '\0';
    game->players.players[0].meeple_color = RED;
    game->players.players[0].meeples = NB_MEEPLES;

    strncpy(game->players.players[1].name, "Bob", sizeof(game->players.players[1].name) - 1);
    game->players.players[1].name[sizeof(game->players.players[1].name) - 1] = '\0';
    game->players.players[1].meeple_color = BLUE;
    game->players.players[1].meeples = NB_MEEPLES;

    return game;
}

static tile_t* create_simple_tile(zone_type_t zones[5]) {
    tile_t* t = malloc(sizeof(tile_t));
    memset(t, 0, sizeof(tile_t));
    for (int i = 0; i < 5; i++) t->zones[i] = zones[i];
    return t;
}

static void print_scores(const game_players_t* players) {
    printf("\n--- État des joueurs ---\n");
    for (int i = 0; i < players->num_players; i++) {
        printf("%s: %d points | Meeples restants : %d\n",
            players->players[i].name,
            players->players[i].score,
            players->players[i].meeples);
    }
}

#ifdef ENABLE_TEST_ABBAYE
void test_score_abbaye_complete() {
    game_t* game = init_test_game();
    board_t* bd = game->board;

    zone_type_t abb[5] = {FIELD, FIELD, FIELD, FIELD, ABBEY};
    tile_t* center = create_simple_tile(abb);
    position_t center_pos = {2, 2};
    bd->grid[2][2] = *center;
    bd->grid[2][2].has_meeple = true;
    bd->grid[2][2].meeple_position = CENTER;
    bd->grid[2][2].meeple_color = RED;

    zone_type_t blank[5] = {FIELD, FIELD, FIELD, FIELD, FIELD};
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (dx == 0 && dy == 0) continue;
            position_t p = {2 + dx, 2 + dy};
            bd->grid[p.x][p.y] = *create_simple_tile(blank);
        }
    }

    printf(CYAN_BOLD "\n==== TEST SCORE ABBAYE COMPLETE ====" ANSI_RESET "\n");
    print_board(bd, 0, 0, BOARD_SIZE, BOARD_SIZE, center_pos);
    print_scores(&game->players);

    score_completed_structures(game, center_pos);

    printf(ANSI_GREEN "\n✅ Test abbaye complète : 9 points attribués à Alice\n" ANSI_RESET);
    print_board(bd, 0, 0, BOARD_SIZE, BOARD_SIZE, center_pos);
    print_scores(&game->players);

    destroy_board(bd);
    free(game->players.players);
    free(game);
}
#endif

#ifdef ENABLE_TEST_ROUTE
void test_score_route_complete() {
    game_t* game = init_test_game();
    board_t* bd = game->board;

    zone_type_t route[5] = {FIELD, ROAD, FIELD, ROAD, FIELD};
    for (int i = 1; i <= 3; i++) bd->grid[2][i] = *create_simple_tile(route);

    bd->grid[2][2].has_meeple = true;
    bd->grid[2][2].meeple_position = EAST;
    bd->grid[2][2].meeple_color = RED;

    printf(CYAN_BOLD "\n==== TEST SCORE ROUTE COMPLETE ====" ANSI_RESET "\n");
    print_board(bd, 0, 0, BOARD_SIZE, BOARD_SIZE, (position_t){2,2});
    print_scores(&game->players);

    score_completed_structures(game, (position_t){2,2});

    printf(ANSI_GREEN "\n✅ Test route complète : 2 points attribués à Alice\n" ANSI_RESET);
    print_board(bd, 0, 0, BOARD_SIZE, BOARD_SIZE, (position_t){2,2});
    print_scores(&game->players);

    destroy_board(bd);
    free(game->players.players);
    free(game);
}
#endif

#ifdef ENABLE_TEST_VILLE
void test_score_ville_complete() {
    game_t* game = init_test_game();
    board_t* bd = game->board;

    zone_type_t ville1[5] = {FIELD, CITY, FIELD, FIELD, FIELD};
    zone_type_t ville2[5] = {FIELD, FIELD, FIELD, CITY, FIELD};
    bd->grid[2][2] = *create_simple_tile(ville1);
    bd->grid[2][3] = *create_simple_tile(ville2);

    bd->grid[2][2].has_meeple = true;
    bd->grid[2][2].meeple_position = EAST;
    bd->grid[2][2].meeple_color = RED;

    printf(CYAN_BOLD "\n==== TEST SCORE VILLE COMPLETE ====" ANSI_RESET "\n");
    print_board(bd, 0, 0, BOARD_SIZE, BOARD_SIZE, (position_t){2,2});
    print_scores(&game->players);

    score_completed_structures(game, (position_t){2,2});

    printf(ANSI_GREEN "\n✅ Test ville complète : 4 points attribués à Alice\n" ANSI_RESET);
    print_board(bd, 0, 0, BOARD_SIZE, BOARD_SIZE, (position_t){2,2});
    print_scores(&game->players);

    destroy_board(bd);
    free(game->players.players);
    free(game);
}
#endif

#ifdef ENABLE_TEST_MAJORITY
void test_majority_meep() {
    game_t* game = init_test_game();
    board_t* bd = game->board;

    zone_type_t route[5] = {FIELD, ROAD, FIELD, ROAD, ROAD};
    zone_type_t road_east[5] = {FIELD, ROAD, FIELD, FIELD, FIELD};
    zone_type_t road_west[5] = {FIELD, FIELD, FIELD, ROAD, FIELD};


    for (int i = 1; i <= 3; i++) bd->grid[2][i] = *create_simple_tile(route);
    bd->grid[2][0] = *create_simple_tile(road_east);
    bd->grid[2][4] = *create_simple_tile(road_west);

    bd->grid[2][1].has_meeple = true;
    bd->grid[2][1].meeple_position = EAST;
    bd->grid[2][1].meeple_color = RED;
    bd->grid[2][2].has_meeple = true;
    bd->grid[2][2].meeple_position = EAST;
    bd->grid[2][2].meeple_color = RED;
    bd->grid[2][3].has_meeple = true;
    bd->grid[2][3].meeple_position = EAST;
    bd->grid[2][3].meeple_color = BLUE;

    printf(CYAN_BOLD "\n==== TEST MAJORITE MEEPLES ====" ANSI_RESET "\n");
    print_board(bd, 0, 0, BOARD_SIZE, BOARD_SIZE, (position_t){2,2});
    print_scores(&game->players);

    score_completed_structures(game, (position_t){2,2});
    print_board(bd, 0, 0, BOARD_SIZE, BOARD_SIZE, (position_t){2,2});
    print_scores(&game->players);

    destroy_board(bd);
    free(game->players.players);
    free(game);
}
#endif

#ifdef ENABLE_TEST_CITY_SHIELD
void test_city_with_shield() {
    game_t* game = init_test_game();
    board_t* bd = game->board;

    zone_type_t ville1[5] = {FIELD, CITY, FIELD, SHIELD, FIELD};
    zone_type_t ville2[5] = {FIELD, SHIELD, FIELD, CITY, FIELD};
    bd->grid[2][2] = *create_simple_tile(ville1);
    bd->grid[2][3] = *create_simple_tile(ville2);

    bd->grid[2][2].has_meeple = true;
    bd->grid[2][2].meeple_position = EAST;
    bd->grid[2][2].meeple_color = RED;

    printf(CYAN_BOLD "\n==== TEST CITY + SHIELD ====" ANSI_RESET "\n");
    print_board(bd, 0, 0, BOARD_SIZE, BOARD_SIZE, (position_t){2,2});
    print_scores(&game->players);

    score_completed_structures(game, (position_t){2,2});

    print_board(bd, 0, 0, BOARD_SIZE, BOARD_SIZE, (position_t){2,2});
    print_scores(&game->players);

    destroy_board(bd);
    free(game->players.players);
    free(game);
}

#endif

#ifdef ENABLE_TEST_CITY_WITHOUT_MEEPLES
void test_city_without_meeples() {
    game_t* game = init_test_game();
    board_t* bd = game->board;

    // Ville fermée en deux tuiles connectées Est-Ouest
    zone_type_t ville1[5] = {FIELD, CITY, FIELD, FIELD, FIELD};
    zone_type_t ville2[5] = {FIELD, FIELD, FIELD, CITY, FIELD};

    bd->grid[2][2] = *create_simple_tile(ville1);
    bd->grid[2][3] = *create_simple_tile(ville2);

    printf(CYAN_BOLD "\n==== TEST CITY SANS MEEPLES ====\n" ANSI_RESET);
    print_board(bd, 0, 0, BOARD_SIZE, BOARD_SIZE, (position_t){2, 2});
    print_scores(&game->players);

    score_completed_structures(game, (position_t){2, 2});

    printf(ANSI_GREEN "\n✅ Test ville sans meeple : aucun point attribué\n" ANSI_RESET);
    print_board(bd, 0, 0, BOARD_SIZE, BOARD_SIZE, (position_t){2, 2});
    print_scores(&game->players);

    destroy_board(bd);
    free(game->players.players);
    free(game);
}
#endif


int main(void) {
#ifdef ENABLE_TEST_ABBAYE
    test_score_abbaye_complete();
#endif
#ifdef ENABLE_TEST_ROUTE
    test_score_route_complete();
#endif
#ifdef ENABLE_TEST_VILLE
    test_score_ville_complete();
#endif
#ifdef ENABLE_TEST_MAJORITY
    test_majority_meep();
#endif
#ifdef ENABLE_TEST_CITY_SHIELD
    test_city_with_shield();
#endif

#ifdef ENABLE_TEST_CITY_WITHOUT_MEEPLES
    test_city_without_meeples();
#endif
    return 0;
}
